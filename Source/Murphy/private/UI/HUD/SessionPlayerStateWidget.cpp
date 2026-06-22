// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/SessionPlayerStateWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void USessionPlayerStateWidget::SetPlayerName(const FString& InPlayerName)
{
	if (Txt_PlayerName)
	{
		Txt_PlayerName->SetText(FText::FromString(InPlayerName));
	}
}

void USessionPlayerStateWidget::SetReadyState(bool bInReady)
{
	if (Txt_PlayerReady)
	{
		const FText NewText = bInReady
			? FText::FromString(TEXT("준비 완료"))
			: FText::FromString(TEXT("준비 중"));
		Txt_PlayerReady->SetText(NewText);
	}

	if (Img_ReadyIcon)
	{
		Img_ReadyIcon->SetVisibility(bInReady ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
