
#include "Manager/DataManager.h"

#include "Settings/DataManagerSettings.h"
#include "Engine/DataTable.h"
#include "Murphy.h"

void UDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadDataTables();
}

void UDataManager::Deinitialize()
{
	Super::Deinitialize();

	ScenarioDataTable = nullptr;
	QuestDataTable = nullptr;
	PhoneAppDataTable = nullptr;
	ItemDataTable = nullptr;
	EmotionDataTable = nullptr;
	LocationDataTable = nullptr;
	CustomsItemDataTable = nullptr;
	TierDataTable = nullptr;
}

void UDataManager::LoadDataTables()
{
	const UDataManagerSettings* Settings = GetDefault<UDataManagerSettings>();
	if (!ensure(Settings))
	{
		PRINTLOGE_JW(TEXT("DataManagerSettings를 찾을 수 없습니다."));
		return;
	}

	// 시나리오 DataTable 동기 로드
	if (!Settings->ScenarioDataTable.IsNull())
	{
		ScenarioDataTable = Settings->ScenarioDataTable.LoadSynchronous();
		if (!ScenarioDataTable)
		{
			PRINTLOGE_JW(TEXT("ScenarioDataTable 로드 실패: %s"), *Settings->ScenarioDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("ScenarioDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}

	// 퀘스트 DataTable 동기 로드
	if (!Settings->QuestDataTable.IsNull())
	{
		QuestDataTable = Settings->QuestDataTable.LoadSynchronous();
		if (!QuestDataTable)
		{
			PRINTLOGE_JW(TEXT("QuestDataTable 로드 실패: %s"), *Settings->QuestDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("QuestDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}
	
	// 폰 앱 DataTable 동기 로드
	if (!Settings->PhoneAppDataTable.IsNull())
	{
		PhoneAppDataTable = Settings->PhoneAppDataTable.LoadSynchronous();
		if (!PhoneAppDataTable)		
		{
			PRINTLOG_SH(TEXT("PhoneAppDataTable 로드 실패: %s"), *Settings->PhoneAppDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("PhoneAppDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}

	// 아이템 DataTable 동기 로드
	if (!Settings->ItemDataTable.IsNull())
	{
		ItemDataTable = Settings->ItemDataTable.LoadSynchronous();
		if (!ItemDataTable)
		{
			PRINTLOGE_JW(TEXT("ItemDataTable 로드 실패: %s"), *Settings->ItemDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("ItemDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}

	// NPC 감정 DataTable 동기 로드
	if (!Settings->EmotionDataTable.IsNull())
	{
		EmotionDataTable = Settings->EmotionDataTable.LoadSynchronous();
		if (!EmotionDataTable)
		{
			PRINTLOGE_JW(TEXT("EmotionDataTable 로드 실패: %s"), *Settings->EmotionDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("EmotionDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}

	// 입국심사 장소 DataTable 동기 로드
	if (!Settings->LocationDataTable.IsNull())
	{
		LocationDataTable = Settings->LocationDataTable.LoadSynchronous();
		if (!LocationDataTable)
		{
			PRINTLOGE_JW(TEXT("LocationDataTable 로드 실패: %s"), *Settings->LocationDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("LocationDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}

	// 입국심사 물품 DataTable 동기 로드
	if (!Settings->CustomsItemDataTable.IsNull())
	{
		CustomsItemDataTable = Settings->CustomsItemDataTable.LoadSynchronous();
		if (!CustomsItemDataTable)
		{
			PRINTLOGE_JW(TEXT("CustomsItemDataTable 로드 실패: %s"), *Settings->CustomsItemDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("CustomsItemDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}

	// 티어 DataTable 동기 로드
	if (!Settings->TierDataTable.IsNull())
	{
		TierDataTable = Settings->TierDataTable.LoadSynchronous();
		if (!TierDataTable)
		{
			PRINTLOGE_JW(TEXT("TierDataTable 로드 실패: %s"), *Settings->TierDataTable.ToString());
		}
	}
	else
	{
		PRINTLOGW_JW(TEXT("TierDataTable이 설정되지 않았습니다. Project Settings -> Murphy Data Settings를 확인하세요."));
	}
}

FScenarioTableRow* UDataManager::GetScenarioData(const FName& RowName) const
{
	if (!ScenarioDataTable)
	{
		PRINTLOGE_JW(TEXT("ScenarioDataTable이 로드되지 않았습니다."));
		return nullptr;
	}

	FScenarioTableRow* Row = ScenarioDataTable->FindRow<FScenarioTableRow>(RowName, TEXT("GetScenarioData"));
	if (!Row)
	{
		PRINTLOGW_JW(TEXT("시나리오 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

TArray<FName> UDataManager::GetAllScenarioRowNames() const
{
	if (!ScenarioDataTable)
	{
		PRINTLOGE_JW(TEXT("ScenarioDataTable이 로드되지 않았습니다."));
		return {};
	}

	return ScenarioDataTable->GetRowNames();
}

FQuestTableRow* UDataManager::GetQuestData(const FName& RowName) const
{
	if (!QuestDataTable)
	{
		PRINTLOGE_JW(TEXT("QuestDataTable이 로드되지 않았습니다."));
		return nullptr;
	}

	FQuestTableRow* Row = QuestDataTable->FindRow<FQuestTableRow>(RowName, TEXT("GetQuestData"));
	if (!Row)
	{
		PRINTLOGW_JW(TEXT("퀘스트 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

TArray<FName> UDataManager::GetAllQuestRowNames() const
{
	if (!QuestDataTable)
	{
		PRINTLOGE_JW(TEXT("QuestDataTable이 로드되지 않았습니다."));
		return {};
	}

	return QuestDataTable->GetRowNames();
}

FPhoneAppRow* UDataManager::GetPhoneAppData(const FName& RowName) const
{
	if (!PhoneAppDataTable)
	{
		PRINTLOG_SH(TEXT("PhoneAppDataTable이 로드되지 않았습니다."));
		return nullptr;
	}

	FPhoneAppRow* Row = PhoneAppDataTable->FindRow<FPhoneAppRow>(RowName, TEXT("GetPhoneAppData"));
	if (!Row)
	{
		PRINTLOG_SH(TEXT("PhoneApp Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

FItemTableRow* UDataManager::GetItemData(const FName& RowName) const
{
	if (!ItemDataTable)
	{
		PRINTLOGE_JW(TEXT("ItemDataTable이 로드되지 않았습니다."));
		return nullptr;
	}

	FItemTableRow* Row = ItemDataTable->FindRow<FItemTableRow>(RowName, TEXT("GetItemData"));
	if (!Row)
	{
		PRINTLOGW_JW(TEXT("아이템 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

TArray<FName> UDataManager::GetAllPhoneAppRowNames() const
{
	if (!PhoneAppDataTable)
	{
		PRINTLOG_SH(TEXT("PhoneAppDataTable이 로드되지 않았습니다."));
		return {};
	}

	return PhoneAppDataTable->GetRowNames();
}

TArray<FPhoneAppRow*> UDataManager::GetAllPhoneAppRows() const
{
	if (!PhoneAppDataTable)
	{
		PRINTLOG_SH(TEXT("PhoneAppDataTable이 로드되지 않았습니다."));
		return {};
	}

	TArray<FPhoneAppRow*> Rows;
	for (const FName& RowName : PhoneAppDataTable->GetRowNames())
	{
		if (FPhoneAppRow* Row = PhoneAppDataTable->FindRow<FPhoneAppRow>(RowName, TEXT("GetAllPhoneAppRows")))
		{
			Rows.Add(Row);
		}
	}
	return Rows;
}

TArray<FName> UDataManager::GetAllItemRowNames() const
{
	if (!ItemDataTable)
	{
		PRINTLOGE_JW(TEXT("ItemDataTable이 로드되지 않았습니다."));
		return {};
	}

	return ItemDataTable->GetRowNames();
}

FAI_EmotionData* UDataManager::GetEmotionData(const FName& RowName) const
{
	if (!EmotionDataTable)
	{
		PRINTLOGE_JW(TEXT("EmotionDataTable이 로드되지 않았습니다."));
		return nullptr;
	}

	FAI_EmotionData* Row = EmotionDataTable->FindRow<FAI_EmotionData>(RowName, TEXT("GetEmotionData"));
	if (!Row)
	{
		PRINTLOGW_JW(TEXT("감정 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

TArray<FName> UDataManager::GetAllEmotionRowNames() const
{
	if (!EmotionDataTable)
	{
		PRINTLOGE_JW(TEXT("EmotionDataTable이 로드되지 않았습니다."));
		return {};
	}

	return EmotionDataTable->GetRowNames();
}

FLocationTextData* UDataManager::GetLocationData(const FName& RowName) const
{
	if (!LocationDataTable)
	{
		PRINTLOG_HJ(TEXT("LocationDataTable이 로드되지 않았습니다."));
		return nullptr;
	}
	
	FLocationTextData* Row = LocationDataTable->FindRow<FLocationTextData>(RowName, TEXT("GetLocationData"));
	if (!Row)
	{
		PRINTLOG_HJ(TEXT("장소 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row; 
}

TArray<FName> UDataManager::GetAllLocationRowNames() const
{
	if (!LocationDataTable)
	{
		PRINTLOG_HJ(TEXT("LocationDataTable이 로드되지 않았습니다."));
		return {};
	}
	
	return LocationDataTable->GetRowNames();
}

FCustomsItemTextData* UDataManager::GetCustomsItemData(const FName& RowName) const
{
	if (!CustomsItemDataTable)
	{
		PRINTLOG_HJ(TEXT("CustomsItemDataTable이 로드되지 않았습니다."));
		return nullptr;
	}
	
	FCustomsItemTextData* Row = CustomsItemDataTable->FindRow<FCustomsItemTextData>(RowName, TEXT("GetCustomsItemData"));
	if (!Row)
	{
		PRINTLOG_HJ(TEXT("입국심사 물품 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

TArray<FName> UDataManager::GetAllCustomsItemRowNames() const
{
	if (!CustomsItemDataTable)
	{
		PRINTLOG_HJ(TEXT("CustomsItemDataTable이 로드되지 않았습니다."));
		return {};
	}
	
	return CustomsItemDataTable->GetRowNames();
}

FTierUIDataRow* UDataManager::GetTierData(const FName& RowName) const
{
	if (!TierDataTable)
	{
		PRINTLOG_HJ(TEXT("TierDataTable이 로드되지 않았습니다."));
		return nullptr;
	}
	
	FTierUIDataRow* Row = TierDataTable->FindRow<FTierUIDataRow>(RowName, TEXT("GetTierData"));
	if (!Row)
	{
		PRINTLOG_HJ(TEXT("티어 Row를 찾을 수 없습니다: %s"), *RowName.ToString());
	}

	return Row;
}

TArray<FName> UDataManager::GetAllTierRowNames() const
{
	if (!TierDataTable)
	{
		PRINTLOG_HJ(TEXT("TierDataTable이 로드되지 않았습니다."));
		return {};
	}
	
	return TierDataTable->GetRowNames();
}
