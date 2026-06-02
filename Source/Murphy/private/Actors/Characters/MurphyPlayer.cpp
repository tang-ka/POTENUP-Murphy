

#include "Actors/Characters/MurphyPlayer.h"

#include "VoiceChat/VoiceRecorderComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "Murphy.h"


AMurphyPlayer::AMurphyPlayer()
{
	PrimaryActorTick.bCanEverTick = false;
	
	VoiceRecorderComp = CreateDefaultSubobject<UVoiceRecorderComponent>(TEXT("VoiceRecorderComp"));
}

void AMurphyPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMurphyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// APlayerController* PC = Cast<APlayerController>(GetController());
	// if (PC && PC->IsLocalPlayerController())
	// {
	// 	auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	// 	if (Subsystem) Subsystem->AddMappingContext(IMC_Murphy, 0);
	// 	
	// 	auto PlayerInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	// 	if (PlayerInput)
	// 	{
	// 		PlayerInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMurphyPlayer::Move);
	// 		PlayerInput->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &AMurphyPlayer::Look);
	// 		PlayerInput->BindAction(IA_Record, ETriggerEvent::Started, this, &AMurphyPlayer::RecordStart);
	// 		PlayerInput->BindAction(IA_Record, ETriggerEvent::Completed, this, &AMurphyPlayer::RecordEnd);
	// 		PlayerInput->BindAction(IA_PlayAudio, ETriggerEvent::Started, this, &AMurphyPlayer::RecordAudioPlay);
	// 	}
	// }
}

// void AMurphyPlayer::Move(const FInputActionValue& Value)
// {
// }
//
// void AMurphyPlayer::Look(const FInputActionValue& Value)
// {
// }

// void AMurphyPlayer::RecordStart(const FInputActionValue& Value)
// {
// 	if (AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(GetController()))
// 	{
// 		if (IsValid(PC->GetTargetNPC()))
// 		{
// 			PRINTLOGW_JW(TEXT("[VoiceTest] - Start Recording"));
// 			VoiceRecorderComp->StartRecording();
// 		}
// 		else
// 		{
// 			PRINTLOGW_JW(TEXT("[VoiceTest] - NPC가 근처에 없습니다. 녹음을 시작하지 않습니다."));
// 		}
// 	}
// }
//
// void AMurphyPlayer::RecordEnd(const FInputActionValue& Value)
// {
// 	if (VoiceRecorderComp->IsRecording())
// 	{
// 		PRINTLOGW_JW(TEXT("[VoiceTest] - Stop & Save"));
// 		VoiceRecorderComp->StopRecording(TEXT("TestRecording"), true);
// 	}
// }
//
// void AMurphyPlayer::RecordAudioPlay(const FInputActionValue& Value)
// {
// 	PRINTLOGW_JW(TEXT("[VoiceTest] - Play"));
// 	VoiceRecorderComp->PlayRecordedSamples();
// }
