#include "Quest/QuestRuntimeHelper.h"

#include "Murphy.h"
#include "Manager/DataManager.h"

// ==============================================================================
// === Public API ===
// ==============================================================================

int32 FQuestRuntimeHelper::BuildScenarioRuntimeQuests(
	const UDataManager* DataManager,
	const FScenarioTableRow* ScenarioData,
	TArray<FQuestRuntimeData>& OutActiveQuests,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	OutActiveQuests.Empty();

	if (!DataManager || !ScenarioData)
	{
		return INDEX_NONE;
	}

	for (FName QuestID : ScenarioData->RequiredQuestIDs)
	{
		if (QuestID.IsNone())
		{
			continue;
		}

		FQuestRuntimeData NewQuestData;
		NewQuestData.QuestID = QuestID;

		// 메인 퀘스트는 항상 InProgress, 서브/토스트 퀘스트는 NotStarted로 초기화합니다.
		if (const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID))
		{
			NewQuestData.QuestState = QuestData->QuestType == EQuestType::MainQuest
				? EScenarioState::InProgress
				: EScenarioState::NotStarted;
		}

		OutActiveQuests.Add(NewQuestData);
	}

	// ScenarioStart 조건을 가진 퀘스트를 배열 순서대로 찾아 즉시 시작합니다.
	// ScenarioStart 퀘스트는 항상 첫 번째 서브퀘스트이므로 인덱스 탐색 전에 처리합니다.
	for (int32 Index = 0; Index < OutActiveQuests.Num(); ++Index)
	{
		FQuestRuntimeData& RuntimeData = OutActiveQuests[Index];
		if (RuntimeData.QuestState != EScenarioState::NotStarted)
		{
			continue;
		}

		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		// if (!QuestData || QuestData->QuestType == EQuestType::MainQuest)
		if (!QuestData || QuestData->QuestType != EQuestType::SubQuest)
		{
			continue;
		}

		if (QuestData->StartCondition == EQuestCondition::ScenarioStart)
		{
			StartQuestAtIndex(OutActiveQuests, Index, OutEvents);
			// ScenarioStart 퀘스트가 첫 번째 서브퀘스트입니다.
			return Index;
		}
	}

	// ScenarioStart 퀘스트가 없으면 첫 번째 SubQuest를 찾아 자동 시작합니다.
	const int32 FirstIndex = FindNextSubQuestIndex(DataManager, OutActiveQuests, -1);
	if (FirstIndex != INDEX_NONE)
	{
		StartQuestAtIndex(OutActiveQuests, FirstIndex, OutEvents);
	}
	return FirstIndex;
}

void FQuestRuntimeHelper::ProcessQuestStartEvent(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	int32& InOutCurrentSubQuestIndex,
	FName TargetID,
	EQuestCondition Condition,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!DataManager || Condition == EQuestCondition::None)
	{
		return;
	}

	if (!ActiveQuests.IsValidIndex(InOutCurrentSubQuestIndex))
	{
		return;
	}

	FQuestRuntimeData& CurrentQuest = ActiveQuests[InOutCurrentSubQuestIndex];

	// 이미 진행 중이면 중복 시작을 무시합니다.
	if (CurrentQuest.QuestState != EScenarioState::NotStarted)
	{
		return;
	}

	const FQuestTableRow* QuestData = DataManager->GetQuestData(CurrentQuest.QuestID);
	if (!QuestData)
	{
		return;
	}

	// StartCondition과 StartTargetID(없으면 any) 매칭 확인
	const bool bTargetMatched = QuestData->StartTargetID.IsNone()
		? true
		: QuestData->StartTargetID == TargetID;

	if (QuestData->StartCondition == Condition && bTargetMatched)
	{
		StartQuestAtIndex(ActiveQuests, InOutCurrentSubQuestIndex, OutEvents);
		PRINTLOG_JW(
			TEXT("[QuestDebug] 시작 조건 처리됨. QuestID: %s, TargetID: %s, Condition: %d"),
			*CurrentQuest.QuestID.ToString(),
			*TargetID.ToString(),
			static_cast<int32>(Condition));
	}
}

void FQuestRuntimeHelper::ProcessQuestConditionMet(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	int32& InOutCurrentSubQuestIndex,
	FName TargetID,
	EQuestCondition Condition,
	TArray<FQuestRuntimeEvent>& OutEvents,
	bool& bOutScenarioCompleted)
{
	bOutScenarioCompleted = false;

	if (!DataManager || Condition == EQuestCondition::None)
	{
		return;
	}

	if (!ActiveQuests.IsValidIndex(InOutCurrentSubQuestIndex))
	{
		PRINTLOGW_JW(
			TEXT("[QuestDebug] 완료 이벤트가 왔지만 유효한 서브퀘스트 인덱스가 없습니다. TargetID: %s, Condition: %d"),
			*TargetID.ToString(),
			static_cast<int32>(Condition));
		return;
	}

	FQuestRuntimeData& CurrentQuest = ActiveQuests[InOutCurrentSubQuestIndex];

	if (CurrentQuest.QuestState != EScenarioState::InProgress)
	{
		PRINTLOGW_JW(
			TEXT("[QuestDebug] 현재 서브퀘스트가 InProgress 상태가 아닙니다. QuestID: %s, State: %d"),
			*CurrentQuest.QuestID.ToString(),
			static_cast<int32>(CurrentQuest.QuestState));
		return;
	}

	const FQuestTableRow* QuestData = DataManager->GetQuestData(CurrentQuest.QuestID);
	if (!QuestData)
	{
		return;
	}

	// ClearCondition과 QuestTargetID 매칭 확인
	if (QuestData->ClearCondition != Condition || QuestData->QuestTargetID != TargetID)
	{
		PRINTLOGW_JW(
			TEXT("[QuestDebug] 완료 조건 불일치. QuestID: %s, Expected Condition: %d / TargetID: %s, Got Condition: %d / TargetID: %s"),
			*CurrentQuest.QuestID.ToString(),
			static_cast<int32>(QuestData->ClearCondition),
			*QuestData->QuestTargetID.ToString(),
			static_cast<int32>(Condition),
			*TargetID.ToString());
		return;
	}

	// 현재 인덱스 퀘스트 완료
	CompleteQuestAtIndex(DataManager, ActiveQuests, InOutCurrentSubQuestIndex, OutEvents, bOutScenarioCompleted);

	PRINTLOG_JW(
		TEXT("[QuestDebug] 서브퀘스트 완료 처리됨. QuestID: %s, TargetID: %s, Condition: %d"),
		*CurrentQuest.QuestID.ToString(),
		*TargetID.ToString(),
		static_cast<int32>(Condition));

	if (bOutScenarioCompleted)
	{
		return;
	}

	// 다음 서브퀘스트로 인덱스 전진
	const int32 NextIndex = FindNextSubQuestIndex(DataManager, ActiveQuests, InOutCurrentSubQuestIndex);
	InOutCurrentSubQuestIndex = NextIndex;

	if (NextIndex != INDEX_NONE)
	{
		StartQuestAtIndex(ActiveQuests, NextIndex, OutEvents);
		PRINTLOG_JW(
			TEXT("[QuestDebug] 다음 서브퀘스트 시작. QuestID: %s, Index: %d"),
			*ActiveQuests[NextIndex].QuestID.ToString(),
			NextIndex);
	}
	else
	{
		PRINTLOG_JW(TEXT("[QuestDebug] 모든 서브퀘스트 완료. 시나리오 종료 조건 충족."));
	}
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

// ==============================================================================
// === Private helpers ===
// ==============================================================================

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

bool FQuestRuntimeHelper::StartQuestAtIndex(
	TArray<FQuestRuntimeData>& ActiveQuests,
	int32 Index,
	TArray<FQuestRuntimeEvent>& OutEvents)
{
	if (!ActiveQuests.IsValidIndex(Index))
	{
		return false;
	}

	FQuestRuntimeData& RuntimeData = ActiveQuests[Index];
	if (RuntimeData.QuestState != EScenarioState::NotStarted)
	{
		return false;
	}

	RuntimeData.QuestState = EScenarioState::InProgress;

	FQuestRuntimeEvent Event;
	Event.QuestID = RuntimeData.QuestID;
	Event.EventType = EQuestRuntimeEventType::Started;
	OutEvents.Add(Event);

	return true;
}

bool FQuestRuntimeHelper::CompleteQuestAtIndex(
	const UDataManager* DataManager,
	TArray<FQuestRuntimeData>& ActiveQuests,
	int32 Index,
	TArray<FQuestRuntimeEvent>& OutEvents,
	bool& bOutScenarioCompleted)
{
	bOutScenarioCompleted = false;

	if (!ActiveQuests.IsValidIndex(Index))
	{
		return false;
	}

	FQuestRuntimeData& RuntimeData = ActiveQuests[Index];
	if (RuntimeData.QuestState != EScenarioState::InProgress)
	{
		return false;
	}

	RuntimeData.QuestState = EScenarioState::Completed;

	FQuestRuntimeEvent CompletedEvent;
	CompletedEvent.QuestID = RuntimeData.QuestID;
	CompletedEvent.EventType = EQuestRuntimeEventType::Completed;
	OutEvents.Add(CompletedEvent);

	// 모든 필수 서브퀘스트가 끝나면 메인 퀘스트를 자동 완료합니다.
	if (AreRequiredChildQuestsCompleted(DataManager, ActiveQuests))
	{
		CompleteMainQuests(DataManager, ActiveQuests, OutEvents);
		bOutScenarioCompleted = true;
	}

	return true;
}

int32 FQuestRuntimeHelper::FindNextSubQuestIndex(
	const UDataManager* DataManager,
	const TArray<FQuestRuntimeData>& ActiveQuests,
	int32 AfterIndex)
{
	if (!DataManager)
	{
		return INDEX_NONE;
	}

	for (int32 Index = AfterIndex + 1; Index < ActiveQuests.Num(); ++Index)
	{
		const FQuestRuntimeData& RuntimeData = ActiveQuests[Index];
		if (RuntimeData.QuestState != EScenarioState::NotStarted)
		{
			continue;
		}

		const FQuestTableRow* QuestData = DataManager->GetQuestData(RuntimeData.QuestID);
		if (QuestData && QuestData->QuestType == EQuestType::SubQuest)
		{
			return Index;
		}
	}

	return INDEX_NONE;
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
