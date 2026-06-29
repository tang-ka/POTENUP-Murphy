
#include "Actors/Characters/AgentNPCBase.h"
#include "Actors/Characters/MurphyPlayer.h"

#include "Animation/WonFaceAnimInstance.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/QuestEventNotifyComponent.h"
#include "Components/WidgetComponent.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/MurphyGameModeBase.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpManager.h"
#include "HttpModule.h"                       // 오디오 다운로드용
#include "UI/HUD/MainHUD.h"
#include "UI/AgentEmojiUI.h"
#include "UObject/UnrealType.h"
#include "Murphy.h"
#include "Manager/DataManager.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundWaveProcedural.h"        // 런타임 사운드 생성용
#include "Settings/MurphyNetSettings.h"
#include "Kismet/GameplayStatics.h"

AAgentNPCBase::AAgentNPCBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	
	// FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
	// FaceMesh->SetupAttachment(GetMesh());
	
	VoiceComp = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceComp"));
	VoiceComp->bAutoActivate = false;
	VoiceComp->EnvelopeFollowerAttackTime = 5;
	VoiceComp->EnvelopeFollowerReleaseTime = 60;
	// VoiceComp->SetupAttachment(FaceMesh);
	
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

	QuestEventNotifier = CreateDefaultSubobject<UQuestEventNotifyComponent>(TEXT("QuestEventNotifier"));
}

void AAgentNPCBase::BeginPlay()
{
	Super::BeginPlay();

	CachedFaceMesh = ResolveFaceMeshComponent();
	if (CachedFaceMesh)
	{
		if (CachedFaceMesh != FaceMesh)
		{
			PRINTLOG_JW(TEXT("[AgentNPC] 실제 Face 컴포넌트 연결: %s"), *CachedFaceMesh->GetName());
		}

		if (IsValid(VoiceComp))
		{
			VoiceComp->AttachToComponent(CachedFaceMesh, FAttachmentTransformRules::KeepRelativeTransform);
		}
		
		CachedFaceAnimInstance = CachedFaceMesh->GetAnimInstance();
		if (CachedFaceAnimInstance)
		{
			CachedFaceLoudnessFloat = FindFProperty<FFloatProperty>(CachedFaceAnimInstance->GetClass(), TEXT("FaceLoudness"));
			CachedFaceLoudnessDouble = FindFProperty<FDoubleProperty>(CachedFaceAnimInstance->GetClass(), TEXT("FaceLoudness"));
		}
	}
	
	CachedCurrentLoudnessFloat = FindFProperty<FFloatProperty>(GetClass(), TEXT("CurrentLoudness"));
	CachedCurrentLoudnessDouble = FindFProperty<FDoubleProperty>(GetClass(), TEXT("CurrentLoudness"));
	
	EmojiUI = Cast<UAgentEmojiUI>(EmojiComp->GetUserWidgetObject());
	EmojiUI->SetEmojiVisible(false);
	EmojiUI->SetNPCName(TEXT("BBung"));
	if (!NPCName.IsNone()) EmojiUI->SetNPCName(NPCName.ToString());
	
	if (QuestTargetID.IsNone()) PRINTLOGE_JW(TEXT("!!!!Quest ID 를 지정해주세요!!!!")); 
	if (QuestEventNotifier && !QuestTargetID.IsNone())
	{
		// 기존 NPC BP가 가진 QuestTargetID 값을 공통 퀘스트 통보 컴포넌트에 동기화합니다.
		QuestEventNotifier->SetQuestTargetID(QuestTargetID);
	}

	if (IsValid(TypingSound)) TypingAudioComp->SetSound(TypingSound);

	InitializeSessionState();

	if (IsValid(VoiceComp))
	{
		VoiceComp->OnAudioSingleEnvelopeValue.RemoveDynamic(this, &AAgentNPCBase::OnVoiceEnvelopeValue);
		VoiceComp->OnAudioSingleEnvelopeValue.AddDynamic(this, &AAgentNPCBase::OnVoiceEnvelopeValue);
	}
	
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
		FVector CameraLoc = FVector::ZeroVector;
		bool bGotCamera = false;
		
		// 로컬 클라이언트의 카메라 위치를 가져옵니다.
		if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
		{
			if (APlayerCameraManager* CameraManager = LocalPC->PlayerCameraManager)
			{
				CameraLoc = CameraManager->GetCameraLocation();
				bGotCamera = true;
			}
		}

		if (bGotCamera && EmojiComp->GetWidgetSpace() == EWidgetSpace::Screen)
		{
			FVector WidgetLoc = EmojiComp->GetComponentLocation();
			// [Screen 모드] 거리에 따라 위젯 스케일 작아지게
			float Distance = FVector::Dist(CameraLoc, WidgetLoc);
			float Scale = FMath::Clamp(300.0f / FMath::Max(Distance, 1.0f), 0.1f, 1.0f);
			
			if (IsValid(EmojiUI))
			{
				EmojiUI->SetRenderScale(FVector2D(Scale, Scale));
			}
		}
	}
	
	bool bShouldLookAtPlayer = bIsLookingAtPlayer; 

	// NPC가 몽타주(문서 보기 등)를 재생 중인지 검사 *임시 지금은 잠시 꺼두기
	// if (bIsLookingAtPlayer)
	// {
	// 	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	// 	GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	// 	for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
	// 	{
	// 		if (SkelMesh->GetName().Equals(TEXT("Body"), ESearchCase::IgnoreCase))
	// 		{
	// 			if (UAnimInstance* AnimInst = SkelMesh->GetAnimInstance())
	// 			{
	// 				// 몽타주 재생 중이라면 ➔ "지금 바쁘니까 시선 꺼!"
	// 				if (AnimInst->IsAnyMontagePlaying())
	// 				{
	// 					bShouldLookAtPlayer = false; 
	// 				}
	// 			}
	// 			break;
	// 		}
	// 	}
	// }

	// 계산된 결과를 ABP가 읽어갈 수 있도록 멤버 변수에 저장
	bEnableIK = bShouldLookAtPlayer;
    
	// 최종적으로 쳐다보는 것이 승인되었을 때만 플레이어 좌표 갱신 (서버에서만 갱신 후 리플리케이트)
	if (bEnableIK && IsValid(CurrentInteractPlayer) && HasAuthority())
	{
		if (APawn* PlayerPawn = Cast<APawn>(CurrentInteractPlayer))
		{
			// 1. 플레이어가 ACharacter를 상속받는다고 가정하고 캐스팅합니다.
			if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
			{
				// 2. 플레이어의 스켈레탈 메시에서 'head' 뼈의 월드 좌표를 가져옵니다. 
				// (대부분의 언리얼 기본/메타휴먼 뼈대에서 미간 위치와 정확히 일치합니다.)
				TargetLookAtLocation = PlayerCharacter->GetMesh()->GetSocketLocation(FName("head"));
			}
			else
			{
				// 캐릭터가 아니라면 임시로 액터 중심점 + 눈높이(약 160)를 타겟으로 삼습니다.
				TargetLookAtLocation = PlayerPawn->GetActorLocation() + FVector(0.0f, 0.0f, 160.0f);
			}
		}
	}
	
	if (bIsWaitingForPlayer)
	{
		CurWaitTime += DeltaSeconds;
		
		if (IsValid(EmojiUI) && GetNetMode() != NM_DedicatedServer)
		{
			EmojiUI->SetProgress(CurWaitTime / MaxWaitTime);
		}
		
		if (CurWaitTime >= MaxWaitTime && HasAuthority())
		{
			bIsWaitingForPlayer = false;
			
			PRINTLOGW_JW(TEXT("[AgentNPC] 1분 타임아웃! 플레이어가 대답하지 않았습니다."));
			
			AActor* TargetPlayer = IsValid(CurrentInteractPlayer) ? CurrentInteractPlayer.Get() : LastInteractPlayer.Get();
			if (TargetPlayer)
			{
				if (APawn* TargetPawn = Cast<APawn>(TargetPlayer))
				{
					if (AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(TargetPawn->GetController()))
					{
						PC->SendTimeoutAudioToAI();
					}
				}
			}
		}
	}
	
}

void AAgentNPCBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAgentNPCBase, bIsTalkingWithPlayer);
	DOREPLIFETIME(AAgentNPCBase, CurrentEmotion);
	DOREPLIFETIME(AAgentNPCBase, bIsLookingAtPlayer);
	DOREPLIFETIME(AAgentNPCBase, TargetLookAtLocation);
}

void AAgentNPCBase::ForShortAnswer()
{
	if (IsValid(VoiceComp) && IsValid(ForShortAnswerSound))
	{
		VoiceComp->SetSound(ForShortAnswerSound);
		VoiceComp->Play();
		
		float SoundDuration = ForShortAnswerSound->GetDuration();
		GetWorld()->GetTimerManager().ClearTimer(VoiceTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(VoiceTimerHandle, this, &AAgentNPCBase::OnVoiceFinished, SoundDuration, false);
	}
	
	PRINTLOG_JW(TEXT("[AgentNPC] 너무 짧은 대답 - 정해져 있는 대사 출력"));
}

void AAgentNPCBase::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn) || OtherPawn == this) return;

	if (bIsScenarioCompleted)
	{
		return; 
	}
	
	if (AMurphyPlayerController* MyPC = Cast<AMurphyPlayerController>(OtherPawn->GetController()))
	{
		if (MyPC->IsLocalController())
		{
			MyPC->SetActiveNPC(this);
			
			// 대화 시작 시 시선 IK 스위치를 켭니다.
			bIsLookingAtPlayer = true;
    
			// 플레이어 액터를 저장해둡니다. (매개변수로 넘어온 OtherActor가 플레이어라고 가정)
			CurrentInteractPlayer = OtherActor;
			LastInteractPlayer = OtherActor;

			if (!QuestTargetID.IsNone())
			{
				// NPC 접근은 대화 완료가 아니라 "대화 시작 조건"만 만족한 것으로 서버에 알립니다.
				if (QuestEventNotifier)
				{
					QuestEventNotifier->SetQuestTargetID(QuestTargetID);
					QuestEventNotifier->NotifyQuestStart(OtherPawn, EQuestCondition::TalkToNPC);
				}
			}
			
			//! 이거 대신에서 Overlap되면 퀘스트가 깨지는 걸 넣어야 할 듯
			// 1 시나리오 매니저 호출 
			// if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
			// {
			// 	ScenarioSubsystem->StartScenario(NPCScenarioType);
			// }
			
			// 2 먼저 말을 거는 NPC
			if (bIsTalkingFirst && IsValid(VoiceComp) && IsValid(PassportSound))
			{
				VoiceComp->SetSound(PassportSound);
				VoiceComp->Play();

				float SoundDuration = PassportSound->GetDuration();
				GetWorld()->GetTimerManager().ClearTimer(VoiceTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(VoiceTimerHandle, this, &AAgentNPCBase::OnVoiceFinished, SoundDuration, false);

				// 캐싱 음성 재생 = 대화 시작 -> 카테고리 생성 + 첫 Agent 대사
				if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(OtherPawn))
				{
					if (UMainHUD* MainHUD = MurphyPlayer->GetMainHUD())
					{
						MainHUD->BeginTranslateConversation(GetScenarioCategoryName(), FText::FromName(NPCName));
						MainHUD->AddAgentDialog(NPCName.ToString(), LastNpcMessage);
					}
				}
			}
			
			// 3 오버랩 직후에는 기본 이모지로 초기화
			UpdateEmotion(EAgentEmotion::Normal);
			EmojiUI->SetEmojiVisible(true);
			// bIsScenarioCompleted = false; // 시나리오 시작 
			
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
	
	// 플레이어가 박스 밖으로 나가면 즉시 시선을 거둡니다.
	bIsLookingAtPlayer = false;
	CurrentInteractPlayer = nullptr;
}

void AAgentNPCBase::SetQuestTargetID(FName InQuestTargetID)
{
	QuestTargetID = InQuestTargetID;

	if (QuestEventNotifier)
	{
		QuestEventNotifier->SetQuestTargetID(QuestTargetID);
	}
}

// 서버 문자열을 엔진 Enum으로 변환
EAgentEmotion AAgentNPCBase::ConvertStringToEmotion(const FString& EmotionString)
{
	if (EmotionString.Equals(TEXT("Normal"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Normal;
	if (EmotionString.Equals(TEXT("Nomal"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Normal;
	if (EmotionString.Equals(TEXT("Joy"),		 ESearchCase::IgnoreCase))	return EAgentEmotion::Joy;
	if (EmotionString.Equals(TEXT("Anger"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Anger;
	if (EmotionString.Equals(TEXT("Sadness"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Sadness;
	if (EmotionString.Equals(TEXT("Panic"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Panic;
	if (EmotionString.Equals(TEXT("Suspicion"), ESearchCase::IgnoreCase))	return EAgentEmotion::Suspicion;
	if (EmotionString.Equals(TEXT("Disgust"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Disgust;
	if (EmotionString.Equals(TEXT("Fear"),		 ESearchCase::IgnoreCase))	return EAgentEmotion::Fear;
	if (EmotionString.Equals(TEXT("Smirk"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Smirk;
	if (EmotionString.Equals(TEXT("Surprise"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Surprise;
	if (EmotionString.Equals(TEXT("Pain"),		 ESearchCase::IgnoreCase))	return EAgentEmotion::Pain;
	if (EmotionString.Equals(TEXT("Confusion"), ESearchCase::IgnoreCase))	return EAgentEmotion::Confusion;
	if (EmotionString.Equals(TEXT("Boredom"),	 ESearchCase::IgnoreCase))	return EAgentEmotion::Boredom;

	PRINTLOGE_JW(TEXT("[AgentNPC] 알 수 없는 감정 키워드 수신: %s"), *EmotionString);
	return EAgentEmotion::Normal;
}

USkeletalMeshComponent* AAgentNPCBase::ResolveFaceMeshComponent() const
{
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
	{
		if (IsValid(SkelMesh) && Cast<UWonFaceAnimInstance>(SkelMesh->GetAnimInstance()))
		{
			return SkelMesh;
		}
	}

	const FString TargetFaceName = FaceComponentName.ToString();
	for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
	{
		if (IsValid(SkelMesh) && !FaceComponentName.IsNone() && SkelMesh->GetName().Equals(TargetFaceName, ESearchCase::IgnoreCase))
		{
			return SkelMesh;
		}
	}

	for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
	{
		if (IsValid(SkelMesh) && !FaceComponentName.IsNone() && SkelMesh->GetName().Contains(TargetFaceName, ESearchCase::IgnoreCase))
		{
			return SkelMesh;
		}
	}

	for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
	{
		if (IsValid(SkelMesh) && !FaceComponentTag.IsNone() && SkelMesh->ComponentHasTag(FaceComponentTag))
		{
			return SkelMesh;
		}
	}

	return IsValid(FaceMesh) ? FaceMesh.Get() : nullptr;
}

void AAgentNPCBase::OnVoiceEnvelopeValue(const USoundWave* PlayingSoundWave, const float EnvelopeValue)
{
	const float Loudness = FMath::Clamp(EnvelopeValue * 8.0f, 0.0f, 1.0f);

	if (CachedCurrentLoudnessFloat) CachedCurrentLoudnessFloat->SetPropertyValue_InContainer(this, Loudness);
	else if (CachedCurrentLoudnessDouble) CachedCurrentLoudnessDouble->SetPropertyValue_InContainer(this, Loudness);

	if (CachedFaceAnimInstance)
	{
		if (CachedFaceLoudnessFloat) CachedFaceLoudnessFloat->SetPropertyValue_InContainer(CachedFaceAnimInstance, Loudness);
		else if (CachedFaceLoudnessDouble) CachedFaceLoudnessDouble->SetPropertyValue_InContainer(CachedFaceAnimInstance, Loudness);
	}
}

void AAgentNPCBase::SetFloatPropertyIfExists(UObject* TargetObject, FName PropertyName, float Value) const
{
	if (!IsValid(TargetObject))
	{
		return;
	}

	if (FFloatProperty* FloatProperty = FindFProperty<FFloatProperty>(TargetObject->GetClass(), PropertyName))
	{
		FloatProperty->SetPropertyValue_InContainer(TargetObject, Value);
	}
	else if (FDoubleProperty* DoubleProperty = FindFProperty<FDoubleProperty>(TargetObject->GetClass(), PropertyName))
	{
		DoubleProperty->SetPropertyValue_InContainer(TargetObject, Value);
	}
}

void AAgentNPCBase::UpdateEmotion(EAgentEmotion EmotionLevel)
{
	// 1. 상태 변수 최신화 (애니메이션 블루프린트에서 매 프레임 읽어갈 데이터)
	CurrentEmotion = EmotionLevel;

	// 서버도 비주얼 갱신이 필요하다면 로컬 함수를 즉시 호출
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_CurrentEmotion();
	}
}

void AAgentNPCBase::OnRep_CurrentEmotion()
{
	const FString EmotionString = StaticEnum<EAgentEmotion>()->GetNameStringByValue(static_cast<int64>(CurrentEmotion));
	const FName EmotionRowName(*EmotionString);
	const FAI_EmotionData* EmotionData = nullptr;

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UDataManager* DataManager = GameInstance->GetSubsystem<UDataManager>())
		{
			EmotionData = DataManager->GetEmotionData(EmotionRowName);
		}
	}

	// 2. 페이셜 애니메이션 연동: DataTable Row의 MuscleValues를 얼굴 AnimBP에 전달
	if (EmotionData)
	{
		if (EmotionData->MuscleValues.IsEmpty())
		{
			PRINTLOGW_JW(TEXT("[AgentNPC] 감정 Row는 찾았지만 MuscleValues가 비어 있습니다: %s"), *EmotionString);
		}

		if (CachedFaceAnimInstance)
		{
			if (UWonFaceAnimInstance* FaceAnimInst = Cast<UWonFaceAnimInstance>(CachedFaceAnimInstance))
			{
				FaceAnimInst->TargetEmotionMap = EmotionData->MuscleValues;
				PRINTLOG_JW(TEXT("[AgentNPC] 얼굴 표정 데이터 적용 -> Emotion:%s MuscleCount:%d"), *EmotionString, EmotionData->MuscleValues.Num());
			}
		}
	}
    
	// 3. 이모지 UI 업데이트 로직
	UTexture2D* EmotionTexture = nullptr;
	if (EmotionData && !EmotionData->EmotionTextures.IsNull())
	{
		EmotionTexture = EmotionData->EmotionTextures.LoadSynchronous();
	}
	if (!EmotionTexture)
	{
		if (TObjectPtr<UTexture2D>* FoundTexture = EmotionTextures.Find(CurrentEmotion))
		{
			EmotionTexture = *FoundTexture;
		}
	}

	if (IsValid(EmojiUI) && IsValid(EmotionTexture))
	{
		EmojiUI->SetEmoji(EmotionTexture);
	}

	// 4. 일회성 감정 표현(애니메이션 몽타주) 재생 로직
	UAnimMontage* EmotionMontage = nullptr;
	if (EmotionData && !EmotionData->EmotionMontages.IsNull())
	{
		EmotionMontage = EmotionData->EmotionMontages.LoadSynchronous();
	}
	if (!EmotionMontage)
	{
		if (TObjectPtr<UAnimMontage>* FoundMontage = EmotionMontages.Find(CurrentEmotion))
		{
			EmotionMontage = *FoundMontage;
		}
	}

	if (IsValid(EmotionMontage))
	{
		// 메타휴먼은 얼굴(Face), 몸통(Body) 등 부위가 나뉘어 있으므로 모든 컴포넌트를 순회하며 재생합니다.
		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

		for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
		{
			if (!IsValid(SkelMesh) || SkelMesh == CachedFaceMesh)
			{
				continue;
			}

			if (UAnimInstance* AnimInst = SkelMesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(EmotionMontage);
			}
		}
       
		// 성공 로그는 콘솔창이 지저분해지지 않게 딱 한 줄만 깔끔하게 남깁니다.
		PRINTLOG_CW(TEXT("[AgentNPC] 감정 몽타주 재생 -> %s"), *EmotionMontage->GetName());
	}
}

FName AAgentNPCBase::GetScenarioCategoryName() const
{
	return FName(*StaticEnum<EScenarioType>()->GetNameStringByValue((int64)NPCScenarioType));
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
	if (CachedCurrentLoudnessFloat) CachedCurrentLoudnessFloat->SetPropertyValue_InContainer(this, 0.0f);
	else if (CachedCurrentLoudnessDouble) CachedCurrentLoudnessDouble->SetPropertyValue_InContainer(this, 0.0f);

	if (CachedFaceAnimInstance)
	{
		if (CachedFaceLoudnessFloat) CachedFaceLoudnessFloat->SetPropertyValue_InContainer(CachedFaceAnimInstance, 0.0f);
		else if (CachedFaceLoudnessDouble) CachedFaceLoudnessDouble->SetPropertyValue_InContainer(CachedFaceAnimInstance, 0.0f);
	}

	if (IsValid(VoiceComp))
	{
		VoiceComp->SetSound(nullptr);
	}

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
		
		AActor* QuestInstigator = CurrentInteractPlayer;
		if (!IsValid(QuestInstigator))
		{
			// 대화 중 플레이어 포인터가 정리된 예외 상황을 대비한 로컬 컨트롤러 폴백입니다.
			QuestInstigator = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		}

		if (QuestEventNotifier && IsValid(QuestInstigator) && !QuestTargetID.IsNone())
		{
			// AI 대화가 최종 종료된 뒤에만 TalkToNPC 완료 조건을 서버로 보냅니다.
			QuestEventNotifier->SetQuestTargetID(QuestTargetID);
			QuestEventNotifier->NotifyQuestComplete(QuestInstigator, EQuestCondition::TalkToNPC);
		}

		bIsLookingAtPlayer = false;
		CurrentInteractPlayer = nullptr;
				
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
		ResponseData.next_node_id == TEXT("IMM_006_DECLARATION_CHECK")) // 이거는 입국심사때의 마지막 노드 
	{
		bIsScenarioCompleted = true;
	}
	
	// todo 욕을 하는 경우 끝남 -> 이건 게임오버 종료 조건 
	if (ResponseData.next_node_id == TEXT("IMM_BAD_END_VERBAL_ABUSE"))
	{
		PRINTLOGW_JW(TEXT("[AgentNPC] 욕으로 인한 시나리오 중단!!"));
		bIsScenarioCompleted = true;

		// 서버인 경우 GameMode에 게임 오버 상태를 트리거합니다.
		if (HasAuthority())
		{
			if (AMurphyGameModeBase* GM = Cast<AMurphyGameModeBase>(GetWorld()->GetAuthGameMode()))
			{
				GM->TriggerGameOver();
			}
		}
	}
	
	// ==========================================================
	// 🔴 [임시 테스트 코드] 서버 데이터 대신 감정 순서대로 무한 순환하기
	// ==========================================================
	// static int32 TestEmotionIndex = 0; // 함수가 끝나도 숫자가 리셋되지 않고 유지됩니다.

	// 현재 인덱스를 Enum 타입으로 강제 변환
	// EAgentEmotion DummyEmotion = static_cast<EAgentEmotion>(TestEmotionIndex);
    
	// 우리가 만든 소화 기관에 쏙 넣어주기 (이모지 변경 + ABP 변수 최신화)
	// UpdateEmotion(DummyEmotion);
    
	// PRINTLOGW_JW(TEXT("[AgentNPC][TEST] 임시 감정 순환 가동 중 -> Index: %d"), TestEmotionIndex);

	// 대답이 끝날 때마다 다음 감정 인덱스로 1씩 증가 (총 7개 감정이므로 0~6까지만 돌고 다시 0으로)
	// TestEmotionIndex = (TestEmotionIndex + 1) % 7;
	// ==========================================================
	
	//===========================================================
	// 🔴 랜덤 로직 삭제 & 서버 감정 연동 (서버가 정보 보내주면 복구할 곳)
	//===========================================================
	FString ServerEmotion = ResponseData.npc.emotion; 
	EAgentEmotion ParsedEmotion = ConvertStringToEmotion(ServerEmotion);
	
	// 파싱된 진짜 감정으로 변수와 이모지 동시 업데이트
	UpdateEmotion(ParsedEmotion);
	PRINTLOG_JW(TEXT("[AgentNPC] 감정 동기화 완료 -> %s (Enum Index: %d)"), *ServerEmotion, (int32)ParsedEmotion);
	//===========================================================
	
	// TTS 재생
	if (IsValid(VoiceComp) && !ResponseData.npc.audio_url.IsEmpty())
	{
		DownloadAndPlayAudio(ResponseData.npc.audio_url);
	}
	PRINTLOGW_JW(TEXT("[AgentNPC] 응답 대사: %s / 목소리 톤: %s"), *ResponseData.npc.text, *ResponseData.npc.tone);
}

void AAgentNPCBase::DownloadAndPlayAudio(const FString& AudioURL)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	// 서버가 자기 기준 localhost(127.0.0.1)로 내려준 절대 URL을 실제 도달 가능한 호스트로 치환.
	// 상대경로면 호스트를 앞에 붙인다. (UMurphyNetSettings 참조)
	const FString FinalURL = GetDefault<UMurphyNetSettings>()->ResolveAudioURL(AudioURL);

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
	int16 NumChannels = 0;
	int32 SampleRate = 0;
	int16 BitsPerSample = 0;

	FMemory::Memcpy(&NumChannels, Raw + 22, 2);
	FMemory::Memcpy(&SampleRate, Raw + 24, 4);
	FMemory::Memcpy(&BitsPerSample, Raw + 34, 2);
	// const int32 PCMDataSize   = *reinterpret_cast<const int32*>(Raw + 40);
	// const uint8* PCMStart     = Raw + 44;
	
	uint32 PCMDataSize = 0;
	uint32 PCMOffset   = 0;
	uint32 Offset      = 12;

	while (Offset + 8 <= static_cast<uint32>(WavData.Num()))
	{
		const uint8* ChunkId   = Raw + Offset;
		uint32 ChunkSize = 0;
		FMemory::Memcpy(&ChunkSize, Raw + Offset + 4, 4);

		if (ChunkId[0] == 'd' && ChunkId[1] == 'a' && ChunkId[2] == 't' && ChunkId[3] == 'a')
		{
			PCMDataSize = ChunkSize;
			PCMOffset   = Offset + 8;
			break;
		}

		Offset += 8 + ChunkSize;
		if (ChunkSize % 2 != 0) Offset += 1;
	}

	if (PCMOffset == 0)
	{
		PRINTLOGE_JW(TEXT("[AgentNPC] WAV data 청크를 찾을 수 없음"));
		return;
	}

	const uint8* PCMStart = Raw + PCMOffset;
	
	// 방어 코드: 실제 데이터 크기와 헤더 명시 크기 비교
	// if (WavData.Num() < 44 + PCMDataSize)
	if (static_cast<uint32>(WavData.Num()) < PCMOffset + PCMDataSize)
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
		
		GetWorld()->GetTimerManager().ClearTimer(VoiceTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(VoiceTimerHandle, this, &AAgentNPCBase::OnVoiceFinished, SoundWave->Duration, false);
	}
}


// ==========================================
// [AI Session]
// ==========================================

void AAgentNPCBase::InitializeSessionState()
{
	CurrentSessionId = FString::Printf(TEXT("session_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Short));
	CurrentNodeId = InitialNodeId;
	TurnIndex = 1;
	LastNpcMessage = InitialNpcMessage;
	CurrentScenarioState = FAI_ScenarioState();
	
	PRINTLOG_JW(TEXT("[AgentNPC] 세션 상태 초기화: %s / Node: %s"), *CurrentSessionId, *CurrentNodeId);
}

void AAgentNPCBase::UpdateSessionStateFromResponse(const FAIResponseData& ResponseData)
{
	LastNpcMessage = ResponseData.npc.text;
	
	CurrentScenarioState.patience += ResponseData.state_delta.patience_delta;
	CurrentScenarioState.suspicion += ResponseData.state_delta.suspicion_delta;
	CurrentScenarioState.retry_count += ResponseData.state_delta.retry_count_delta;
	CurrentScenarioState.hint_count += ResponseData.state_delta.hint_count_delta;
	
	TurnIndex += 1;
	
	// if (!ResponseData.current_node_id.IsEmpty())
	// {
	// 	CurrentNodeId = ResponseData.current_node_id;
	// }

	PRINTLOG_JW(TEXT("[AgentNPC] 시나리오 (Action: %s)"), *ResponseData.next_action);
	if (ResponseData.next_action == TEXT("ADVANCE") && !ResponseData.next_node_id.IsEmpty())
	{
		CurrentNodeId = ResponseData.next_node_id;
		
		//. 수하물 Info NPC와의 시나리오 끝 노드 
		if (CurrentNodeId == TEXT("BAG_004_STAFF_REDIRECT_TO_CUSTOMS_HOLD"))
		{
			if (IsValid(InteractionBox))
			{
				InteractionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				PRINTLOG_JW(TEXT("[AgentNPC] 시나리오 종료됨 (Action: %s). InteractionBox 비활성화."), *ResponseData.next_action);
			}
		
			bIsScenarioCompleted = true;
		}
	}
	
	// 시나리오가 종료되었을 때 InteractionBox를 끕니다. (더 이상 대화할 수 없도록)
	// if (ResponseData.next_action == TEXT("END") || ResponseData.next_action == TEXT("COMPLETE_CHAPTER") || ResponseData.next_action == TEXT("SUCCESS") || ResponseData.next_action == TEXT("FAIL"))
	if (ResponseData.next_action == TEXT("COMPLETE_CHAPTER")) // BaggageClaim 에서 info랑 대화하는 부분 체크
	{
		//. 비행기에서 시나리오 끝난 경우 억까 상황 받아오기
		if (ResponseData.next_node_id == TEXT("FLIGHT_999_COMPLETE"))
		{
			if (APawn* InteractingPawn = Cast<APawn>(CurrentInteractPlayer))
			{
				if (AMurphyPlayerState* PS = InteractingPawn->GetPlayerState<AMurphyPlayerState>())
				{
					// 백엔드에서 받은 세관 데이터를 PlayerState의 단일 진입점으로 저장합니다.
					// PS->SetCustomsAssignment(
					// 	ResponseData.customs_data.assigned_visit_location,
					// 	ResponseData.customs_data.random_customs_item);
					//
					// PRINTLOG_JW(TEXT("⚠️억까 상황 : %s / %s"), *ResponseData.customs_data.assigned_visit_location, *ResponseData.customs_data.random_customs_item);
					
					
					// 백엔드에서 받은 세관 데이터를 PlayerState의 단일 진입점으로 저장합니다.
					PS->SetCustomsAssignment(
						ResponseData.game_state.assigned_visit_location_id,
						ResponseData.game_state.random_customs_item_id);
					
					PRINTLOG_JW(TEXT("⚠️억까 상황 : %s / %s"), *ResponseData.game_state.assigned_visit_location_id, *ResponseData.game_state.random_customs_item_id);
				}
			}
		}
		
		if (IsValid(InteractionBox))
		{
			// InteractionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// PRINTLOG_JW(TEXT("[AgentNPC] 시나리오 종료됨 (Action: %s). InteractionBox 비활성화."), *ResponseData.next_action);
		}
		
		bIsScenarioCompleted = true;
	}
}
