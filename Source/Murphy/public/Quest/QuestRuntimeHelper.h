#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "QuestRuntimeHelper.generated.h"

class UDataManager;

UENUM()
enum class EQuestRuntimeEventType : uint8
{
	// 퀘스트가 NotStarted에서 InProgress로 바뀐 경우 UI 토스트 후보가 됩니다.
	Started,

	// 퀘스트가 InProgress에서 Completed로 바뀐 경우 완료 delegate 후보가 됩니다.
	Completed
};

USTRUCT()
struct FQuestRuntimeEvent
{
	GENERATED_BODY()

	UPROPERTY()
	FName QuestID;

	UPROPERTY()
	EQuestRuntimeEventType EventType = EQuestRuntimeEventType::Started;
};

/**
 * PlayerState/GameState가 같은 퀘스트 계산 규칙을 쓰도록 분리한 순수 런타임 helper입니다.
 */
class MURPHY_API FQuestRuntimeHelper
{
public:
	// ScenarioData의 RequiredQuestIDs를 런타임 상태 배열로 변환합니다.
	static void BuildScenarioRuntimeQuests(
		const UDataManager* DataManager,
		const FScenarioTableRow* ScenarioData,
		TArray<FQuestRuntimeData>& OutActiveQuests,
		TArray<FQuestRuntimeEvent>& OutEvents);

	// 하나의 이벤트를 완료 조건과 시작 조건 양쪽에 적용합니다. 예: 아이템 획득은 완료이면서 다음 퀘스트 시작 조건이 될 수 있습니다.
	static void NotifyQuestEvent(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName TargetID,
		EQuestStartCondition EventCondition,
		TArray<FQuestRuntimeEvent>& OutEvents,
		bool& bOutScenarioCompleted);

	// NPC 접근처럼 퀘스트 완료와 분리된 시작 이벤트만 처리할 때 사용합니다.
	static void NotifyQuestStartEvent(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName TargetID,
		EQuestStartCondition EventCondition,
		TArray<FQuestRuntimeEvent>& OutEvents);

	// Trigger, Item, NPC 대화 완료처럼 명확한 완료 조건을 처리할 때 사용합니다.
	static void NotifyQuestConditionMet(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName TargetID,
		EQuestClearCondition ClearCondition,
		TArray<FQuestRuntimeEvent>& OutEvents,
		bool& bOutScenarioCompleted);

	// 메인 퀘스트를 제외한 필수 하위 퀘스트가 모두 완료됐는지 확인합니다.
	static bool AreRequiredChildQuestsCompleted(
		const UDataManager* DataManager,
		const TArray<FQuestRuntimeData>& ActiveQuests);

	// 공유 시나리오 종료 정책에서 메인 퀘스트 완료 여부를 확인할 때 사용합니다.
	static bool AreMainQuestsCompleted(
		const UDataManager* DataManager,
		const TArray<FQuestRuntimeData>& ActiveQuests);

	static EQuestStartCondition ConvertClearConditionToStartCondition(EQuestClearCondition ClearCondition);
	static bool TryConvertStartConditionToClearCondition(EQuestStartCondition StartCondition, EQuestClearCondition& OutClearCondition);

private:
	// TMap 복제 리스크를 피하기 위해 TArray를 쓰고, QuestID로 런타임 데이터를 찾습니다.
	static FQuestRuntimeData* FindRuntimeQuest(TArray<FQuestRuntimeData>& ActiveQuests, FName QuestID);
	static const FQuestRuntimeData* FindRuntimeQuest(const TArray<FQuestRuntimeData>& ActiveQuests, FName QuestID);

	static void TryStartQuestsByEvent(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName TargetID,
		EQuestStartCondition EventCondition,
		TArray<FQuestRuntimeEvent>& OutEvents);

	static void TryCompleteQuestsByEvent(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName TargetID,
		EQuestClearCondition ClearCondition,
		TArray<FQuestRuntimeEvent>& OutEvents,
		bool& bOutScenarioCompleted);

	static bool StartQuest(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName QuestID,
		TArray<FQuestRuntimeEvent>& OutEvents);

	static bool CompleteQuest(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName QuestID,
		TArray<FQuestRuntimeEvent>& OutEvents,
		bool& bOutScenarioCompleted);

	static bool StartFirstSequentialSubQuest(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		TArray<FQuestRuntimeEvent>& OutEvents);

	static bool StartNextSequentialSubQuest(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		FName CompletedQuestID,
		TArray<FQuestRuntimeEvent>& OutEvents);

	static bool IsSequentialSubQuest(const UDataManager* DataManager, FName QuestID);

	static bool IsMainQuest(const UDataManager* DataManager, FName QuestID);
	static bool CompleteMainQuests(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		TArray<FQuestRuntimeEvent>& OutEvents);
};
