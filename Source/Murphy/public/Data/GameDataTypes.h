
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameDataTypes.generated.h"

// ========================
// 플레이어 선택 캐릭터
// ========================
UENUM(BlueprintType)
enum class EPlayerCharacterType : uint8
{
	None			UMETA(DisplayName = "None"),
	BoyCharacter		UMETA(DisplayName = "Boy Character"),
	GirlCharacter		UMETA(DisplayName = "Girl Character")
};

// ========================
// 여행지
// ========================
UENUM(BlueprintType)
enum class ETravelDestination : uint8
{
	None			UMETA(DisplayName = "None"),
	NewYork			UMETA(DisplayName = "NewYork"),
	Tokyo			UMETA(DisplayName = "Tokyo"),
	Paris			UMETA(DisplayName = "Paris")
};

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
// 퀘스트 진행 범위 정책
// ========================
UENUM(BlueprintType)
enum class EQuestProgressScope : uint8
{
	Personal				UMETA(DisplayName = "Personal"),
	Shared					UMETA(DisplayName = "Shared")
};

// ========================
// 시나리오 종료 정책
// ========================
UENUM(BlueprintType)
enum class EScenarioEndPolicy : uint8
{
	AllPlayersCompleted		UMETA(DisplayName = "All Players Completed"),
	SharedQuestCompleted	UMETA(DisplayName = "Shared Quest Completed"),
	AnyPlayerCompleted		UMETA(DisplayName = "Any Player Completed")
};

// ========================
// 퀘스트/시나리오 런타임 알림 Delegate
// ========================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMurphyScenarioStateChanged, EScenarioType, NewScenario);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMurphyScenarioEnded, EScenarioType, EndedScenario, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMurphyQuestCompleted, FName, CompletedQuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMurphyQuestStarted, FName, QuestID, FText, QuestTitle, FText, QuestDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMurphyQuestStateChanged);

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
// 퀘스트 조건 (시작/완료 공용)
// ========================
UENUM(BlueprintType)
enum class EQuestCondition : uint8
{
	None					UMETA(DisplayName = "None"),
	ScenarioStart			UMETA(DisplayName = "Scenario Start"),    // 시작 전용
	QuestCompleted			UMETA(DisplayName = "Quest Completed"),   // 시작 전용 (선형 모델에서 자동 처리)
	TalkToNPC				UMETA(DisplayName = "Talk to NPC"),       // 시작/완료 공용
	ReachLocation			UMETA(DisplayName = "ReachLocation"),     // 시작/완료 공용
	CheckItem				UMETA(DisplayName = "Check Item"),        // 시작/완료 공용
	GetItem					UMETA(DisplayName = "Get Item"),          // 시작/완료 공용
	UseItem					UMETA(DisplayName = "Use Item")           // 시작/완료 공용
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
	EScenarioType ScenarioType = EScenarioType::None;

	/** 표시용 시나리오 이름 (임시, 기획 변경 시 수정) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	FText ScenarioName;

	/** 이 시나리오에서 클리어해야 할 퀘스트 ID 목록 (CSV Row Name 기준) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	TArray<FName> RequiredQuestIDs;

	/** 개인 진행/공유 진행 중 어떤 저장소를 사용할지 결정합니다. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	EQuestProgressScope QuestProgressScope = EQuestProgressScope::Personal;

	/** 시나리오 종료 조건을 데이터로 결정합니다. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Scenario")
	EScenarioEndPolicy ScenarioEndPolicy = EScenarioEndPolicy::AllPlayersCompleted;
	
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
	EQuestCondition StartCondition = EQuestCondition::None;

	/** 퀘스트 시작 조건 대상 ID (예: 선행 퀘스트 ID, NPC ID, 위치 ID, 아이템 ID 등) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	FName StartTargetID;

	/** 퀘스트 완료 조건 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Murphy|Data|Quest")
	EQuestCondition ClearCondition = EQuestCondition::None;

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

	/** 사용하기 버튼 클릭 시 띄울 위젯 클래스 (nullptr이면 위젯 없이 UseItem 이벤트만 발생) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	TSubclassOf<UUserWidget> UseWidgetClass;

	/** 아이템 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	TSoftObjectPtr<UTexture2D> ItemIcon;
	
	/** 아이템 Description 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	TSoftObjectPtr<UTexture2D> ItemDetailIcon;
};


// 방문 장소 (공통 속성)
USTRUCT(BlueprintType)
struct FLocationTextData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NameEN;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NameKR;
};

// 신고 물품 (공통 속성 상속, 텍스쳐 추가)
USTRUCT(BlueprintType)
struct FCustomsItemTextData : public FLocationTextData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemTexture;
	
};

// 결과창 티어 관련
USTRUCT(BlueprintType)
struct FTierUIDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 띄워줄 티어 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Tier")
	TObjectPtr<UTexture2D> TierIcon;

	// 띄워줄 칭호 (예: "베테랑 여행자")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Tier")
	FString TierTitle;
};