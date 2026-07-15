// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayReport/PlayReportMainWidget.h"

#include "Components/TextBlock.h"
#include "UI/PlayReport/PlayReportItemWidget.h" 


void UPlayReportMainWidget::DisplayReport(const FPlayReportData& ReportData)
{
	// === 상단 영역 (총점, 티어) 갱신 ===
	if (txt_TotalScore)
	{
		txt_TotalScore->SetText(FText::AsNumber(ReportData.TotalScore));
	}

	if (txt_TierName)
	{
		txt_TierName->SetText(FText::FromString(ReportData.TierName));
	}

	// === 하단 세부 항목 점수 ===
	if (item_Comprehension)
	{
		item_Comprehension->InitializeItem(TEXT("이해력"), ReportData.ComprehensionScore);
	}

	if (item_Fluency)
	{
		item_Fluency->InitializeItem(TEXT("유창성"), ReportData.FluencyScore);
	}

	if (item_Grammar)
	{
		item_Grammar->InitializeItem(TEXT("문법성"), ReportData.GrammarScore);
	}

	if (item_Vocabulary)
	{
		item_Vocabulary->InitializeItem(TEXT("어휘력"), ReportData.VocabularyScore);
	}

	if (item_Clarity)
	{
		item_Clarity->InitializeItem(TEXT("명확성"), ReportData.ClarityScore);
	}

	if (item_ProblemSolving)
	{
		item_ProblemSolving->InitializeItem(TEXT("문제해결력"), ReportData.ProblemSolvingScore);
	}
	
}
