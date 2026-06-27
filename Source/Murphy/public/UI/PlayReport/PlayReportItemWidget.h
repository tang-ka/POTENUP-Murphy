// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayReportItemWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class MURPHY_API UPlayReportItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	// 항목 이름 영문 (예: Comprehension)
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_CategoryTitleEN;
	
	// 항목 이름 국문 (예: 이해력)
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_CategoryTitleKR;
	
	// 점수
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_Score;
	
	// 원형 ProgressBar
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UImage> img_CircleProgress;
	
	// 머티리얼 파라미터 제어
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicProgressMaterial;
	
	// 점수별 색상 세팅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FLinearColor PerfectColor = FLinearColor(0.2f, 0.8f, 0.4f);		// 100
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FLinearColor ExcellentColor = FLinearColor(0.4f, 0.f, 0.9f);		// 90~99
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FLinearColor GoodColor = FLinearColor(0.1f, 0.5f, 0.9f);			// 70~89
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FLinearColor NormalColor = FLinearColor(0.6f, 0.8f, 0.2f);		// 50~69
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|PlayReport")
	FLinearColor PoorColor = FLinearColor(0.9f, 0.6f, 0.4f);			// 50 미만
	
private:
	// 점수에 따라 알맞은 색상 선택하는 내부 함수
	FLinearColor GetColorByScore(int32 Score) const;
	
public:
	/**
	 * 데이터를 받아 이 항목의 UI를 갱신하는 함수
	 * @param Title 항목 이름
	 * @param Score 0~100 사이의 점수
	 */
	void InitializeItem(const FString& TitleEN, const FString& TitleKR, int32 Score);
};
