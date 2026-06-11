
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameDataTypes.generated.h"

// ========================
// 시나리오 타입 Enum
// (기존 UScenarioSubsystem.h에서 이곳으로 이동)
// ========================
UENUM(BlueprintType)
enum class EScenarioType : uint8
{
	None, 
	Tutorial_Airplane			UMETA(DisplayName = "Tutorial_Airplane"),
	Prologue_Immigration		UMETA(DisplayName = "Prologue_Immigration"),
	Prologue_Baggage			UMETA(DisplayName = "Prologue_Baggage")
};

// ========================
// 시나리오 스탯 Enum
// (기존 UScenarioSubsystem.h에서 이곳으로 이동)
// ========================
UENUM(BlueprintType)
enum class EScenarioState : uint8
{
	None, 
	Enter						UMETA(DisplayName = "Enter"),
	InProgress					UMETA(DisplayName = "InProgress"),
	Completed					UMETA(DisplayName = "Completed")
};

// ========================
// 퀘스트 타입 Enum
// (기존 QuestEntryWidget.h에서 이곳으로 이동)
// ========================
UENUM(BlueprintType)
enum class EQuestType : uint8
{
	MainQuest	UMETA(DisplayName = "MainQuest"),
	SubQuest	UMETA(DisplayName = "SubQuest")
};

// FScenarioInfo ========================
// (기존 UScenarioSubsystem.h에서 이곳으로 이동)
// ========================
// USTRUCT(BlueprintType)
// struct FScenarioInfo
// {
// 	GENERATED_BODY()
// 	
// 	UPROPERTY(BlueprintReadWrite, Category="Murphy|Scenario")
// 	EScenarioType ScenarioType = EScenarioType::None;
// 	
// 	UPROPERTY(BlueprintReadWrite, Category="Murphy|Scenario")
// 	EScenarioState ScenarioState = EScenarioState::None;
// };

// ========================
// 시나리오 DataTable 행 구조체
// CSV Row Name이 시나리오 ID 역할을 합니다 (예: S_Airplane)
// ========================
USTRUCT(BlueprintType)
struct FScenarioTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 코드 매핑용 시나리오 Enum */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	EScenarioType ScenarioType;

	/** 표시용 시나리오 이름 (임시, 기획 변경 시 수정) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	FText ScenarioName;

	/** 이 시나리오에서 클리어해야 할 퀘스트 ID 목록 (CSV Row Name 기준) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	TArray<FName> RequiredQuestIDs;
};

// ========================
// 퀘스트 DataTable 행 구조체
// CSV Row Name이 퀘스트 ID 역할을 합니다 (예: Q_FindPassport)
// ========================
USTRUCT(BlueprintType)
struct FQuestTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 메인/서브 퀘스트 여부 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	EQuestType QuestType = EQuestType::MainQuest;

	/** 퀘스트 목표 텍스트 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FText ObjectiveText;

	/** 상호작용할 대상 NPC ID */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FName TargetNPC_ID;
};
