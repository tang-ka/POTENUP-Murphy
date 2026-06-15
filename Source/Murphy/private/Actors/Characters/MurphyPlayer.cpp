

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Items/ItemBaseActor.h"

#include "VoiceChat/VoiceRecorderComponent.h"
#include "Components/PlayerViewComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"

#include "Actors/Characters/AgentNPCBase.h"
#include "Framework/InteractableInterface.h"
#include "Components/CapsuleComponent.h"
#include "Framework/MurphyPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Murphy.h"
#include "Camera/CameraComponent.h"
#include "UI/SystemMenuUI.h"
#include "UI/HUD/MainHUD.h"


AMurphyPlayer::AMurphyPlayer()
{
	// PrimaryActorTick.bCanEverTick = true; // 채팅 중 카메라 보간을 위해 -> PlayerViewComponent::TickComponent로 이전, Actor Tick 불필요

	VoiceRecorderComp = CreateDefaultSubobject<UVoiceRecorderComponent>(TEXT("VoiceRecorderComp"));

	PlayerViewComp = CreateDefaultSubobject<UPlayerViewComponent>(TEXT("PlayerViewComp"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 100.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetRelativeLocation(FVector(0.0f, 30.0f, 0.0f));
}

void AMurphyPlayer::BeginPlay()
{
	Super::BeginPlay();

	// GameMode 단위 ChatViewMode 설정을 읽어와 덮어씀 (GameMode 없으면 기존 EditAnywhere 기본값 사용)
	if (AMurphyGameModeBase* MurphyGameMode = GetWorld()->GetAuthGameMode<AMurphyGameModeBase>())
	{
		ChatViewMode = MurphyGameMode->ChatViewMode;
		PRINTLOG_SH(TEXT("BeginPlay: GameMode ChatViewMode(%d) 적용"), static_cast<int32>(ChatViewMode));
	}

	if (PlayerViewComp)
	{
		PlayerViewComp->OnViewTransitionComplete.AddDynamic(this, &AMurphyPlayer::HandleViewTransitionComplete);

		// 기내 씬 등: 시작부터 1인칭 자유시점 고정
		if (ChatViewMode == EChatViewMode::FirstPersonLocked)
		{
			PlayerViewComp->RequestViewState(EPlayerViewState::FirstPersonTalk);
		}
	}

	// MainHUDClassInstance 생성
	if (IsLocallyControlled() && MainHUDClass != nullptr)
	{
		MainHUDInstance = CreateWidget<UMainHUD>(GetWorld(), MainHUDClass);
		if (MainHUDInstance != nullptr)
		{
			MainHUDInstance->AddToViewport();
		}
	}

	// SystemMenuInstance 생성
	if (IsLocallyControlled() && SystemMenuClass != nullptr)
	{
		SystemMenuInstance = CreateWidget<USystemMenuUI>(GetWorld(), SystemMenuClass);
		if (SystemMenuInstance != nullptr)
		{
			SystemMenuInstance->AddToViewport(100);
			SystemMenuInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// void AMurphyPlayer::Tick(float DeltaSeconds)
// {
// 	Super::Tick(DeltaSeconds);
//
// 	if (bIsAligningWithNPC && TargetNPC) FocusNPC(DeltaSeconds);
// }
// -> PlayerViewComponent::TickComponent + TickThirdPersonFocusAlign으로 이전됨

void AMurphyPlayer::SetMovementLocked(bool bLocked)
{
	// 서버 권위 값 세팅
	bMovementLocked = bLocked;

	// 소유 클라이언트로 RPC 전송 (서버에서만 유효)
	if (HasAuthority())
	{
		Client_SetMovementLocked(bLocked);
	}
}

void AMurphyPlayer::Client_SetMovementLocked_Implementation(bool bLocked)
{
	bMovementLocked = bLocked;
}

void AMurphyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalPlayerController())
	{
		if (auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Murphy, 0);
		}

		if (auto PlayerInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			PlayerInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMurphyPlayer::Move);
			PlayerInput->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &AMurphyPlayer::Look);
			PlayerInput->BindAction(IA_Record, ETriggerEvent::Started, this, &AMurphyPlayer::RecordStart);
			PlayerInput->BindAction(IA_Record, ETriggerEvent::Completed, this, &AMurphyPlayer::RecordEnd);
			PlayerInput->BindAction(IA_PlayAudio, ETriggerEvent::Started, this, &AMurphyPlayer::RecordAudioPlay);
			PlayerInput->BindAction(IA_ToggleBag, ETriggerEvent::Started, this, &AMurphyPlayer::ToggleBagPressed);
			PlayerInput->BindAction(IA_TogglePhone, ETriggerEvent::Started, this, &AMurphyPlayer::TogglePhonePressed);
			PlayerInput->BindAction(IA_Interact, ETriggerEvent::Started, this, &AMurphyPlayer::InteractPressed);	// F키
			PlayerInput->BindAction(IA_SystemMenu, ETriggerEvent::Started, this, &AMurphyPlayer::SystemMenuPressed);
		}
	}
}

void AMurphyPlayer::StartChatWithNPC(AAgentNPCBase* NPC)
{
	if (NPC && CurChatState == EPlayerChatState::Idle)
	{
		if (!NPC->CanTalkWithPlayer())
		{
			PRINTLOGW_JW(TEXT("[Chat] 해당 NPC는 다른 플레이어와 대화중임"));
			return;
		}

		if (NPC->TryStartConversation())
		{
			TargetNPC = NPC;
			SetChatState(EPlayerChatState::Talking);
			// bIsAligningWithNPC = true; // PlayerViewComp::RequestViewState로 이전
			// bPendingEndChat = false; // PlayerViewComp::PendingViewState로 이전

			if (PlayerViewComp)
			{
				switch (ChatViewMode)
				{
				case EChatViewMode::ThirdPersonFocus:
					{
						PlayerViewComp->RequestViewState(EPlayerViewState::ThirdPersonFocus, NPC);
						break;
					}
				case EChatViewMode::FirstPersonTalk:
					{
						PlayerViewComp->RequestViewState(EPlayerViewState::FirstPersonTalk);
						break;
					}
				case EChatViewMode::FirstPersonLocked:
					{
						// 이미 1인칭 고정 상태 - 시점 변경 없음
						break;
					}
				}
			}
		}
	}
}

void AMurphyPlayer::EndChatWithNPC()
{
	// if (bIsAligningWithNPC) // PlayerViewComp::IsTransitioning()으로 이전
	// {
	// 	bPendingEndChat = true;
	// 	return;
	// }

	// ThirdPersonFocus 모드에서 정렬 중이면 종료 처리를 보류 (정렬 완료 시 HandleViewTransitionComplete에서 재호출)
	if (ChatViewMode == EChatViewMode::ThirdPersonFocus && PlayerViewComp && PlayerViewComp->IsTransitioning())
	{
		PlayerViewComp->RequestViewState(EPlayerViewState::Idle);
		return;
	}

	if (TargetNPC)
	{
		TargetNPC->EndConversation();
		TargetNPC = nullptr;
	}

	// bPendingEndChat = false; // 더 이상 사용 안 함
	SetChatState(EPlayerChatState::Idle);

	if (PlayerViewComp && ChatViewMode != EChatViewMode::FirstPersonLocked)
	{
		PlayerViewComp->RequestViewState(EPlayerViewState::Idle);
	}
}

void AMurphyPlayer::HandleViewTransitionComplete(EPlayerViewState ReachedState)
{
	if (ReachedState == EPlayerViewState::Idle && CurChatState != EPlayerChatState::Idle)
	{
		EndChatWithNPC();
	}
}

// void AMurphyPlayer::FocusNPC(float DeltaSeconds)
// {
// 	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetNPC->GetActorLocation());
// 	TargetRot.Pitch = 0.0f;
// 	TargetRot.Roll = 0.0f;
//
// 	FRotator Rot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds,  5.0f);
// 	SetActorRotation(Rot);
//
// 	bool bActorAligned = GetActorRotation().Equals(TargetRot, 2.0f);
// 	bool bCamAligned = true;
//
// 	// 카메라는 특정 시점으로 고정 시킬 수 있도륙
// 	if (Controller)
// 	{
// 		FRotator CamRot = FMath::RInterpTo(Controller->GetControlRotation(), CamTargetRot, DeltaSeconds,  5.0f);
// 		Controller->SetControlRotation(CamRot);
//
// 		bCamAligned = Controller->GetControlRotation().Equals(CamTargetRot, 2.0f);
// 	}
//
// 	if (bActorAligned && bCamAligned)
// 	{
// 		bIsAligningWithNPC  = false;
//
// 		if (bPendingEndChat)
// 		{
// 			EndChatWithNPC();
// 		}
// 	}
// }
// -> PlayerViewComponent::TickThirdPersonFocusAlign으로 이전됨

float AMurphyPlayer::GetRecordTime() const
{
	if (GetWorld())
	{
		return GetWorld()->GetTimeSeconds() - RecordTime;
	}
	
	return 0.0f;
}

void AMurphyPlayer::Move(const FInputActionValue& Value)
{
	if (bMovementLocked)
	{
		return;
	}

	if (CurChatState == EPlayerChatState::WaitingForAI || CurChatState == EPlayerChatState::Recording ||  CurChatState == EPlayerChatState::Talking)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMurphyPlayer::Look(const FInputActionValue& Value)
{
	if (CurChatState == EPlayerChatState::WaitingForAI || CurChatState == EPlayerChatState::Recording)
	{
		return;
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMurphyPlayer::RecordStart(const FInputActionValue& Value)
{
	if (AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(GetController()))
	{
		AAgentNPCBase* OverlappedNPC = PC->GetTargetNPC();
		if (!IsValid(OverlappedNPC))
		{
			PRINTLOGW_JW(TEXT("[VoiceTest] - NPC가 근처에 없습니다. 녹음을 시작하지 않습니다."));
			return;
		}
		
		// 플레이어가 Idle 상태일 때만 대화 시작 시도
		if (CurChatState == EPlayerChatState::Idle)
		{
			if (!OverlappedNPC->CanTalkWithPlayer())
			{
				PRINTLOGW_JW(TEXT("[Chat] 해당 NPC는 다른 플레이어와 대화중입니다."));
				return;
			}
			
			StartChatWithNPC(OverlappedNPC);
		}

		PRINTLOGW_JW(TEXT("[VoiceTest] - Start Recording"));
		VoiceRecorderComp->StartRecording();
		SetChatState(EPlayerChatState::Recording);
		RecordTime = GetWorld()->GetTimeSeconds();
	}
}

void AMurphyPlayer::RecordEnd(const FInputActionValue& Value)
{
	if (VoiceRecorderComp->IsRecording())
	{
		PRINTLOGW_JW(TEXT("[VoiceTest] - Stop & Save"));
		VoiceRecorderComp->StopRecording(TEXT("TestRecording"), true);
		
		// 녹음 종료 후 AI 응답 대기 상태로 변경
		SetChatState(EPlayerChatState::WaitingForAI);
	}
}

void AMurphyPlayer::RecordAudioPlay(const FInputActionValue& Value)
{
	PRINTLOGW_JW(TEXT("[VoiceTest] - Play"));
	VoiceRecorderComp->PlayRecordedSamples();
}

void AMurphyPlayer::ToggleBagPressed()
{
	if (MainHUDInstance != nullptr)
	{
		MainHUDInstance->RequestToggleBag();
	}
}

void AMurphyPlayer::TogglePhonePressed()
{
	if (MainHUDInstance != nullptr)
	{
		MainHUDInstance->RequestTogglePhone();
	}
}

void AMurphyPlayer::InteractPressed()
{
	TArray<AActor*> OverlappingActors;
	GetCapsuleComponent()->GetOverlappingActors(OverlappingActors);

	AActor* ClosestActor = nullptr;
	float ClosestDistSq = MAX_flt;

	FVector ViewLoc = GetActorLocation();
	FRotator ViewRot = GetActorRotation();
	if (Controller)
	{
		Controller->GetPlayerViewPoint(ViewLoc, ViewRot);
	}

	const FVector ViewForward = ViewRot.Vector();
	const FVector PlayerLoc = GetActorLocation(); // 거리는 캐릭터 중심 기준 유지
	const float ThresholdCos = FMath::Cos(FMath::DegreesToRadians(InteractAngleDeg));

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->Implements<UInteractableInterface>())
		{
			// 카메라 위치에서 아이템 방향으로의 시야각 체크
			FVector ToActor = (Actor->GetActorLocation() - ViewLoc).GetSafeNormal();
			float DotResult = FVector::DotProduct(ViewForward, ToActor);

			if (DotResult >= ThresholdCos)
			{
				float DistSq = FVector::DistSquared(PlayerLoc, Actor->GetActorLocation());
				if (DistSq < ClosestDistSq)
				{
					ClosestDistSq = DistSq;
					ClosestActor = Actor;
				}
			}
		}
	}

	if (ClosestActor)
	{
		if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(ClosestActor))
		{
			Interactable->Interact(this);
		}
	}
}

void AMurphyPlayer::SystemMenuPressed()
{
	if (SystemMenuClass == nullptr) return;
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || SystemMenuInstance == nullptr) return;
	
	if (SystemMenuInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		// 닫기
		SystemMenuInstance->SetVisibility(ESlateVisibility::Hidden);
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
	else
	{
		// 열기
		SystemMenuInstance->SetVisibility(ESlateVisibility::Visible);
		PC->bShowMouseCursor = true;
		
		// 캐릭터의 입력 막고 UI만 포커싱
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(SystemMenuInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

void AMurphyPlayer::SetMicUIState(bool bIsRecording)
{
	if (MainHUDInstance != nullptr)
	{
		MainHUDInstance->UpdateMicState(bIsRecording);
	}
}
