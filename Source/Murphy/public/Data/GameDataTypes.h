
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameDataTypes.generated.h"

// ========================
// 시나리오 타입 Enum
// ========================
UENUM(BlueprintType)
enum class EScenarioType : uint8
{
	None					UMETA(DisplayName = "None"),
	Tutorial_Airplane		UMETA(DisplayName = "Tutorial_Airplane"),
	Prologue_Immigration	UMETA(DisplayName = "Prologue_Immigration"),
	Prologue_Baggage		UMETA(DisplayName = "Prologue_Baggage")
};

// ========================
// 시나리오 스탯 Enum
// ========================
UENUM(BlueprintType)
enum class EScenarioState : uint8
{
	NotStarted				UMETA(DisplayName = "NotStarted"),
	InProgress				UMETA(DisplayName = "InProgress"),
	Completed				UMETA(DisplayName = "Completed")
};

// ========================
// 퀘스트 타입 Enum
// ========================
UENUM(BlueprintType)
enum class EQuestType : uint8
{
	MainQuest				UMETA(DisplayName = "MainQuest"),
	SubQuest				UMETA(DisplayName = "SubQuest"),
	ToastQuest				UMETA(DisplayName = "ToastQuest")
};

// ========================
// 퀘스트 시작 조건
// ========================
UENUM(BlueprintType)
enum class EQuestStartCondition : uint8
{
	None					UMETA(DisplayName = "None"),
	ScenarioStart			UMETA(DisplayName = "Scenario Start"),
	QuestCompleted			UMETA(DisplayName = "Quest Completed"),
	CheckItem				UMETA(DisplayName = "Check Item"),
	ReachLocation			UMETA(DisplayName = "ReachLocation"),
	TalkToNPC				UMETA(DisplayName = "Talk to NPC"),
	GetItem					UMETA(DisplayName = "Get Item"),
	UseItem					UMETA(DisplayName = "Use Item")
};

// ========================
// 퀘스트 완료 조건
// ========================
UENUM(BlueprintType)
enum class EQuestClearCondition : uint8
{
	None					UMETA(DisplayName = "None"),
	CheckItem				UMETA(DisplayName = "Check Item"),
	ReachLocation			UMETA(DisplayName = "ReachLocation"),
	TalkToNPC				UMETA(DisplayName = "Talk to NPC"),
	GetItem					UMETA(DisplayName = "Get Item"),
	UseItem					UMETA(DisplayName = "Use Item")
};

// ========================
// 시나리오 DataTable 행 구조체
// CSV Row Name이 시나리오 ID 역할을 합니다 (예: Tutorial_Airplane)
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
	
	// 	UPROPERTY(BlueprintReadWrite, Category="Murphy|Scenario")
	// 	EScenarioState ScenarioState = EScenarioState::None;
};

// ========================
// 퀘스트 DataTable 행 구조체
// CSV Row Name이 퀘스트 ID 역할을 합니다 (예: Q_FindPassport)
// ========================
USTRUCT(BlueprintType)
struct FQuestTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 현재 진행 중인 퀘스트의 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Runtime")
	FName QuestID;
	
	/** 메인/서브 퀘스트 여부 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	EQuestType QuestType = EQuestType::MainQuest;

	/** 퀘스트 제목 텍스트 (비어있으면 토스트에서 "돌발 미션"으로 표시) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FText QuestTitle;

	/** 퀘스트 목표 텍스트 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FText QuestDescription;

	/** 퀘스트가 시작되는 조건 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	EQuestStartCondition StartCondition = EQuestStartCondition::None;

	/** 퀘스트 시작 조건 대상 ID (예: 선행 퀘스트 ID, NPC ID, 위치 ID, 아이템 ID 등) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FName StartTargetID;

	/** 퀘스트 완료 조건 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	EQuestClearCondition ClearCondition = EQuestClearCondition::None;

	/** 퀘스트 완료 대상 ID (예: 대화할 NPC ID, 획득할 아이템 ID, 도달할 위치 ID 등) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FName QuestTargetID;

	/** true면 이 퀘스트가 완료되어야 시나리오 종료 조건에 포함됩니다. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	bool bRequiredForScenarioEnd = true;

	/** true면 퀘스트 시작 시 토스트 알림을 표시합니다. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	bool bShowToastOnStart = true;
};

// ========================
// 퀘스트 런타임 진행 데이터 (동적 데이터)
// ========================
USTRUCT(BlueprintType)
struct FQuestRuntimeData
{
	GENERATED_BODY()

	// 현재 진행 중인 퀘스트의 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Runtime")
	FName QuestID;

	// 현재 퀘스트의 진행 상태 (대장님이 만드신 Enum 활용!)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Murphy|Runtime")
	EScenarioState QuestState = EScenarioState::NotStarted;
};

// ========================
// 아이템 정보 구조체
// DataTable 없이 ItemBaseActor 에디터에서 직접 설정
// ========================
USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 아이템 고유 ID (퀘스트 CSV의 QuestTargetID 컬럼값과 일치시킬 것) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	FName ItemID;

	/** UI에 표시할 아이템 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	FText ItemName;

	/** ItemDetailWidget에 표시할 아이템 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	FText ItemDescription;

	/** true → 클릭 시 즉시 사용 처리 / false → 상세 팝업만 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	bool bIsUsable = false;

	/** 아이템 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	TSoftObjectPtr<UTexture2D> ItemIcon;
	
	/** 아이템 Description 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	TSoftObjectPtr<UTexture2D> ItemDetailIcon;
};

