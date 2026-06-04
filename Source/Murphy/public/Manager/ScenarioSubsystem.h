
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScenarioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EScenarioType : uint8
{
	None, 
	Tutorial_Airplane			UMETA(DisplayName = "Tutorial_Airplane"),
	Prologue_Immigration		UMETA(DisplayName = "Prologue_Immigration"),
	Prologue_Baggage			UMETA(DisplayName = "Prologue_Baggage")
};

UENUM(BlueprintType)
enum class EScenarioState : uint8
{
	None, 
	Enter						UMETA(DisplayName = "Enter"),
	InProgress					UMETA(DisplayName = "InProgress"),
	Completed					UMETA(DisplayName = "Completed")
};

USTRUCT(BlueprintType)
struct FScenarioInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="Murphy|Scenario")
	EScenarioType ScenarioType = EScenarioType::None;
	
	UPROPERTY(BlueprintReadWrite, Category="Murphy|Scenario")
	EScenarioState ScenarioState = EScenarioState::None;
};

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
	
public:
	UPROPERTY(BlueprintAssignable, Category="Murphy|Scenario|Delegates")
	FOnScenarioStateChanged OnScenarioStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Murphy|Scenario|Delegates")
	FOnScenarioEnded OnScenarioEnded;
	
private:
	UPROPERTY()
	EScenarioType CurScenario = EScenarioType::None;
};
