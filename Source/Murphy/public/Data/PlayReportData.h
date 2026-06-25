// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayReportData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct MURPHY_API FPlayReportData
{
	GENERATED_BODY()

public:
	// 티어
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FString TierName;
	
	// 총 점수 (항목 평균)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 TotalScore = 0;
	
	// 이해력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 ComprehensionScore = 0;
	
	// 유창성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 FluencyScore = 0;
	
	// 문법 정확도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 GrammarAccuracyScore = 0;
	
	// 어휘력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 VocabularyRangeScore = 0;
	
	// 의도 명확성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 ClarityScore = 0;
	
	// 시나리오 해결 능력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	int32 InteractionProblemSolvingScore = 0;
	
	// 최종 추천 결과
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
};
