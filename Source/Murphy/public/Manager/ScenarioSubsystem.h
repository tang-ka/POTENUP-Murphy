
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GameDataTypes.h"
#include "ScenarioSubsystem.generated.h"

// 새로운 시나리오로 넘어간 경우
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioStateChanged, EScenarioType, NewScenario);
// 시나리오 끝난 경우 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioEnded, EScenarioType, EndedScenario, bool, bSuccess); 
// 완료한 퀘스트 Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FName, CompletedQuestID);
// SubQuest가 새로 시작된 경우 (QuestID, Title, Description)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestStarted, FName, QuestID, FText, QuestTitle, FText, QuestDescription);

UCLASS()
class MURPHY_API UScenarioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="Murphy|Scenario")
	void StartScenario(EScenarioType NewScenario);
	UFUNCTION(BlueprintCallable, Category="Murphy|Scenario")
	void EndScenario(bool bSuccess);
	
	UFUNCTION(BlueprintPure, Category="Murphy|Scenario")
	bool IsInScenario() const { return CurScenario != EScenarioType::None; }
	
	UFUNCTION(BlueprintPure, Category="Murphy|Scenario")
	EScenarioType GetCurScenario() const { return CurScenario; }

	// 퀘스트 완료 처리 함수
	UFUNCTION(BlueprintCallable, Category="Murphy|Quest")
	void CompleteQuest(FName QuestID);
	
	// 특정 조건 만족 시 수동으로 퀘스트를 시작합니다 (예: 서브퀘스트 발생)
	UFUNCTION(BlueprintCallable, Category="Murphy|Quest")
	void StartQuest(FName QuestID);
	
	// 특정 조건과 대상 ID를 가진 이벤트를 수신하여 퀘스트를 달성 처리합니다.
	UFUNCTION(BlueprintCallable, Category="Murphy|Quest")
	void NotifyQuestConditionMet(FName TargetID, EQuestClearCondition Condition);
	
	/** 현재 진행 중인 퀘스트의 전체 데이터(상태 포함) 반환 */
	UFUNCTION(BlueprintPure, Category="Murphy|Scenario")
	const TMap<FName, FQuestRuntimeData>& GetActiveQuests() const { return ActiveQuests; }
	
public:
	UPROPERTY(BlueprintAssignable, Category="Murphy|Scenario|Delegates")
	FOnScenarioStateChanged OnScenarioStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Murphy|Scenario|Delegates")
	FOnScenarioEnded OnScenarioEnded;
	
	UPROPERTY(BlueprintAssignable, Category="Murphy|Quest|Delegates")
	FOnQuestCompleted OnQuestCompleted;
	
	UPROPERTY(BlueprintAssignable, Category="Murphy|Quest|Delegates")
	FOnQuestStarted OnQuestStarted;
	
private:
	// 모든 활성 퀘스트가 완료되었는지 검사합니다.
	void CheckAllQuestsCompleted();

	UPROPERTY()
	EScenarioType CurScenario = EScenarioType::None;

	// 퀘스트 ID를 Key로, 진행 상태가 담긴 구조체를 Value로 가집니다.
	UPROPERTY()
	TMap<FName, FQuestRuntimeData> ActiveQuests;
};
