// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/TranslateTypes.h"
#include "TranslateDialogManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogAdded, FName, CategoryName, const FDialogEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCategoryRegistered, FName, CategoryName, const FText&, DisplayName);

UCLASS()
class MURPHY_API UTranslateDialogManager : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FOnDialogAdded OnDialogAdded;

	UPROPERTY()
	FOnCategoryRegistered OnCategoryRegistered;

public:
	void AddDialog(FName InCategoryName, const FDialogEntry& Entry);
	void RegisterCategory(FName InCategoryName, const FText& DisplayName);
	bool HasCategory(FName InCategoryName) const;
	const TArray<FDialogEntry>* GetDialogs(FName InCategoryName) const;
	const TMap<FName, FText>& GetCategoryDisplayNameMap() const;

private:
	TMap<FName, TArray<FDialogEntry>> DialogDataMap;
	TMap<FName, FText> CategoryDisplayNameMap;
};
