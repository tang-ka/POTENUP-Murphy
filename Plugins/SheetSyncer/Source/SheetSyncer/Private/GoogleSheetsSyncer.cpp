// Fill out your copyright notice in the Description page of Project Settings.

#include "GoogleSheetsSyncer.h"

#include "DataTableEditorUtils.h"
#include "FileHelpers.h"
#include "HttpModule.h"
#include "SheetSyncerSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/DataTable.h"

bool UGoogleSheetsSyncer::ParseURL(const FString& URL, FString& OutSpreadsheetId, FString& OutGid)
{
    // SpreadsheetId 추출
    // https://docs.google.com/spreadsheets/d/{ID}/edit?gid={GID}
    FString Left, Right;
    if (!URL.Split(TEXT("/spreadsheets/d/"), &Left, &Right))
    {
        UE_LOG(LogTemp, Error, TEXT("[SheetSyncer] URL 형식이 올바르지 않습니다: %s"), *URL);
        return false;
    }

    Right.Split(TEXT("/"), &OutSpreadsheetId, &Left);

    // gid 추출
    if (!Right.Split(TEXT("gid="), &Left, &OutGid))
    {
        UE_LOG(LogTemp, Error, TEXT("[SheetSyncer] gid를 찾을 수 없습니다: %s"), *URL);
        return false;
    }

    // gid 뒤에 붙는 불필요한 문자 제거
    FString GidOnly;
    OutGid.Split(TEXT("#"), &GidOnly, &Left);
    if (!GidOnly.IsEmpty())
    {
        OutGid = GidOnly;
    }

    return true;
}

void UGoogleSheetsSyncer::SyncAll()
{
    const USheetSyncerSettings* Settings = USheetSyncerSettings::Get();
    if (Settings->Categories.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SheetSyncer] Categories가 비어있습니다."));
        return;
    }
    
    for (int32 i = 0; i < Settings->Categories.Num(); i++)
    {
        SyncCategory(i);
    }
}

void UGoogleSheetsSyncer::SyncCategory(int32 CategoryIndex)
{
    const USheetSyncerSettings* Settings = USheetSyncerSettings::Get();
    if (!Settings->Categories.IsValidIndex(CategoryIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("[SheetSyncer] 유효하지 않은 CategoryIndex: %d"), CategoryIndex);
        return;
    }

    const FSheetSyncerCategory& Category = Settings->Categories[CategoryIndex];
    UE_LOG(LogTemp, Log, TEXT("[SheetSyncer] [%s] 동기화 시작"), *Category.CategoryName);

    for (int32 i = 0; i < Category.Entries.Num(); i++)
    {
        RequestCSV(CategoryIndex, i);
    }
}

void UGoogleSheetsSyncer::RequestCSV(int32 CategoryIndex, int32 EntryIndex)
{
    const USheetSyncerSettings* Settings = USheetSyncerSettings::Get();
    const FSheetSyncerEntry& Entry = Settings->Categories[CategoryIndex].Entries[EntryIndex];

    FString SpreadsheetId, Gid;
    if (!ParseURL(Entry.SheetURL, SpreadsheetId, Gid))
    {
        return;
    }

    FString URL = FString::Printf(
        TEXT("https://docs.google.com/spreadsheets/d/%s/export?format=csv&gid=%s"),
        *SpreadsheetId,
        *Gid);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindLambda(
        [CategoryIndex, EntryIndex](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
        {
            if (!bSuccess || !Res.IsValid())
            {
                UE_LOG(LogTemp, Error,
                    TEXT("[SheetSyncer] 요청 실패 Category: %d, Entry: %d"), CategoryIndex, EntryIndex);
                return;
            }

            OnCSVReceived(CategoryIndex, EntryIndex, Res->GetContentAsString());
        });
    Request->ProcessRequest();
}

void UGoogleSheetsSyncer::OnCSVReceived(int32 CategoryIndex, int32 EntryIndex, const FString& CSVText)
{
    const USheetSyncerSettings* Settings = USheetSyncerSettings::Get();
    const FSheetSyncerCategory& Category = Settings->Categories[CategoryIndex];
    UDataTable* DataTable = Category.Entries[EntryIndex].TargetDataTable.LoadSynchronous();
    

    if (!IsValid(DataTable))
    {
        UE_LOG(LogTemp, Error,
            TEXT("[SheetSyncer] [%s] DataTable이 유효하지 않습니다. Entry: %d"),
            *Category.CategoryName, EntryIndex);
        return;
    }

    TArray<FString> Errors = DataTable->CreateTableFromCSVString(CSVText);
    if (Errors.Num() > 0)
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("[SheetSyncer] 파싱 에러: %s"), *Error);
        }
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("[SheetSyncer] [%s] 동기화 완료 Entry: %d"), *Category.CategoryName, EntryIndex);

    if (DataTable->MarkPackageDirty())
    {
        UE_LOG(LogTemp, Log,
            TEXT("[SheetSyncer] [%s] DataTable 업데이트 완료 Entry: %d"), *Category.CategoryName, EntryIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[SheetSyncer] [%s] DataTable 업데이트 실패 Entry: %d"), *Category.CategoryName, EntryIndex);
    }
    
    // 변경
    FDataTableEditorUtils::BroadcastPostChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
    
    // 에셋 저장
    UEditorLoadingAndSavingUtils::SavePackages({ DataTable->GetOutermost() }, false);

    UE_LOG(LogTemp, Log, TEXT("[SheetSyncer] [%s] DataTable 저장 완료 Entry: %d"), *Category.CategoryName, EntryIndex);
}