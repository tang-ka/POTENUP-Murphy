// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceChat/VoiceRecorderComponent.h"


// Sets default values for this component's properties
UVoiceRecorderComponent::UVoiceRecorderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UVoiceRecorderComponent::StartRecording()
{
}

void UVoiceRecorderComponent::StopRecording(const FString& FileName, bool bSaveToWav)
{
}

void UVoiceRecorderComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UVoiceRecorderComponent::SaveToWav(const FString& FileName)
{
}


