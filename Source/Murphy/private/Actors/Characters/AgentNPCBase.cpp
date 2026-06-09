
#include "Actors/Characters/AgentNPCBase.h"

#include "HttpManager.h"
#include "Murphy.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "HttpModule.h"                       // 오디오 다운로드용
#include "Components/WidgetComponent.h"
#include "Framework/MurphyPlayerController.h"
#include "Interfaces/IHttpResponse.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundWaveProcedural.h"        // 런타임 사운드 생성용
#include "Manager/ScenarioSubsystem.h"
#include "UI/AgentEmojiUI.h"
#include "Kismet/GameplayStatics.h"

AAgentNPCBase::AAgentNPCBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	
	FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
	FaceMesh->SetupAttachment(GetMesh());
	
	VoiceComp = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceComp"));
	VoiceComp->SetupAttachment(FaceMesh);
	VoiceComp->bAutoActivate = false;
	
	EmojiComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	EmojiComp->SetupAttachment(GetMesh());
	static ConstructorHelpers::FClassFinder<UUserWidget> EmojiUIClass(TEXT("/Game/UI/Blueprints/WBP_NPCEmoji.WBP_NPCEmoji_C"));
	if (EmojiUIClass.Succeeded()) EmojiComp->SetWidgetClass(EmojiUIClass.Class);
	EmojiComp->SetWidgetSpace(EWidgetSpace::Screen);
	EmojiComp->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	
	// 타이핑 오디오 컴포넌트 생성 및 설정
	TypingAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("TypingAudioComp"));
	TypingAudioComp->SetupAttachment(GetMesh());
	TypingAudioComp->bAutoActivate = false;
}

void AAgentNPCBase::BeginPlay()
{
	Super::BeginPlay();
	
	EmojiUI = Cast<UAgentEmojiUI>(EmojiComp->GetUserWidgetObject());
	EmojiUI->SetEmojiVisible(false);
	EmojiUI->SetNPCName(TEXT("BBung"));
	if (!NPCName.IsEmpty()) EmojiUI->SetNPCName(NPCName);

	if (IsValid(TypingSound)) TypingAudioComp->SetSound(TypingSound);
	
	if (IsValid(InteractionBox))
	{
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AAgentNPCBase::OnInteractionBoxBeginOverlap);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AAgentNPCBase::OnInteractionBoxEndOverlap);
	}
}

void AAgentNPCBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// --- 위젯 컴포넌트 업데이트 (Screen 스케일 / World 빌보딩) ---
	if (IsValid(EmojiComp) && GetNetMode() != NM_DedicatedServer)
	{
		if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
		{
			FVector CameraLoc = CameraManager->GetCameraLocation();
			FVector WidgetLoc = EmojiComp->GetComponentLocation();

			if (EmojiComp->GetWidgetSpace() == EWidgetSpace::Screen)
			{
				// [Screen 모드] 거리에 따라 위젯 스케일을 작아지게 만듭니다.
				float Distance = FVector::Dist(CameraLoc, WidgetLoc);
				float Scale = FMath::Clamp(300.0f / FMath::Max(Distance, 1.0f), 0.1f, 1.0f);
				
				if (IsValid(EmojiUI))
				{
					EmojiUI->SetRenderScale(FVector2D(Scale, Scale));
				}
			}
			// else if (EmojiComp->GetWidgetSpace() == EWidgetSpace::World)
			// {
			 	// [World 모드] 위젯이 항상 로컬 플레이어의 카메라를 바라보도록 회전시킵니다. (빌보딩)
			// 	FRotator LookAtRot = (CameraLoc - WidgetLoc).Rotation();
			// 	EmojiComp->SetWorldRotation(LookAtRot);
			// }
		}
	}
	
	if (bIsWaitingForPlayer)
	{
		CurWaitTime += DeltaSeconds;
		
		// 지연 캐싱 (BeginPlay에서 실패했을 경우를 대비한 안전 장치)
		// if (!IsValid(EmojiUI))
		// {
		// 	EmojiUI = Cast<UAgentEmojiUI>(EmojiComp->GetUserWidgetObject());
		// }
		
		if (IsValid(EmojiUI))
		{
			EmojiUI->SetProgress(CurWaitTime / MaxWaitTime);
		}
		
		if (CurWaitTime >= MaxWaitTime)
		{
			bIsWaitingForPlayer = false;
			
			PRINTLOGW_JW(TEXT("[AgentNPC] 1분 타임아웃! 플레이어가 대답하지 않았습니다."));
			
			if (AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				PC->SendTimeoutAudioToAI();
			}
		}
	}
}

void AAgentNPCBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAgentNPCBase, bIsTalkingWithPlayer);
}

void AAgentNPCBase::ForShortAnswer()
{
	if (IsValid(VoiceComp) && IsValid(ForShortAnswerSound))
	{
		VoiceComp->SetSound(ForShortAnswerSound);
		VoiceComp->Play();
		
		float SoundDuration = ForShortAnswerSound->GetDuration();
		GetWorld()->GetTimerManager().SetTimer(VoiceTimerHandle, this, &AAgentNPCBase::OnVoiceFinished, SoundDuration, false);
	}
	
	PRINTLOG_JW(TEXT("[AgentNPC] 너무 짧은 대답 - 정해져 있는 대사 출력"));
}

void AAgentNPCBase::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn) || OtherPawn == this) return;

	if (AMurphyPlayerController* MyPC = Cast<AMurphyPlayerController>(OtherPawn->GetController()))
	{
		if (MyPC->IsLocalController())
		{
			MyPC->SetActiveNPC(this);
			
			// 1 시나리오 매니저 호출 
			if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
			{
				ScenarioSubsystem->StartScenario(NPCScenarioType);
			}
			
			// 2 먼저 말을 거는 NPC
			if (bIsTalkingFirst && IsValid(VoiceComp) && IsValid(PassportSound))
			{
				VoiceComp->SetSound(PassportSound);
				VoiceComp->Play();
				
				float SoundDuration = PassportSound->GetDuration();
				GetWorld()->GetTimerManager().SetTimer(VoiceTimerHandle, this, &AAgentNPCBase::OnVoiceFinished, SoundDuration, false);
			}
			
			// 3 오버랩 직후에는 기본 이모지로 초기화
			UpdateEmotion(EAgentEmotion::Normal);
			EmojiUI->SetEmojiVisible(true);
			bIsScenarioCompleted = false; // 시나리오 시작 
			
			PRINTLOG_JW(TEXT("PC에 현재 Overlap 된 NPC Active (입국심사 시작)."));
		}
	}
}

void AAgentNPCBase::OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn) || OtherPawn == this) return;
	
	if (AMurphyPlayerController* MyPC = Cast<AMurphyPlayerController>(OtherPawn->GetController()))
	{
		MyPC->SetActiveNPC(nullptr);
		
		EmojiUI->SetEmojiVisible(false);
	}
}

void AAgentNPCBase::UpdateEmotion(EAgentEmotion EmotionLevel)
{
	// todo: AnimInstance로 EmotionLevel을 Enum으로 만들어서 분기 처리?
	
	// Enum에 맞는 Texture를 Map에서 찾아옴
	if (TObjectPtr<UTexture2D>* FoundTexture = EmotionTextures.Find(EmotionLevel))
	{
		// 이모지 UI 캐스팅 후 이미지 변경
		if (IsValid(EmojiUI))
		{
			EmojiUI->SetEmoji(*FoundTexture);
		}
	}
}

bool AAgentNPCBase::TryStartConversation()
{
	if (bIsTalkingWithPlayer)
	{
		return false;
	}
	
	bIsTalkingWithPlayer = true;
	return true;
}

void AAgentNPCBase::EndConversation()
{
	bIsTalkingWithPlayer = false;
}

void AAgentNPCBase::StartTypingWait()
{
	bIsWaitingForAIResponse = true;

	if (IsValid(TypingAudioComp) && IsValid(TypingAudioComp->GetSound()))
	{
		TypingAudioComp->Play();
	}
	
	PRINTLOG_JW(TEXT("[AgentNPC] AI 응답 대기 시작 - 타이핑 연출 재생"));
}

void AAgentNPCBase::StopTypingWait()
{
	bIsWaitingForAIResponse = false;

	if (IsValid(TypingAudioComp) && TypingAudioComp->IsPlaying())
	{
		TypingAudioComp->Stop();
	}
	
	PRINTLOG_JW(TEXT("[AgentNPC] AI 응답 대기 종료 - 타이핑 연출 중지"));
}

void AAgentNPCBase::OnVoiceFinished()
{	
	if (bIsScenarioCompleted)
	{
		PRINTLOG_JW(TEXT("[AgentNPC] 시나리오 종료. 1분 대기 타이머를 끄고 상호작용을 종료합니다."));
		
		bIsWaitingForPlayer = false;
		CurWaitTime = 0.0f;
		if (IsValid(EmojiUI))
		{
			EmojiUI->SetProgress(0.0f);
			EmojiUI->SetEmojiVisible(false);
		}
		
		if (IsValid(InteractionBox))
		{
			InteractionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		
		if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
		{
			ScenarioSubsystem->EndScenario(true);
		}
		
		EndConversation();
		return;
	}
	
	// 1분 대기 타이머 시작
	bIsWaitingForPlayer = true;
	CurWaitTime = 0;
	PRINTLOG_JW(TEXT("[AgentNPC] NPC 대사 종료. 1분 대기 타이머를 시작합니다."));
}

void AAgentNPCBase::NotifyPlayerSpoke()
{	
	// 대기 타이머 종료 및 게이지 초기화
	bIsWaitingForPlayer = false;
	if (IsValid(EmojiUI))
	{
		EmojiUI->SetProgress(0.0f);
	}

	StartTypingWait();
}

void AAgentNPCBase::ProcessDialogueResponse(const FAIResponseData& ResponseData)
{
	// AI 서버 응답이 도착했으므로 대기 연출 먼저 종료
	StopTypingWait();

	// 시나리오 종료 판단
	if (ResponseData.next_action == TEXT("FINAL_DECISION") || 
		ResponseData.next_action == TEXT("FAIL_END") || 
		ResponseData.next_node_id == TEXT("IMM_006_DECLARATION_CHECK"))
	{
		bIsScenarioCompleted = true;
	}
	
	// TODO: 추후 AI 팀과 Tone 키워드가 맞춰지면 문자열 파싱 로직으로 복구
	// 현재는 프로토타입 테스트를 위해 0(Normal)부터 6(Furious) 사이의 값을 랜덤하게 추출합니다.
	
	// FMath::RandRange(Min, Max)는 Min과 Max를 포함한 난수를 반환합니다.
	int32 RandomIndex = FMath::RandRange(0, 6);
	EAgentEmotion RandomEmotion = static_cast<EAgentEmotion>(RandomIndex);
	
	PRINTLOG_JW(TEXT("[AgentNPC] 프로토타입 랜덤 감정 출력 -> 인덱스: %d"), RandomIndex);
	
	// 랜덤으로 뽑힌 감정으로 이모지 업데이트
	UpdateEmotion(RandomEmotion);
	
	// TTS 재생
	if (IsValid(VoiceComp) && !ResponseData.npc.audio_url.IsEmpty())
	{
		DownloadAndPlayAudio(ResponseData.npc.audio_url);
	}
	PRINTLOGW_JW(TEXT("[AgentNPC] AI 응답 대사: %s / Tone: %s"), *ResponseData.npc.text, *ResponseData.npc.tone);
}

void AAgentNPCBase::DownloadAndPlayAudio(const FString& AudioURL)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	FString FinalURL = AudioURL;
	if (!FinalURL.StartsWith(TEXT("http")))
	{
		// FinalURL = TEXT("http://172.16.15.36:8000") + FinalURL;
		FinalURL = TEXT("http://127.0.0.1:8000") + FinalURL;
	}
	
	Request->SetURL(FinalURL);
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &AAgentNPCBase::OnAudioDownloaded);
	Request->ProcessRequest();
	
	PRINTLOGW_JW(TEXT("[AgentNPC] 오디오 다운로드 시작: %s"), *FinalURL);
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
	SoundWave->Duration     = static_cast<float>(PCMDataSize) / (SampleRate * NumChannels * (BitsPerSample / 8));
	SoundWave->SoundGroup   = SOUNDGROUP_Voice; // 보이스 그룹으로 설정
	SoundWave->bLooping     = false;
	
	// PCM 데이터를 큐에 적재
	SoundWave->QueueAudio(PCMStart, PCMDataSize);
	
	// VoiceComp에 사운드를 교체하고 재생
	if (IsValid(VoiceComp))
	{
		VoiceComp->SetSound(SoundWave);
		VoiceComp->Play();
		PRINTLOG_JW(TEXT("[AgentNPC] NPC 음성 재생 시작 (%.1f초)"), SoundWave->Duration);
		
		GetWorld()->GetTimerManager().SetTimer(VoiceTimerHandle, this, &AAgentNPCBase::OnVoiceFinished, SoundWave->Duration, false);
	}
}
