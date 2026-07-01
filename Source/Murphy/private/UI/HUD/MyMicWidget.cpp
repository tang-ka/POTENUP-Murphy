// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/MyMicWidget.h"

void UMyMicWidget::SetRecordingState(bool bIsRecording)
{
	if (bIsRecording)
	{
		// SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		PlayAnimation(ShowMic, 0, 1, EUMGSequencePlayMode::Forward);
	}
	else
	{
		// SetVisibility(ESlateVisibility::Hidden); 
		PlayAnimation(ShowMic, 0, 1, EUMGSequencePlayMode::Reverse);
	}
}

void UMyMicWidget::RecordingAnimation(bool bIsRecording)
{
	if (bIsRecording)
	{
		PlayAnimation(Recording, 0, 0, EUMGSequencePlayMode::PingPong);
	}
	else
	{
		StopAnimation(Recording);
	}
}
