// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/PlayReportData.h"
#include "FeedbackCardWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class MURPHY_API UFeedbackCardWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_Title;					// 카드 제목					
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_Summary;					// 한글 핵심 문법 요약
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_OriginalUtterances;		// 유저가 말한 오답
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_SuggestedExpressions;	// 모범 영어 예문
	
	// UPROPERTY(meta =(BindWidget))
	// TObjectPtr<UTextBlock> txt_PracticePrompt;			// 복습 작문 한글 프롬포트
	// 
	// UPROPERTY(meta =(BindWidget))
	// TObjectPtr<UTextBlock> txt_AnswerExample;			// 작문 모범 영어 예문
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UImage> img_Star1;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UImage> img_Star2;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UImage> img_Star3;
	
public:
	// 최종 결과 피드백 카드 데이터를 받아 UI를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Murphy|PlayReport")
	void InitializeCard(const FPlayReportFeedbackCardData& CardData);
	
};
