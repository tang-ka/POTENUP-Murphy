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
 * 퀘스트는 항상 RequiredQuestIDs 순서대로 순차 진행되므로 CurrentSubQuestIndex 기반 O(1) 탐색을 사용합니다.
 */
class MURPHY_API FQuestRuntimeHelper
{
public:
	// ScenarioData의 RequiredQuestIDs를 런타임 상태 배열로 변환하고, 첫 서브퀘스트의 인덱스를 반환합니다.
	// 반환값: 첫 번째 InProgress 서브퀘스트의 배열 인덱스 (없으면 INDEX_NONE)
	static int32 BuildScenarioRuntimeQuests(
		const UDataManager* DataManager,
		const FScenarioTableRow* ScenarioData,
		TArray<FQuestRuntimeData>& OutActiveQuests,
		TArray<FQuestRuntimeEvent>& OutEvents);

	// NPC 접근처럼 시작 조건만 처리할 때 사용합니다.
	// 현재 인덱스 퀘스트의 StartCondition과 일치할 때만 InProgress로 전환합니다.
	static void ProcessQuestStartEvent(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		int32& InOutCurrentSubQuestIndex,
		FName TargetID,
		EQuestCondition Condition,
		TArray<FQuestRuntimeEvent>& OutEvents);

	// Trigger, Item, NPC 대화 완료처럼 완료 조건을 처리할 때 사용합니다.
	// 현재 인덱스 서브퀘스트를 완료하고 다음 서브퀘스트로 인덱스를 전진합니다.
	static void ProcessQuestConditionMet(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		int32& InOutCurrentSubQuestIndex,
		FName TargetID,
		EQuestCondition Condition,
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

private:
	// TMap 복제 리스크를 피하기 위해 TArray를 쓰고, QuestID로 런타임 데이터를 찾습니다.
	static FQuestRuntimeData* FindRuntimeQuest(TArray<FQuestRuntimeData>& ActiveQuests, FName QuestID);
	static const FQuestRuntimeData* FindRuntimeQuest(const TArray<FQuestRuntimeData>& ActiveQuests, FName QuestID);

	// ActiveQuests[Index]를 InProgress로 전환합니다.
	static bool StartQuestAtIndex(
		TArray<FQuestRuntimeData>& ActiveQuests,
		int32 Index,
		TArray<FQuestRuntimeEvent>& OutEvents);

	// ActiveQuests[Index]를 Completed로 전환하고 이후 메인 퀘스트 자동 완료를 처리합니다.
	static bool CompleteQuestAtIndex(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		int32 Index,
		TArray<FQuestRuntimeEvent>& OutEvents,
		bool& bOutScenarioCompleted);

	// Index 이후에서 다음 SubQuest의 배열 인덱스를 찾습니다. 없으면 INDEX_NONE.
	static int32 FindNextSubQuestIndex(
		const UDataManager* DataManager,
		const TArray<FQuestRuntimeData>& ActiveQuests,
		int32 AfterIndex);

	static bool IsMainQuest(const UDataManager* DataManager, FName QuestID);
	static bool CompleteMainQuests(
		const UDataManager* DataManager,
		TArray<FQuestRuntimeData>& ActiveQuests,
		TArray<FQuestRuntimeEvent>& OutEvents);
};
