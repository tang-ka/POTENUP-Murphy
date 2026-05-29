// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceChat/VoiceChatActor.h"

#include "Murphy.h"
#include "Components/InputComponent.h"
#include "VoiceChat/VoiceRecorderComponent.h"


// Sets default values
AVoiceChatActor::AVoiceChatActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	VoiceRecorderComp = CreateDefaultSubobject<UVoiceRecorderComponent>(TEXT("VoiceRecorderComp"));
	
	//! AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AVoiceChatActor::BeginPlay()
{
	Super::BeginPlay();
}

void AVoiceChatActor::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 레거시 BindKey로 빠르게 테스트 (EnhancedInput 안 거치고)
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AVoiceChatActor::OnPressRecord);
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &AVoiceChatActor::OnPressStop);
	PlayerInputComponent->BindKey(EKeys::Y, IE_Pressed, this, &AVoiceChatActor::OnPressPlay);
}

void AVoiceChatActor::OnPressRecord()
{
	PRINTLOG_SH(TEXT("[VoiceTest] R - Start Recording"));
	VoiceRecorderComp->StartRecording();
}

void AVoiceChatActor::OnPressStop()
{
	PRINTLOG_SH(TEXT("[VoiceTest] T - Stop & Save"));
	VoiceRecorderComp->StopRecording(TEXT("TestRecording"), true);
}

void AVoiceChatActor::OnPressPlay()
{
	PRINTLOG_SH(TEXT("[VoiceTest] Y - Play"));
	VoiceRecorderComp->PlayRecordedSamples();
}

