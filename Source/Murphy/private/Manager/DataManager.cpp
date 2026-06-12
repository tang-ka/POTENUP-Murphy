
#include "Manager/DataManager.h"

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
}

void UDataManager::LoadDataTables()
{
	// 다른 팀원이 더 나은 방식으로 데이터 로드 기능을 구현함
	PRINTLOGW_JW(TEXT("DataManagerSettings를 대체하는 새로운 방식으로 DataTable이 로드됩니다."));
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
