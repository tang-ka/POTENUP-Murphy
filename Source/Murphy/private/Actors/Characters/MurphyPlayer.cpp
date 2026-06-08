

#include "Actors/Characters/MurphyPlayer.h"

#include "VoiceChat/VoiceRecorderComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "Actors/Characters/AgentNPCBase.h"
#include "Framework/MurphyPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Murphy.h"
#include "UI/HUD/MainHUD.h"


AMurphyPlayer::AMurphyPlayer()
{
	PrimaryActorTick.bCanEverTick = true; // 채팅 중 카메라 보간을 위해 
	
	VoiceRecorderComp = CreateDefaultSubobject<UVoiceRecorderComponent>(TEXT("VoiceRecorderComp"));
}

void AMurphyPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	// MainHUDClassInstance 생성
	if (IsLocallyControlled() && MainHUDClass != nullptr)
	{
		MainHUDInstance = CreateWidget<UMainHUD>(GetWorld(), MainHUDClass);
		if (MainHUDInstance != nullptr)
		{
			MainHUDInstance->AddToViewport();
		}
	}
	
}

void AMurphyPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// NPC와 대화하는 중일 경우 
	if (bIsAligningWithNPC  && TargetNPC) FocusNPC(DeltaSeconds);
}

void AMurphyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalPlayerController())
	{
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (Subsystem) Subsystem->AddMappingContext(IMC_Murphy, 0);
		
		auto PlayerInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
		if (PlayerInput)
		{
			PlayerInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMurphyPlayer::Move);
			PlayerInput->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &AMurphyPlayer::Look);
			PlayerInput->BindAction(IA_Record, ETriggerEvent::Started, this, &AMurphyPlayer::RecordStart);
			PlayerInput->BindAction(IA_Record, ETriggerEvent::Completed, this, &AMurphyPlayer::RecordEnd);
			PlayerInput->BindAction(IA_PlayAudio, ETriggerEvent::Started, this, &AMurphyPlayer::RecordAudioPlay);
			PlayerInput->BindAction(IA_ToggleBag, ETriggerEvent::Started, this, &AMurphyPlayer::ToggleBagPressed);
			PlayerInput->BindAction(IA_TogglePhone, ETriggerEvent::Started, this, &AMurphyPlayer::TogglePhonePressed);
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
			bIsAligningWithNPC = true;
			bPendingEndChat = false;
		}
	}
}

void AMurphyPlayer::EndChatWithNPC()
{
	if (bIsAligningWithNPC)
	{
		bPendingEndChat = true;
		return;
	}
	
	if (TargetNPC)
	{
		TargetNPC->EndConversation();
		TargetNPC = nullptr;
	}
	
	bPendingEndChat = false;
	SetChatState(EPlayerChatState::Idle);
}

void AMurphyPlayer::FocusNPC(float DeltaSeconds)
{
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetNPC->GetActorLocation());
	TargetRot.Pitch = 0.0f;
	TargetRot.Roll = 0.0f;
		
	FRotator Rot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds,  5.0f);
	SetActorRotation(Rot);
		
	bool bActorAligned = GetActorRotation().Equals(TargetRot, 2.0f);
	bool bCamAligned = true;
		
	// 카메라는 특정 시점으로 고정 시킬 수 있도륙
	if (Controller)
	{	
		FRotator CamRot = FMath::RInterpTo(Controller->GetControlRotation(), CamTargetRot, DeltaSeconds,  5.0f);
		Controller->SetControlRotation(CamRot);
			
		bCamAligned = Controller->GetControlRotation().Equals(CamTargetRot, 2.0f);
	}
		
	if (bActorAligned && bCamAligned)
	{
		bIsAligningWithNPC  = false;
			
		if (bPendingEndChat)
		{
			EndChatWithNPC();
		}
	}
}

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
	uint8 b = CurChatState == EPlayerChatState::WaitingForAI || CurChatState == EPlayerChatState::Recording ||  CurChatState == EPlayerChatState::Talking;
	if (b)
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

void AMurphyPlayer::SetMicUIState(bool bIsRecording)
{
	if (MainHUDInstance != nullptr)
	{
		MainHUDInstance->UpdateMicState(bIsRecording);
	}
}
