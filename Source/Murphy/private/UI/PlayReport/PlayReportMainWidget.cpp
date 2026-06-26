// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayReport/PlayReportMainWidget.h"

#include "UI/PlayReport/PlayReportItemWidget.h" 
#include "Components/TextBlock.h"
#include "Data/AIDataTypes.h"

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

	// 2. 최종 결과 API 기준 점수 데이터 주입
	if (item_TaskSuccess)
	{
		item_TaskSuccess->InitializeItem(TEXT("과업성공"), ReportData.TaskSuccessScore);
	}

	if (item_Clarity)
	{
		item_Clarity->InitializeItem(TEXT("명확성"), ReportData.ClarityScore);
	}

	if (item_Grammar)
	{
		item_Grammar->InitializeItem(TEXT("문법정확도"), ReportData.GrammarScore);
	}

	if (item_Vocabulary)
	{
		item_Vocabulary->InitializeItem(TEXT("어휘력"), ReportData.VocabularyScore);
	}

	if (item_ProblemSolving)
	{
		item_ProblemSolving->InitializeItem(TEXT("상황해결력"), ReportData.ProblemSolvingScore);
	}

	if (item_Politeness)
	{
		item_Politeness->InitializeItem(TEXT("유창성"), ReportData.FluencyScore);
	}
}

void UPlayReportMainWidget::DisplayEvaluationReport(const FAI_Evaluation& EvaluationData, int32 TotalScore, const FString& TierName)
{
	// 1. 총점 및 티어 업데이트
	if (txt_TotalScore)
	{
		txt_TotalScore->SetText(FText::FromString(FString::Printf(TEXT("%d"), TotalScore)));
	}

	if (txt_TierName)
	{
		txt_TierName->SetText(FText::FromString(TierName));
	}

	// 2. 하위 세부 위젯들에 개별 데이터 주입
	// Item 위젯의 InitializeItem(항목 이름, 점수) 함수 호출
    
	if (item_TaskSuccess)
	{
		item_TaskSuccess->InitializeItem(TEXT("과업성공"), EvaluationData.scores.task_success);
	}

	if (item_Clarity)
	{
		item_Clarity->InitializeItem(TEXT("명확성"), EvaluationData.scores.clarity);
	}

	if (item_Grammar)
	{
		item_Grammar->InitializeItem(TEXT("문법정확도"), EvaluationData.scores.grammar);
	}

	if (item_Vocabulary)
	{
		item_Vocabulary->InitializeItem(TEXT("어휘력"), EvaluationData.scores.vocabulary);
	}

	if (item_ProblemSolving)
	{
		item_ProblemSolving->InitializeItem(TEXT("상황해결력"), EvaluationData.scores.problem_solving);
	}

	if (item_Politeness)
	{
		item_Politeness->InitializeItem(TEXT("공손함"), EvaluationData.scores.politeness);
	}
}
