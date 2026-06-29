// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Phone/WidgetWidget.h"
#include "DateAppWidget.generated.h"

class UTextBlock;

UCLASS()
class MURPHY_API UDateAppWidget : public UWidgetWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Day;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Date;
};
