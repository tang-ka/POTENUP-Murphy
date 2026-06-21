// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayReport/PlayReportMainWidget.h"

#include "UI/PlayReport/PlayReportItemWidget.h" 
#include "Components/TextBlock.h"

void UPlayReportMainWidget::DisplayReport(const FPlayReportData& ReportData)
{
	// 1. 총점 및 티어 업데이트
	if (txt_TotalScore)
	{
		txt_TotalScore->SetText(FText::FromString(FString::Printf(TEXT("%d"), ReportData.TotalScore)));
	}

	if (txt_TierName)
	{
		txt_TierName->SetText(FText::FromString(ReportData.TierName));
	}

	// 2. 하위 세부 위젯들에 개별 데이터 주입
	// Item 위젯의 InitializeItem(항목 이름, 점수) 함수 호출
    
	if (item_Comprehension)
	{
		item_Comprehension->InitializeItem(TEXT("이해력"), ReportData.ComprehensionScore);
	}

	if (item_Fluency)
	{
		item_Fluency->InitializeItem(TEXT("유창성"), ReportData.FluencyScore);
	}

	if (item_GrammarAccuracy)
	{
		item_GrammarAccuracy->InitializeItem(TEXT("문법정확도"), ReportData.GrammarAccuracyScore);
	}

	if (item_VocabularyRange)
	{
		item_VocabularyRange->InitializeItem(TEXT("어휘력"), ReportData.VocabularyRangeScore);
	}

	if (item_Clarity)
	{
		item_Clarity->InitializeItem(TEXT("의도명확성"), ReportData.ClarityScore);
	}

	if (item_ProblemSolving)
	{
		item_ProblemSolving->InitializeItem(TEXT("시나리오해결"), ReportData.InteractionProblemSolvingScore);
	}
}