#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GameDataTypes.h"
#include "ScenarioSubsystem.generated.h"

class AMurphyGameStateBase;

// 새로운 시나리오로 넘어간 경우
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioStateChanged, EScenarioType, NewScenario);
// 시나리오 끝난 경우
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioEnded, EScenarioType, EndedScenario, bool, bSuccess);

/**
 * 기존 Blueprint/테스트 코드 호환을 위한 얇은 facade입니다.
 * 실제 시나리오/퀘스트 상태는 AMurphyGameStateBase와 AMurphyPlayerState가 관리합니다.
 */
UCLASS()
class MURPHY_API UScenarioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Murphy|Scenario")
	void StartScenario(EScenarioType NewScenario);

	UFUNCTION(BlueprintCallable, Category = "Murphy|Scenario")
	void EndScenario(bool bSuccess);

	UFUNCTION(BlueprintPure, Category = "Murphy|Scenario")
	bool IsInScenario() const;

	UFUNCTION(BlueprintPure, Category = "Murphy|Scenario")
	EScenarioType GetCurScenario() const;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Scenario|Delegates")
	FOnScenarioStateChanged OnScenarioStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Scenario|Delegates")
	FOnScenarioEnded OnScenarioEnded;

private:
	AMurphyGameStateBase* ResolveMurphyGameState() const;

	// GameState가 없는 로비/테스트 환경에서만 쓰는 최소 fallback 상태입니다.
	UPROPERTY()
	EScenarioType CurScenario = EScenarioType::None;
};
