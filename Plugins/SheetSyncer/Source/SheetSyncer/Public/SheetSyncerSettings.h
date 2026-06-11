// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "SheetSyncerSettings.generated.h"

USTRUCT(BlueprintType)
struct FSheetSyncerEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SheetSyncer")
	FString SheetURL;

	UPROPERTY(EditAnywhere, Category = "SheetSyncer")
	TSoftObjectPtr<UDataTable> TargetDataTable;
};

USTRUCT(BlueprintType)
struct FSheetSyncerCategory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SheetSyncer")
	FString CategoryName;

	UPROPERTY(EditAnywhere, Category = "SheetSyncer")
	TArray<FSheetSyncerEntry> Entries;
};

UCLASS(Config = SheetSyncer, DefaultConfig, meta = (DisplayName = "Sheet Syncer"))
class SHEETSYNCER_API USheetSyncerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// UFUNCTION(CallInEditor, Category = "SheetSyncer")
	// void SyncAll();
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "SheetSyncer")
	TArray<FSheetSyncerCategory> Categories;

	static const USheetSyncerSettings* Get()
	{
		return GetDefault<USheetSyncerSettings>();
	}

	virtual FName GetCategoryName() const override
	{
		return TEXT("Plugins");
	}
};
