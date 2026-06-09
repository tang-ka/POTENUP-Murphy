// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AgentEmojiUI.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;

UCLASS()
class MURPHY_API UAgentEmojiUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void SetProgress(float ProgressValue);
	void SetProgressColor(FLinearColor Color);
	void SetEmoji(UTexture2D* Emoji) const; 
	void SetNPCName(FString NPCName) const;
	
	void SetEmojiVisible(bool bIsVisible);
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_EmotionGuage;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Emoji;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_NPCName;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> AppearEmoji;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMat;
	
	float Progress{0.0f}; 
	FLinearColor PBColor{FLinearColor(0.799f, 0.06f, 0.001f)};
};
