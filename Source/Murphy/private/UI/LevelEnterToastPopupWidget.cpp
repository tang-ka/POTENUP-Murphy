// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LevelEnterToastPopupWidget.h"

#include "Components/TextBlock.h"

void ULevelEnterToastPopupWidget::SetUp(const FText& LevelName, float InLifeTime)
{
	if (!Txt_LevelName)
	{
		Txt_LevelName = Cast<UTextBlock>(GetWidgetFromName(TEXT("Txt_LevelName")));
	}

	if (Txt_LevelName)
	{
		Txt_LevelName->SetText(LevelName);
	}
	
	LifeTime = InLifeTime;
}

void ULevelEnterToastPopupWidget::StartLifeTimeCountdown()
{
	GetWorld()->GetTimerManager().SetTimer(
		LifeTimeHandle,
		this,
		&ULevelEnterToastPopupWidget::RemoveFromParent,
		LifeTime,
		false
	);
}
