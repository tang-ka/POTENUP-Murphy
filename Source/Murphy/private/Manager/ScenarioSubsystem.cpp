
#include "Manager/ScenarioSubsystem.h"
#include "Manager/DataManager.h"
#include "Murphy.h"


void UScenarioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurScenario = EScenarioType::None;
}

void UScenarioSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UScenarioSubsystem::StartScenario(EScenarioType NewScenario)
{
	if (CurScenario == NewScenario) return;
	
	CurScenario = NewScenario;

	// Enum 이름을 CSV Row Name으로 사용 (EScenarioType 값 = Row Name 규칙)
	const FString EnumName = StaticEnum<EScenarioType>()->GetNameStringByValue((int64)CurScenario);
	PRINTLOGW_JW(TEXT("현재 시나리오 타입: %s"), *EnumName);

	// DataManager에서 시나리오 데이터 로드 → 퀘스트 목록 세팅
	ActiveQuestIDs.Empty();
	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		if (const FScenarioTableRow* ScenarioData = DataManager->GetScenarioData(FName(*EnumName)))
		{
			ActiveQuestIDs = ScenarioData->RequiredQuestIDs;
			PRINTLOGW_JW(TEXT("퀘스트 %d개 로드 완료"), ActiveQuestIDs.Num());
		}
		else
		{
			PRINTLOGW_JW(TEXT("시나리오 Row를 찾지 못함: %s (데이터 없으면 정상)"), *EnumName);
		}
	}
	else
	{
		PRINTLOGE_JW(TEXT("DataManager Subsystem을 가져올 수 없습니다."));
	}

	OnScenarioStateChanged.Broadcast(CurScenario);
}

void UScenarioSubsystem::EndScenario(bool bSuccess)
{
	if (CurScenario == EScenarioType::None) return;
	
	EScenarioType EndScene = CurScenario;
	CurScenario = EScenarioType::None;

	// 시나리오 종료 시 퀘스트 목록 초기화
	ActiveQuestIDs.Empty();

	OnScenarioEnded.Broadcast(EndScene, bSuccess);
	OnScenarioStateChanged.Broadcast(CurScenario);
}
