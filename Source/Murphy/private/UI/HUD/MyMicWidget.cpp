// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/MyMicWidget.h"

void UMyMicWidget::SetRecordingState(bool bIsRecording)
{
	if (bIsRecording)
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden); 
	}
}
