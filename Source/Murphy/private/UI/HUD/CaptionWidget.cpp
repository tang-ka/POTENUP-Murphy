// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/CaptionWidget.h"
#include "Components/TextBlock.h"

void UCaptionWidget::SetCaption(const FString& CaptionText)
{
	if (!Txt_Caption)
	{
		return;
	}

	Txt_Caption->SetText(FText::FromString(CaptionText));
}

void UCaptionWidget::ClearCaption()
{
	if (Txt_Caption)
	{
		Txt_Caption->SetText(FText::GetEmpty());
	}

	SetCaptionVisible(false);
}

void UCaptionWidget::SetCaptionVisible(bool bIsVisible)
{
	SetVisibility(bIsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
}
