#include "Quest/QuestRuntimeHelper.h"

#include "Manager/DataManager.h"

void FQuestRuntimeHelper::BuildScenarioRuntimeQuests(
	const UDataManager* DataManager,
	const FScenarioTableRow* ScenarioData,
	TArray<FQuestRuntimeData>& OutActiveQuests,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	OutActiveQuests.Empty();

	if (!DataManager || !ScenarioData)
	{
		return;
	}

	for (FName QuestID : ScenarioData->RequiredQuestIDs)
	{
		if (QuestID.IsNone())
		{
			continue;
		}

		FQuestRuntimeData NewQuestData;
		NewQuestData.QuestID = QuestID;
		NewQuestData.QuestState = EScenarioState::InProgress;

		// 메인 퀘스트는 시나리오 목표로 항상 진행 중 상태를 유지하고, 서브/토스트 퀘스트는 시작 조건을 기다립니다.
		if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
		{
			NewQuestData.QuestState = QuestData->QuestType == EQuestType::MainQuest
				? EScenarioState::InProgress
				: EScenarioState::NotStarted;
		}

		OutActiveQuests.Add(NewQuestData);
	}

	// 시나리오 시작과 동시에 열려야 하는 퀘스트를 한 번 더 검사합니다.
	TryStartQuestsByEvent(DataManager, OutActiveQuests, NAME_None, EQuestStartCondition::ScenarioStart, OutEvents);

	// 데이터에 시작 조건이 빠져 있어도 RequiredQuestIDs 순서상 첫 서브퀘스트는 시작합니다.
	StartFirstSequentialSubQuest(DataManager, OutActiveQuests, OutEvents);
}

void FQuestRuntimeHelper::NotifyQuestEvent(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName TargetID,
	EQuestStartCondition EventCondition,
	TArray<FQuestRuntimeEvent>& OutEvents,
	bool& bOutScenarioCompleted)
{
	bOutScenarioCompleted = false;

	if (!DataManager || EventCondition == EQuestStartCondition::None)
	{
		return;
	}

	EQuestClearCondition ClearCondition = EQuestClearCondition::None;
	if (TryConvertStartConditionToClearCondition(EventCondition, ClearCondition))
	{
		// 일부 이벤트는 "조건 만족"과 "다음 시작 조건"을 동시에 의미하므로 완료 처리를 먼저 수행합니다.
		TryCompleteQuestsByEvent(DataManager, ActiveQuests, TargetID, ClearCondition, OutEvents, bOutScenarioCompleted);
	}

	TryStartQuestsByEvent(DataManager, ActiveQuests, TargetID, EventCondition, OutEvents);
}

void FQuestRuntimeHelper::NotifyQuestStartEvent(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName TargetID,
	EQuestStartCondition EventCondition,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager || EventCondition == EQuestStartCondition::None)
	{
		return;
	}

	TryStartQuestsByEvent(DataManager, ActiveQuests, TargetID, EventCondition, OutEvents);
}

void FQuestRuntimeHelper::NotifyQuestConditionMet(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName TargetID,
	EQuestClearCondition ClearCondition,
	TArray<FQuestRuntimeEvent>& OutEvents,
	bool& bOutScenarioCompleted)
{
	NotifyQuestEvent(
		DataManager,
		ActiveQuests,
		TargetID,
		ConvertClearConditionToStartCondition(ClearCondition),
		OutEvents,
		bOutScenarioCompleted);
}

bool FQuestRuntimeHelper::AreRequiredChildQuestsCompleted(
	const UDataManager* DataManager,
	const TArray<FQuestRuntimeData>& ActiveQuests)
{
	if (!DataManager || ActiveQuests.IsEmpty())
	{
		return false;
	}

	// 필수 하위 퀘스트가 하나도 없으면 메인 퀘스트를 자동 완료하지 않습니다.
	bool bHasRequiredChildQuest = false;

	for (const FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		if (!QuestData)
		{
			continue;
		}

		if (QuestData->QuestType == EQuestType::MainQuest || !QuestData->bRequiredForScenarioEnd)
		{
			continue;
		}

		bHasRequiredChildQuest = true;
		if (RuntimeData.QuestState != EScenarioState::Completed)
		{
			return false;
		}
	}

	return bHasRequiredChildQuest;
}

bool FQuestRuntimeHelper::AreMainQuestsCompleted(
	const UDataManager* DataManager,
	const TArray<FQuestRuntimeData>& ActiveQuests)
{
	if (!DataManager || ActiveQuests.IsEmpty())
	{
		return false;
	}

	bool bHasMainQuest = false;

	for (const FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		if (!QuestData || QuestData->QuestType != EQuestType::MainQuest)
		{
			continue;
		}

		bHasMainQuest = true;
		if (RuntimeData.QuestState != EScenarioState::Completed)
		{
			return false;
		}
	}

	return bHasMainQuest;
}

EQuestStartCondition FQuestRuntimeHelper::ConvertClearConditionToStartCondition(EQuestClearCondition ClearCondition)
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

bool FQuestRuntimeHelper::TryConvertStartConditionToClearCondition(EQuestStartCondition StartCondition, EQuestClearCondition& OutClearCondition)
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

FQuestRuntimeData* FQuestRuntimeHelper::FindRuntimeQuest(TArray<FQuestRuntimeData>& ActiveQuests, FName QuestID)
{
	return ActiveQuests.FindByPredicate([QuestID](const FQuestRuntimeData& RuntimeData)
	{
		return RuntimeData.QuestID == QuestID;
	});
}

const FQuestRuntimeData* FQuestRuntimeHelper::FindRuntimeQuest(const TArray<FQuestRuntimeData>& ActiveQuests, FName QuestID)
{
	return ActiveQuests.FindByPredicate([QuestID](const FQuestRuntimeData& RuntimeData)
	{
		return RuntimeData.QuestID == QuestID;
	});
}

void FQuestRuntimeHelper::TryStartQuestsByEvent(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName TargetID,
	EQuestStartCondition EventCondition,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager || EventCondition == EQuestStartCondition::None)
	{
		return;
	}

	// 순회 중 배열 상태를 바꾸지 않도록 먼저 시작 대상만 모읍니다.
	TArray<FName> QuestIDsToStart;
	for (const FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		if (RuntimeData.QuestState != EScenarioState::NotStarted)
		{
			continue;
		}

		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		if (!QuestData)
		{
			continue;
		}

		const bool bTargetMatched = QuestData->StartTargetID.IsNone()
			? TargetID.IsNone()
			: QuestData->StartTargetID == TargetID;

		if (QuestData->StartCondition == EventCondition && bTargetMatched)
		{
			QuestIDsToStart.Add(RuntimeData.QuestID);
		}
	}

	for (FName QuestID : QuestIDsToStart)
	{
		StartQuest(DataManager, ActiveQuests, QuestID, OutEvents);
	}
}

void FQuestRuntimeHelper::TryCompleteQuestsByEvent(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName TargetID,
	EQuestClearCondition ClearCondition,
	TArray<FQuestRuntimeEvent>& OutEvents,
	bool& bOutScenarioCompleted)
{
	if (!DataManager || ClearCondition == EQuestClearCondition::None)
	{
		return;
	}

	// 완료 처리 중 다음 퀘스트가 시작될 수 있으므로 먼저 완료 대상만 수집합니다.
	TArray<FName> QuestIDsToComplete;
	for (const FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		if (RuntimeData.QuestState != EScenarioState::InProgress)
		{
			continue;
		}

		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		if (!QuestData || QuestData->QuestType == EQuestType::MainQuest)
		{
			continue;
		}

		if (QuestData->ClearCondition == ClearCondition && QuestData->QuestTargetID == TargetID)
		{
			QuestIDsToComplete.Add(RuntimeData.QuestID);
		}
	}

	for (FName QuestID : QuestIDsToComplete)
	{
		bool bQuestCompletedScenario = false;
		const bool bCompletedScenario = CompleteQuest(DataManager, ActiveQuests, QuestID, OutEvents, bQuestCompletedScenario);
		bOutScenarioCompleted = bOutScenarioCompleted || bQuestCompletedScenario || bCompletedScenario;
	}
}

bool FQuestRuntimeHelper::StartQuest(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName QuestID,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager || QuestID.IsNone())
	{
		return false;
	}

	FQuestRuntimeData* RuntimeData = FindRuntimeQuest(ActiveQuests, QuestID);
	if (!RuntimeData || RuntimeData->QuestState != EScenarioState::NotStarted)
	{
		return false;
	}

	RuntimeData->QuestState = EScenarioState::InProgress;

	FQuestRuntimeEvent Event;
	Event.QuestID = QuestID;
	Event.EventType = EQuestRuntimeEventType::Started;
	OutEvents.Add(Event);

	return true;
}

bool FQuestRuntimeHelper::CompleteQuest(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName QuestID,
	TArray<FQuestRuntimeEvent>& OutEvents,
	bool& bOutScenarioCompleted)
{
	bOutScenarioCompleted = false;

	if (!DataManager || QuestID.IsNone() || IsMainQuest(DataManager, QuestID))
	{
		return false;
	}

	FQuestRuntimeData* RuntimeData = FindRuntimeQuest(ActiveQuests, QuestID);
	if (!RuntimeData || RuntimeData->QuestState != EScenarioState::InProgress)
	{
		return false;
	}

	RuntimeData->QuestState = EScenarioState::Completed;

	FQuestRuntimeEvent CompletedEvent;
	CompletedEvent.QuestID = QuestID;
	CompletedEvent.EventType = EQuestRuntimeEventType::Completed;
	OutEvents.Add(CompletedEvent);

	// 완료된 퀘스트를 선행 조건으로 삼는 후속 퀘스트를 즉시 시작합니다.
	TryStartQuestsByEvent(DataManager, ActiveQuests, QuestID, EQuestStartCondition::QuestCompleted, OutEvents);
	StartNextSequentialSubQuest(DataManager, ActiveQuests, QuestID, OutEvents);

	if (AreRequiredChildQuestsCompleted(DataManager, ActiveQuests))
	{
		// 메인 퀘스트는 외부 이벤트로 직접 완료하지 않고, 필수 하위 퀘스트 완료 결과로만 자동 완료합니다.
		CompleteMainQuests(DataManager, ActiveQuests, OutEvents);
		bOutScenarioCompleted = true;
	}

	return bOutScenarioCompleted;
}

bool FQuestRuntimeHelper::StartFirstSequentialSubQuest(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager)
	{
		return false;
	}

	for (const FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		if (!IsSequentialSubQuest(DataManager, RuntimeData.QuestID))
		{
			continue;
		}

		if (RuntimeData.QuestState == EScenarioState::NotStarted)
		{
			return StartQuest(DataManager, ActiveQuests, RuntimeData.QuestID, OutEvents);
		}

		return false;
	}

	return false;
}

bool FQuestRuntimeHelper::StartNextSequentialSubQuest(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	FName CompletedQuestID,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager || CompletedQuestID.IsNone())
	{
		return false;
	}

	bool bFoundCompletedQuest = false;

	for (const FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		if (!bFoundCompletedQuest)
		{
			bFoundCompletedQuest = RuntimeData.QuestID == CompletedQuestID;
			continue;
		}

		if (!IsSequentialSubQuest(DataManager, RuntimeData.QuestID))
		{
			continue;
		}

		if (RuntimeData.QuestState == EScenarioState::NotStarted)
		{
			return StartQuest(DataManager, ActiveQuests, RuntimeData.QuestID, OutEvents);
		}

		return false;
	}

	return false;
}

bool FQuestRuntimeHelper::IsSequentialSubQuest(const UDataManager* DataManager, FName QuestID)
{
	if (!DataManager || QuestID.IsNone())
	{
		return false;
	}

	if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
	{
		return QuestData->QuestType == EQuestType::SubQuest;
	}

	return false;
}

bool FQuestRuntimeHelper::IsMainQuest(const UDataManager* DataManager, FName QuestID)
{
	if (!DataManager || QuestID.IsNone())
	{
		return false;
	}

	if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
	{
		return QuestData->QuestType == EQuestType::MainQuest;
	}

	return false;
}

bool FQuestRuntimeHelper::CompleteMainQuests(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager)
	{
		return false;
	}

	bool bCompletedAnyMainQuest = false;
	for (FQuestRuntimeData& RuntimeData : ActiveQuests)
	{
		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		if (!QuestData || QuestData->QuestType != EQuestType::MainQuest)
		{
			continue;
		}

		if (RuntimeData.QuestState == EScenarioState::Completed)
		{
			continue;
		}

		RuntimeData.QuestState = EScenarioState::Completed;
		bCompletedAnyMainQuest = true;

		FQuestRuntimeEvent CompletedEvent;
		CompletedEvent.QuestID = RuntimeData.QuestID;
		CompletedEvent.EventType = EQuestRuntimeEventType::Completed;
		OutEvents.Add(CompletedEvent);
	}

	return bCompletedAnyMainQuest;
}
