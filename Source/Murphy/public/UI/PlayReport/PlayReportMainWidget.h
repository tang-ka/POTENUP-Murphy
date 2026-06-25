// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayReportMainWidget.generated.h"

class UTextBlock;
class UPlayReportItemWidget;

/**
 * 게임 결과창 전체를 관리하는 마스터 위젯 클래스
 */

UCLASS()
class MURPHY_API UPlayReportMainWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// [상단 영역] 총점과 티어 표시
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_TotalScore;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_TierName;

	// [하단 영역] 6개의 세부 평가 항목 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_TaskSuccess;		// 과업 성공

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Clarity;			// 명확성
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Grammar;			// 문법

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Vocabulary;		// 어휘

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_ProblemSolving;	// 문제 해결
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Politeness;		// 공손함



public:
	// 외부(게임모드 등)에서 데이터를 주입하여 UI를 갱신하는 함수
	UFUNCTION(BlueprintCallable, Category = "Murphy|PlayReport")
	void DisplayReport(const FAI_Evaluation& EvaluationData, int32 TotalScore, const FString& TierName);
};
