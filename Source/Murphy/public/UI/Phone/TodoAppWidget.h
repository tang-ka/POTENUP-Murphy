// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Phone/WidgetWidget.h"
#include "TodoAppWidget.generated.h"

class UCheckBox;
class UImage;

UCLASS()
class MURPHY_API UTodoAppWidget : public UWidgetWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Check_First;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_StrikeThrough1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Check_Second;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_StrikeThrough2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Check_Third;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_StrikeThrough3;

private:
	UFUNCTION()
	void OnFirstCheckChanged(bool bIsChecked);

	UFUNCTION()
	void OnSecondCheckChanged(bool bIsChecked);

	UFUNCTION()
	void OnThirdCheckChanged(bool bIsChecked);
};
