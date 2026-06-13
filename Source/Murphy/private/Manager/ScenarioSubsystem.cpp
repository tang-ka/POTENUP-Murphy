
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
	const FString EnumName = StaticEnum<EScenarioType>()->GetNameStringByValue((int64)CurScenario);
	PRINTLOGW_JW(TEXT("현재 시나리오 타입: %s"), *EnumName);

	// DataManager에서 시나리오 데이터 로드 → 퀘스트 목록 세팅 Map 초기화
	ActiveQuests.Empty();
	
	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		if (const FScenarioTableRow* ScenarioData = DataManager->GetScenarioData(FName(*EnumName)))
		{
			// 기존 배열 덮어쓰기 대신, 반복문을 돌며 RuntimeData(포스트잇)를 생성해 Map에 등록합니다.
			for (FName TargetQuestID : ScenarioData->RequiredQuestIDs)
			{
				FQuestRuntimeData NewQuestData;
				NewQuestData.QuestID = TargetQuestID;
				NewQuestData.QuestState = EScenarioState::InProgress; // 퀘스트 시작 시 '진행 중' 부여
              
				ActiveQuests.Add(TargetQuestID, NewQuestData);
			}
			PRINTLOGW_JW(TEXT("퀘스트 %d개 런타임 세팅 완료"), ActiveQuests.Num());
		}
		else
		{
			PRINTLOGW_JW(TEXT("시나리오 Row를 찾지 못함: %s (데이터 없으면 정상)"), *EnumName);
		}
	}
	
	OnScenarioStateChanged.Broadcast(CurScenario);
}

void UScenarioSubsystem::EndScenario(bool bSuccess)
{
	if (CurScenario == EScenarioType::None) return;
	
	EScenarioType EndScene = CurScenario;
	CurScenario = EScenarioType::None;
	
	ActiveQuests.Empty();

	OnScenarioEnded.Broadcast(EndScene, bSuccess);
	OnScenarioStateChanged.Broadcast(CurScenario);
}

void UScenarioSubsystem::CompleteQuest(FName QuestID)
{
	// Map에 해당 퀘스트가 존재하는지 확인하고, 아직 진행 중인 경우에만 완료 처리
	if (ActiveQuests.Contains(QuestID) && ActiveQuests[QuestID].QuestState != EScenarioState::Completed)
	{
		// 상태를 완료로 변경!
		ActiveQuests[QuestID].QuestState = EScenarioState::Completed;
		PRINTLOGW_JW(TEXT("퀘스트 클리어 통과!: %s"), *QuestID.ToString());

		// 퀘스트 완료 델리게이트 브로드캐스트 (UI 등에서 수신)
		OnQuestCompleted.Broadcast(QuestID);

		// 모든 퀘스트가 완료되었는지 검사
		CheckAllQuestsCompleted();
	}
}


// NotifyQuestConditionMet("NPC_ImmigrationOfficer", EQuestClearCondition::TalkToNPC);
void UScenarioSubsystem::NotifyQuestConditionMet(FName TargetID, EQuestClearCondition Condition)
{
	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		// ActiveQuests를 순회하며 조건이 맞는 퀘스트 찾기
		for (const auto& Pair : ActiveQuests)
		{
			FName QuestID = Pair.Key;
			const FQuestRuntimeData& RuntimeData = Pair.Value;

			// 이미 완료된 퀘스트는 스킵
			if (RuntimeData.QuestState == EScenarioState::Completed)
			{
				continue;
			}

			// DataManager에서 퀘스트 원본 데이터 가져오기
			if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
			{
				// 조건과 타겟 ID가 모두 일치하면 퀘스트 달성!
				if (QuestData->ClearCondition == Condition && QuestData->QuestTargetID == TargetID)
				{
					CompleteQuest(QuestID);
				}
			}
		}
	}
}

void UScenarioSubsystem::CheckAllQuestsCompleted()
{
	// 진행해야 할 퀘스트가 하나도 없으면 무시
	if (ActiveQuests.IsEmpty())
	{
		return;
	}

	bool bAllCompleted = true;
	for (const auto& Pair : ActiveQuests)
	{
		if (Pair.Value.QuestState != EScenarioState::Completed)
		{
			bAllCompleted = false;
			break;
		}
	}

	if (bAllCompleted)
	{
		PRINTLOGW_JW(TEXT("모든 퀘스트 완료! 현재 시나리오 종료."));
		EndScenario(true);
	}
}