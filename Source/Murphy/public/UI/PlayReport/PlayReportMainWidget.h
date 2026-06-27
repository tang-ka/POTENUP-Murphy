// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/PlayReportData.h"
#include "PlayReportMainWidget.generated.h"

class UImage;
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
	TObjectPtr<UTextBlock> txt_Result;				// 성공 여부
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_TotalScore;			// 총점
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> img_Tier;					// 티어 이미지
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_TierName;			// 티어명(영어)

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_TravelerTitle;		// 칭호
	
	// [하단 영역] 6개의 세부 평가 항목 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Comprehension;	// 이해도

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Fluency;			// 유창성
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Grammar;			// 문법성

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Vocabulary;		// 어휘력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_Clarity;			// 명확성
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayReportItemWidget> item_ProblemSolving;	// 문제해결력

public:
	UFUNCTION(BlueprintCallable, Category = "Murphy|PlayReport")
	void DisplayReport(const FPlayReportData& ReportData);
};
