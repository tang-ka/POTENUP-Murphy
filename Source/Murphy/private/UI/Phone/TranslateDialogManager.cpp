// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/TranslateDialogManager.h"

#include "Murphy.h"

void UTranslateDialogManager::RegisterCategory(FName InCategoryName, const FText& DisplayName)
{
	if (!DialogDataMap.Contains(InCategoryName))
	{
		DialogDataMap.Add(InCategoryName, {});
		CategoryDisplayNameMap.Add(InCategoryName, DisplayName);
		OnCategoryRegistered.Broadcast(InCategoryName, DisplayName);
		PRINTLOG_SH(TEXT("Category Registered: %s"), *InCategoryName.ToString());
	}
}

void UTranslateDialogManager::AddDialog(FName InCategoryName, const FDialogEntry& Entry)
{
	if (!DialogDataMap.Contains(InCategoryName))
	{
		PRINTLOG_SH(TEXT("Unknown Category: %s"), *InCategoryName.ToString());
		return;
	}

	DialogDataMap[InCategoryName].Add(Entry);
	OnDialogAdded.Broadcast(InCategoryName, Entry);
}

bool UTranslateDialogManager::HasCategory(FName InCategoryName) const
{
	return DialogDataMap.Contains(InCategoryName);
}

const TArray<FDialogEntry>* UTranslateDialogManager::GetDialogs(FName InCategoryName) const
{
	return DialogDataMap.Find(InCategoryName);
}

const TMap<FName, FText>& UTranslateDialogManager::GetCategoryDisplayNameMap() const
{
	return CategoryDisplayNameMap;
}
