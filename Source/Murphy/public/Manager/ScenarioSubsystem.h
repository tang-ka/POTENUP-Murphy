
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GameDataTypes.h"
#include "ScenarioSubsystem.generated.h"

// 새로운 시나리오로 넘어간 경우
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioStateChanged, EScenarioType, NewScenario);
// 시나리오 끝난 경우 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioEnded, EScenarioType, EndedScenario, bool, bSuccess); 

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

	/** 현재 시나리오에서 진행해야 할 퀘스트 ID 목록 (CSV Row Name 기준) */
	UFUNCTION(BlueprintPure, Category="Murphy|Scenario")
	const TArray<FName>& GetActiveQuestIDs() const { return ActiveQuestIDs; }
	
public:
	UPROPERTY(BlueprintAssignable, Category="Murphy|Scenario|Delegates")
	FOnScenarioStateChanged OnScenarioStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Murphy|Scenario|Delegates")
	FOnScenarioEnded OnScenarioEnded;
	
private:
	UPROPERTY()
	EScenarioType CurScenario = EScenarioType::None;

	/** 현재 활성 시나리오의 퀘스트 ID 목록 (StartScenario 시 DataManager에서 로드) */
	UPROPERTY()
	TArray<FName> ActiveQuestIDs;
};
