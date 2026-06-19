
#include "Manager/ScenarioSubsystem.h"
#include "Manager/DataManager.h"
#include "Murphy.h"


void UScenarioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurScenario = EScenarioType::None;
	StartScenario(EScenarioType::Prologue_Immigration);
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

				// 메인 퀘스트는 바로 시작, 서브 퀘스트는 대기(NotStarted) 상태로 둡니다.
				if (const FQuestTableRow* QuestData = DataManager->GetQuestData(TargetQuestID))
				{
					if (QuestData->QuestType == EQuestType::MainQuest)
					{
						NewQuestData.QuestState = EScenarioState::InProgress;
					}
					else
					{
						NewQuestData.QuestState = EScenarioState::NotStarted;
					}
				}
				else
				{
					NewQuestData.QuestState = EScenarioState::InProgress;
				}
              
				ActiveQuests.Add(TargetQuestID, NewQuestData);
			}
			PRINTLOGW_JW(TEXT("퀘스트 %d개 런타임 세팅 완료"), ActiveQuests.Num());
		}
		else
		{
			PRINTLOGW_JW(TEXT("시나리오 Row를 찾지 못함: %s (데이터 없으면 정상)"), *EnumName);
		}
	}

	TryStartQuestsByEvent(NAME_None, EQuestStartCondition::ScenarioStart);
	
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

void UScenarioSubsystem::StartQuest(FName QuestID)
{
	// 1. 등록되지 않은 퀘스트인지 확인
	FQuestRuntimeData* RuntimeData = ActiveQuests.Find(QuestID);
	if (!RuntimeData)
	{
		PRINTLOGW_JW(TEXT("StartQuest 실패: ActiveQuests에 퀘스트가 없습니다 (시나리오 미시작 또는 목록에 없음). ID: %s"), *QuestID.ToString());
		return;
	}

	// 2. 이미 시작되었거나 완료된 상태인지 확인
	if (RuntimeData->QuestState != EScenarioState::NotStarted)
	{
		PRINTLOGW_JW(TEXT("StartQuest 실패: 퀘스트가 대기(NotStarted) 상태가 아닙니다. ID: %s (현재 상태: %d)"), *QuestID.ToString(), (int32)RuntimeData->QuestState);
		return;
	}

	RuntimeData->QuestState = EScenarioState::InProgress;
	PRINTLOGW_JW(TEXT("퀘스트 시작 성공!: %s"), *QuestID.ToString());

	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
		{
			// 서브/토스트 퀘스트인 경우 토스트 알림 발생
			if (QuestData->bShowToastOnStart && (QuestData->QuestType == EQuestType::SubQuest || QuestData->QuestType == EQuestType::ToastQuest))
			{
				OnQuestStarted.Broadcast(QuestID, QuestData->QuestTitle, QuestData->QuestDescription);
				PRINTLOGW_JW(TEXT("퀘스트 토스트 발생!: %s"), *QuestID.ToString());
			}
			else
			{
				PRINTLOGW_JW(TEXT("StartQuest 알림: %s는 토스트 표시 대상이 아닙니다."), *QuestID.ToString());
			}
		}
	}
}

void UScenarioSubsystem::CompleteQuest(FName QuestID)
{
	FQuestRuntimeData* RuntimeData = ActiveQuests.Find(QuestID);
	if (!RuntimeData)
	{
		PRINTLOGW_JW(TEXT("CompleteQuest 실패: ActiveQuests에 퀘스트가 없습니다. ID: %s"), *QuestID.ToString());
		return;
	}

	if (RuntimeData->QuestState != EScenarioState::InProgress)
	{
		PRINTLOGW_JW(TEXT("CompleteQuest 실패: 퀘스트가 진행 중(InProgress) 상태가 아닙니다. ID: %s (현재 상태: %d)"), *QuestID.ToString(), (int32)RuntimeData->QuestState);
		return;
	}

	// 상태를 완료로 변경!
	RuntimeData->QuestState = EScenarioState::Completed;
	PRINTLOGW_JW(TEXT("퀘스트 클리어 통과!: %s"), *QuestID.ToString());

	// 퀘스트 완료 델리게이트 브로드캐스트 (UI 등에서 수신)
	OnQuestCompleted.Broadcast(QuestID);

	// 완료된 퀘스트를 선행 조건으로 삼는 다음 퀘스트를 시작합니다.
	TryStartQuestsByEvent(QuestID, EQuestStartCondition::QuestCompleted);

	// 모든 퀘스트가 완료되었는지 검사
	CheckAllQuestsCompleted();
}


// NotifyQuestConditionMet("NPC_ImmigrationOfficer", EQuestClearCondition::TalkToNPC);
void UScenarioSubsystem::NotifyQuestConditionMet(FName TargetID, EQuestClearCondition Condition)
{
	NotifyQuestEvent(TargetID, ConvertClearConditionToStartCondition(Condition));
}

void UScenarioSubsystem::NotifyQuestEvent(FName TargetID, EQuestStartCondition EventCondition)
{
	if (EventCondition == EQuestStartCondition::None)
	{
		return;
	}

	EQuestClearCondition ClearCondition = EQuestClearCondition::None;
	if (TryConvertStartConditionToClearCondition(EventCondition, ClearCondition))
	{
		TryCompleteQuestsByEvent(TargetID, ClearCondition);
	}

	TryStartQuestsByEvent(TargetID, EventCondition);
}

void UScenarioSubsystem::NotifyQuestStartEvent(FName TargetID, EQuestStartCondition EventCondition)
{
	TryStartQuestsByEvent(TargetID, EventCondition);
}

void UScenarioSubsystem::TryStartQuestsByEvent(FName TargetID, EQuestStartCondition EventCondition)
{
	if (EventCondition == EQuestStartCondition::None)
	{
		return;
	}

	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		for (const auto& Pair : ActiveQuests)
		{
			FName QuestID = Pair.Key;
			const FQuestRuntimeData& RuntimeData = Pair.Value;

			if (RuntimeData.QuestState != EScenarioState::NotStarted)
			{
				continue;
			}

			if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
			{
				const bool bTargetMatched = QuestData->StartTargetID.IsNone()
					? TargetID.IsNone()
					: QuestData->StartTargetID == TargetID;

				if (QuestData->StartCondition == EventCondition && bTargetMatched)
				{
					StartQuest(QuestID);
				}
			}
		}
	}
}

void UScenarioSubsystem::TryCompleteQuestsByEvent(FName TargetID, EQuestClearCondition ClearCondition)
{
	if (ClearCondition == EQuestClearCondition::None)
	{
		return;
	}

	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		// ActiveQuests를 순회하며 조건이 맞는 진행 중 퀘스트 찾기
		for (const auto& Pair : ActiveQuests)
		{
			FName QuestID = Pair.Key;
			const FQuestRuntimeData& RuntimeData = Pair.Value;

			if (RuntimeData.QuestState != EScenarioState::InProgress)
			{
				continue;
			}

			if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
			{
				if (QuestData->ClearCondition == ClearCondition && QuestData->QuestTargetID == TargetID)
				{
					CompleteQuest(QuestID);
				}
			}
		}
	}
}

EQuestStartCondition UScenarioSubsystem::ConvertClearConditionToStartCondition(EQuestClearCondition ClearCondition)
{
	switch (ClearCondition)
	{
	case EQuestClearCondition::CheckItem:
		return EQuestStartCondition::CheckItem;
	case EQuestClearCondition::ReachLocation:
		return EQuestStartCondition::ReachLocation;
	case EQuestClearCondition::TalkToNPC:
		return EQuestStartCondition::TalkToNPC;
	case EQuestClearCondition::GetItem:
		return EQuestStartCondition::GetItem;
	case EQuestClearCondition::UseItem:
		return EQuestStartCondition::UseItem;
	case EQuestClearCondition::None:
	default:
		return EQuestStartCondition::None;
	}
}

bool UScenarioSubsystem::TryConvertStartConditionToClearCondition(EQuestStartCondition StartCondition, EQuestClearCondition& OutClearCondition)
{
	switch (StartCondition)
	{
	case EQuestStartCondition::CheckItem:
		OutClearCondition = EQuestClearCondition::CheckItem;
		return true;
	case EQuestStartCondition::ReachLocation:
		OutClearCondition = EQuestClearCondition::ReachLocation;
		return true;
	case EQuestStartCondition::TalkToNPC:
		OutClearCondition = EQuestClearCondition::TalkToNPC;
		return true;
	case EQuestStartCondition::GetItem:
		OutClearCondition = EQuestClearCondition::GetItem;
		return true;
	case EQuestStartCondition::UseItem:
		OutClearCondition = EQuestClearCondition::UseItem;
		return true;
	case EQuestStartCondition::None:
	case EQuestStartCondition::ScenarioStart:
	case EQuestStartCondition::QuestCompleted:
	default:
		OutClearCondition = EQuestClearCondition::None;
		return false;
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
	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		for (const auto& Pair : ActiveQuests)
		{
			const FQuestRuntimeData& RuntimeData = Pair.Value;
			const FQuestTableRow* QuestData = DataManager->GetQuestData(Pair.Key);

			if (QuestData && !QuestData->bRequiredForScenarioEnd)
			{
				continue;
			}

			if (RuntimeData.QuestState != EScenarioState::Completed)
			{
				bAllCompleted = false;
				break;
			}
		}
	}
	else
	{
		for (const auto& Pair : ActiveQuests)
		{
			if (Pair.Value.QuestState != EScenarioState::Completed)
			{
				bAllCompleted = false;
				break;
			}
		}
	}

	if (bAllCompleted)
	{
		PRINTLOGW_JW(TEXT("모든 퀘스트 완료! 현재 시나리오 종료."));
		EndScenario(true);
	}
}
