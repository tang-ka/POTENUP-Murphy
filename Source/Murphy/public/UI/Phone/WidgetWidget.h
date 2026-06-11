// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class MURPHY_API UWidgetWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Widget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_WidgetName;
};

