// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayReport/FeedbackCardWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

namespace
{
int32 GetStarCountFromPriority(const FString& Priority)
{
	const FString NormalizedPriority = Priority.TrimStartAndEnd().ToLower();
	if (NormalizedPriority.IsEmpty())
	{
		return 0;
	}

	if (NormalizedPriority.Contains(TEXT("high")) || NormalizedPriority.Contains(TEXT("critical")) || NormalizedPriority.Contains(TEXT("p1")))
	{
		return 3;
	}

	if (NormalizedPriority.Contains(TEXT("medium")) || NormalizedPriority.Contains(TEXT("normal")) || NormalizedPriority.Contains(TEXT("p2")))
	{
		return 2;
	}

	if (NormalizedPriority.Contains(TEXT("low")) || NormalizedPriority.Contains(TEXT("p3")))
	{
		return 1;
	}

	return 1;
}

void SetTextBlockText(UTextBlock* TextBlock, const FString& Text)
{
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(Text));
	}
}

void SetStarVisibility(UImage* StarImage, bool bVisible)
{
	if (StarImage)
	{
		StarImage->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
}

void UFeedbackCardWidget::InitializeCard(const FPlayReportFeedbackCardData& CardData)
{
	SetTextBlockText(txt_Title, CardData.Title);
	SetTextBlockText(txt_Summary, CardData.Summary);
	SetTextBlockText(txt_OriginalUtterances, FString::Join(CardData.OriginalUtterances, TEXT("\n")));
	SetTextBlockText(txt_SuggestedExpressions, FString::Join(CardData.SuggestedExpressions, TEXT("\n")));
	SetTextBlockText(txt_PracticePrompt, CardData.PracticePrompt);
	SetTextBlockText(txt_AnswerExample, CardData.AnswerExample);

	const int32 StarCount = GetStarCountFromPriority(CardData.Priority);
	SetStarVisibility(img_Star1, StarCount >= 1);
	SetStarVisibility(img_Star2, StarCount >= 2);
	SetStarVisibility(img_Star3, StarCount >= 3);
}
