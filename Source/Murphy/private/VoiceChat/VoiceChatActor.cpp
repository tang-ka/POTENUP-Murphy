// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceChat/VoiceChatActor.h"

#include "VoiceChat/VoiceRecorderComponent.h"


// Sets default values
AVoiceChatActor::AVoiceChatActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	VoiceRecorderComp = CreateDefaultSubobject<UVoiceRecorderComponent>(TEXT("VoiceRecorderComponent"));
}

void AVoiceChatActor::BeginPlay()
{
	Super::BeginPlay();
}

void AVoiceChatActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

