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
	
	
};