// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScenarioCategoryButtonWidget.generated.h"

class UTextBlock;
class UCheckBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategorySelected, FName, CategoryName);

UCLASS()
class MURPHY_API UScenarioCategoryButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ScenarioName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Tgl_Category;

	UPROPERTY(BlueprintAssignable, Category = "Translate")
	FOnCategorySelected OnCategorySelected;

	void SetCategoryData(FName InCategoryName, const FText& InDisplayName);
	void SetSelected(bool bSelected);
	bool IsCategorySelected() const;

private:
	FName CategoryName;
	bool bIsUpdatingSelection = false;

	void UpdateTextColor(bool bIsSelected);

	UFUNCTION()
	void OnCategoryChanged(bool bIsChecked);
};
