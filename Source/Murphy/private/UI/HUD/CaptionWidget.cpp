// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/CaptionWidget.h"
#include "Components/TextBlock.h"

void UCaptionWidget::SetCaption(const FString& CaptionText)
{
	if (Txt_Caption)
	{
		Txt_Caption->SetText(FText::FromString(CaptionText));
	}
}
