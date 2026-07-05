// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestToastPopupWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * WBP_QuestToastPopup 와 연결되는 위젯 클래스.
 */
UCLASS()
class MURPHY_API UQuestToastPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetUp(const FText& Title, const FText& Content, float InLifeTime);

	UFUNCTION(BlueprintCallable)
	void SetIcon(UTexture2D* IconTexture);

	UFUNCTION(BlueprintCallable)
	void StartLifeTimeCountdown();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Content;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Pop;
	
private:
	FTimerHandle LifeTimeHandle;
	float LifeTime = 0.f;
};
