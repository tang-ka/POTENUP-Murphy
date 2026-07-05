// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuestToastPopupWidget.h"

#include "Murphy.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UQuestToastPopupWidget::SetUp(const FText& Title, const FText& Content, float InLifeTime)
{
	if (Txt_Title)
	{
		Txt_Title->SetText(Title);
	}
	else
	{
		PRINTLOG_SH(TEXT("Txt_Title is nullptr."));
	}

	if (Txt_Content)
	{
		Txt_Content->SetText(Content);
	}
	else
	{
		PRINTLOG_SH(TEXT("Txt_Content is nullptr."));
	}

	LifeTime = InLifeTime;
}

void UQuestToastPopupWidget::SetIcon(UTexture2D* IconTexture)
{
	if (!Img_Icon)
	{
		PRINTLOG_SH(TEXT("Img_Icon is nullptr."));
		return;
	}

	if (!IconTexture)
	{
		PRINTLOG_SH(TEXT("IconTexture is nullptr."));
		return;
	}

	Img_Icon->SetBrushFromTexture(IconTexture);
}

void UQuestToastPopupWidget::StartLifeTimeCountdown()
{
	PlayAnimation(Pop);
	GetWorld()->GetTimerManager().SetTimer(
		LifeTimeHandle,
		this,
		&UQuestToastPopupWidget::RemoveFromParent,
		LifeTime,
		false
	);
}
