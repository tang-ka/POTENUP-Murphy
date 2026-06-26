// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayReportData.generated.h"

/**
 * 최종 결과창 피드백 카드 하나에 표시할 데이터입니다.
 */
USTRUCT(BlueprintType)
struct MURPHY_API FPlayReportFeedbackCardData
{
	GENERATED_BODY()

public:
	// 카드 제목
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString Title;

	// 한글 핵심 규칙 요약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString Summary;

	// 유저가 실제로 말한 표현
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	TArray<FString> OriginalUtterances;

	// 추천 영어 표현
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	TArray<FString> SuggestedExpressions;

	// 복습용 한글 프롬프트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString PracticePrompt;

	// 복습 모범 답안
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString AnswerExample;

	// 중요도. high/medium/low 값이면 별 개수로 표시합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString Priority;
};

/**
 * 최종 결과창 전체에 표시할 데이터입니다.
 */
USTRUCT(BlueprintType)
struct MURPHY_API FPlayReportData
{
	GENERATED_BODY()

public:
	// 티어
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString TierName;
	
	// 최종 총점
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 TotalScore = 0;
	
	// 이해도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 ComprehensionScore = 0;
	
	// 유창성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 FluencyScore = 0;
	
	// 문법성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 GrammarScore = 0;
	
	// 어휘력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 VocabularyScore = 0;
	
	// 명확성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 ClarityScore = 0;
	
	// 문제해결력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 ProblemSolvingScore = 0;
	
	// 최종 추천 결과 (P/NP)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString FinalRecommendation;

	// 표시용 랭크 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString Rank;

	// 최종 총평
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString OverallSummary;

	// 게임 밖 피드백 한글 총평
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString OverallSummaryKr;

	// 핵심 개선 가이드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString MainImprovement;

	// 가장 잘 수행한 노드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString BestNode;

	// 가장 약했던 노드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString WeakestNode;

	// 다음 플레이 추천 연습 문제
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString NextPracticePromptKr;

	// 다음 플레이 추천 모범 답안
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString NextAnswerExample;

	// 표현/문법 교정 카드 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	TArray<FPlayReportFeedbackCardData> FeedbackCards;
};
