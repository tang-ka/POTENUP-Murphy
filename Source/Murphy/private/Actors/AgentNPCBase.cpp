
#include "Actors/AgentNPCBase.h"

#include "HttpManager.h"
#include "Murphy.h"
#include "Framework/AKTestPlayerController.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "HttpModule.h"                       // 오디오 다운로드용
#include "Interfaces/IHttpResponse.h"
#include "Sound/SoundWaveProcedural.h"        // 런타임 사운드 생성용

AAgentNPCBase::AAgentNPCBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(FName("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	
	FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
	FaceMesh->SetupAttachment(GetMesh());
	
	VoiceComp = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceComp"));
	VoiceComp->SetupAttachment(FaceMesh);
	VoiceComp->bAutoActivate = false;
}

void AAgentNPCBase::BeginPlay()
{
	Super::BeginPlay();
	 
	if (IsValid(InteractionBox))
	{
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AAgentNPCBase::OnInteractionBoxBeginOverlap);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AAgentNPCBase::OnInteractionBoxEndOverlap);
	}
}

void AAgentNPCBase::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn) || OtherPawn == this) return;

	if (AAKTestPlayerController* MyPC = Cast<AAKTestPlayerController>(OtherPawn->GetController()))
	{
		MyPC->SetActiveNPC(this);
		PRINTLOG_JW(TEXT("PC에 현재 Overlap 된 NPC Active."));
	}
}

void AAgentNPCBase::OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn) || OtherPawn == this) return;
	
	if (AAKTestPlayerController* MyPC = Cast<AAKTestPlayerController>(OtherPawn->GetController()))
	{
		MyPC->SetActiveNPC(nullptr);
	}
}

void AAgentNPCBase::UpdateEmotion(int32 EmotionLevel)
{
	// todo: AnimInstance로 EmotionLevel을 Enum으로 만들어서 분기 처리
}

void AAgentNPCBase::ProcessDialogueResponse(const FString& Dialogue, int32 EmotionLevel, const FString& AudioURL)
{
	PRINTLOGW_JW(TEXT("[AgentNPC] AI 응답 대사: %s / 감정 수치: %d"), *Dialogue, EmotionLevel);
	
	UpdateEmotion(EmotionLevel);
	
	// TTS 재생 (프로토타입: 더미 사운드, 추후 AudioURL 기반 스트리밍으로 교체)
	if (IsValid(VoiceComp))
	{
		// VoiceComp->Play();
		DownloadAndPlayAudio(AudioURL);
	}
}

void AAgentNPCBase::DownloadAndPlayAudio(const FString& AudioURL)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(AudioURL);
	Request->SetVerb(TEXT("Get"));
	Request->OnProcessRequestComplete().BindUObject(this, &AAgentNPCBase::OnAudioDownloaded);
	Request->ProcessRequest();
	
	PRINTLOGW_JW(TEXT("[AgentNPC] 오디오 다운로드 시작: %s"), *AudioURL);
}

void AAgentNPCBase::OnAudioDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		PRINTLOGE_JW(TEXT("[AgentNPC] 오디오 다운로드 실패 - 코드: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
		return;
	}
	
	const TArray<uint8>& WavData = Response->GetContent();
	
	// WAV 헤더 최소 크기 방어 체크 (44바이트)
	if (WavData.Num() < 44)
	{
		PRINTLOGE_JW(TEXT("[AgentNPC] 수신된 WAV 데이터가 너무 작음: %d bytes"), WavData.Num());
		return;
	}
	
	// === WAV 헤더 파싱 
	const uint8* Raw = WavData.GetData();
	
	// "RIFF" 시그니처 검증
	if (Raw[0] != 'R' || Raw[1] != 'I' || Raw[2] != 'F' || Raw[3] != 'F')
	{
		PRINTLOGE_JW(TEXT("[AgentNPC] 유효하지 않은 WAV 파일 (RIFF 시그니처 없음)"));
		return;
	}
	const int16 NumChannels   = *reinterpret_cast<const int16*>(Raw + 22);
	const int32 SampleRate    = *reinterpret_cast<const int32*>(Raw + 24);
	const int16 BitsPerSample = *reinterpret_cast<const int16*>(Raw + 34);
	const int32 PCMDataSize   = *reinterpret_cast<const int32*>(Raw + 40);
	const uint8* PCMStart     = Raw + 44;
	
	// 방어 코드: 실제 데이터 크기와 헤더 명시 크기 비교
	if (WavData.Num() < 44 + PCMDataSize)
	{
		PRINTLOGE_JW(TEXT("[AgentNPC] WAV 데이터 크기 불일치"));
		return;
	}
	
	PRINTLOGW_JW(TEXT("[AgentNPC] WAV 파싱 완료 - SR:%d Ch:%d Bit:%d PCM:%d bytes"), SampleRate, NumChannels, BitsPerSample, PCMDataSize);
	
	// ── USoundWaveProcedural 생성 및 재생 ────────────────────────
	// NewObject의 Outer를 this로 지정해 GC에서 액터와 함께 관리되게 함
	USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>(this);
	SoundWave->SetSampleRate(SampleRate);
	SoundWave->NumChannels  = NumChannels;
	SoundWave->Duration     = static_cast<float>(PCMDataSize) /
							  (SampleRate * NumChannels * (BitsPerSample / 8));
	SoundWave->SoundGroup   = SOUNDGROUP_Voice; // 보이스 그룹으로 설정
	SoundWave->bLooping     = false;
	
	// PCM 데이터를 큐에 적재
	SoundWave->QueueAudio(PCMStart, PCMDataSize);
	
	// VoiceComp에 사운드를 교체하고 재생
	if (IsValid(VoiceComp))
	{
		VoiceComp->SetSound(SoundWave);
		VoiceComp->Play();
		UE_LOG(LogTemp, Log, TEXT("[AgentNPC] NPC 음성 재생 시작 (%.1f초)"), SoundWave->Duration);
	}
}
