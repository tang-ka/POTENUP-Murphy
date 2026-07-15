// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "LevelEnterToastPopupWidget.generated.h"

/**
 * 
 */
UCLASS()
class MURPHY_API ULevelEnterToastPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetUp(const FText& LevelName, float InLifeTime);

	UFUNCTION(BlueprintCallable)
	void StartLifeTimeCountdown();
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_LevelName;
	
	FTimerHandle LifeTimeHandle;
	float LifeTime = 0.f;
};
