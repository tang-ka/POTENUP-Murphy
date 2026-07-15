#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GoogleSheetsSyncer.generated.h"

struct FSheetSyncerEntry;

UCLASS(ClassGroup=(SheetSyncer), meta=(BlueprintSpawnableComponent))
class SHEETSYNCER_API UGoogleSheetsSyncer : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Sheet Syncer")
	static void SyncAll();
	
	UFUNCTION(BlueprintCallable, Category = "Sheet Syncer")
	static void SyncCategory(int32 CategoryIndex);

private:
	static bool ParseURL(const FString& URL, FString& OutSpreadsheetId, FString& OutGid);
	static void RequestCSV(int32 CategoryIndex, int32 EntryIndex);
	static void OnCSVReceived(int32 CategoryIndex, int32 EntryIndex, const FString& CSVText);
};
