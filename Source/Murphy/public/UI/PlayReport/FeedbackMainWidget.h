// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FeedbackMainWidget.generated.h"

struct FPlayReportData;
class UTextBlock;
class UScrollBox;
class UFeedbackCardWidget;
/**
 *  2페이지: 상세 텍스트 피드백 화면을 담당하는 위젯
 */
UCLASS()
class MURPHY_API UFeedbackMainWidget : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	// 동적으로 생성된 피드백 카드들이 붙을 스크롤 박스
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UScrollBox> sb_FeedbackCards;

	// 블루프린트에서 할당할 카드 위젯 클래스 (틀)
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|PlayReport")
	TSubclassOf<UFeedbackCardWidget> FeedbackCardClass;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_Overall;				// 최종 총평

	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_MainImprovement;		// 핵심 개선 가이드
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_NextPracticePrompt;	// 다음 플레이 추천 연습 문제
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_NextAnswerExample;	// 다음 플레이 추천 모범 답안
	
public:
	void DisplayFeedback(const FPlayReportData& ReportData);
};
