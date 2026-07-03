#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "Components/PlayerViewComponent.h" // EChatViewMode 사용
#include "GameFramework/GameStateBase.h"
#include "MurphyGameStateBase.generated.h"

class AMurphyPlayerState;
class UCinematicSequenceData;
struct FQuestRuntimeEvent;

/**
 * 게임 종료 결과 상태
 */
UENUM(BlueprintType)
enum class EGameResultState : uint8
{
	Playing,
	GameOver,
	GameClear
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMurphyGameResultStateChanged, EGameResultState, NewState);

/**
 * 시나리오/공유 퀘스트의 서버 authoritative state를 들고 있는 공통 GameState입니다.
 */
UCLASS()
class MURPHY_API AMurphyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Murphy|Scenario")
	void StartScenario(EScenarioType NewScenario);

	UFUNCTION(BlueprintCallable, Category = "Murphy|Scenario")
	void EndScenario(bool bSuccess);

	UFUNCTION(BlueprintPure, Category = "Murphy|Scenario")
	bool IsInScenario() const { return CurrentScenario != EScenarioType::None; }

	UFUNCTION(BlueprintPure, Category = "Murphy|Scenario")
	EScenarioType GetCurrentScenario() const { return CurrentScenario; }

	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	const TArray<FQuestRuntimeData>& GetSharedActiveQuests() const { return SharedActiveQuests; }

	// 씬 단위 대화 시점 모드 (GameMode 값을 복제해 클라에도 전달)
	UFUNCTION(BlueprintPure, Category = "Murphy|View")
	EChatViewMode GetChatViewMode() const
	{
		return ChatViewMode;
	}

	// 서버 전용: GameMode가 InitGameState에서 호출해 복제 값 세팅
	void SetChatViewMode(EChatViewMode NewMode);

	// 이 레벨 진입 시 로컬 재생할 인트로 시네마틱 (레벨별 GameState BP에서 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Cinematic")
	TObjectPtr<UCinematicSequenceData> LevelCinematic;

	UFUNCTION(BlueprintPure, Category = "Murphy|Cinematic")
	UCinematicSequenceData* GetLevelCinematic() const
	{
		return LevelCinematic;
	}

protected:
	virtual void BeginPlay() override;

	const FScenarioTableRow* GetCurrentScenarioData() const;

	// PlayerController RPC가 도착하면 ScenarioData 정책에 따라 개인/공유 저장소로 라우팅합니다.
	void NotifyQuestStartEvent(AMurphyPlayerState* SourcePlayerState, FName TargetID, EQuestCondition EventCondition);
	void NotifyQuestConditionMet(AMurphyPlayerState* SourcePlayerState, FName TargetID, EQuestCondition Condition);

	// 개인 진행 시나리오에서 해당 플레이어가 완료됐는지 확인하고 완료 목록에 반영합니다.
	void CheckPersonalScenarioCompletion(AMurphyPlayerState* SourcePlayerState);

	// ScenarioEndPolicy에 따라 시나리오 종료 가능 여부를 최종 판단합니다.
	void TryEndScenarioByPolicy();

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Scenario|Delegates")
	FOnMurphyScenarioStateChanged OnScenarioStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Scenario|Delegates")
	FOnMurphyScenarioEnded OnScenarioEnded;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Quest|Delegates")
	FOnMurphyQuestStarted OnSharedQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Quest|Delegates")
	FOnMurphyQuestCompleted OnSharedQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Quest|Delegates")
	FOnMurphyQuestStateChanged OnSharedQuestStateChanged;

	// 게임 결과 상태가 변경될 때 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Murphy|GameResult|Delegates")
	FOnMurphyGameResultStateChanged OnGameResultStateChanged;

	// 서버에서 결과 상태 변경 (GameMode에서 호출)
	void SetGameResultState(EGameResultState NewState);

protected:
	UFUNCTION()
	void OnRep_CurrentScenario();

	UFUNCTION()
	void OnRep_SharedActiveQuests(TArray<FQuestRuntimeData> OldSharedActiveQuests);

	UFUNCTION()
	void OnRep_ScenarioCompletedPlayers();

	UFUNCTION()
	void OnRep_ChatViewMode();

private:
	// 개인 시나리오는 모든 플레이어의 PlayerState에 각각 같은 시나리오를 시작시킵니다.
	void StartPersonalScenarioForAllPlayers(const FScenarioTableRow* ScenarioData);

	// 공유 시나리오는 GameState의 SharedActiveQuests 하나만 생성합니다.
	void StartSharedScenario(const FScenarioTableRow* ScenarioData);
	void ClearPersonalScenarioForAllPlayers();

	void NotifySharedQuestStartEvent(FName TargetID, EQuestCondition EventCondition);
	void NotifySharedQuestConditionMet(FName TargetID, EQuestCondition Condition);

	bool AreAllPlayersCompleted() const;
	bool IsSharedScenarioCompleted() const;
	void AddScenarioCompletedPlayer(AMurphyPlayerState* SourcePlayerState);

	// 서버 즉시 실행과 클라이언트 RepNotify 양쪽에서 같은 UI delegate를 발생시키기 위한 helper입니다.
	void BroadcastQuestRuntimeEvents(const TArray<FQuestRuntimeEvent>& Events);
	void BroadcastQuestDeltaEvents(const TArray<FQuestRuntimeData>& OldActiveQuests);
	void BroadcastQuestStarted(FName QuestID);

	// 현재 서버가 authoritative하게 관리하는 시나리오입니다.
	UPROPERTY(ReplicatedUsing = OnRep_CurrentScenario, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Scenario", meta = (AllowPrivateAccess = "true"))
	EScenarioType CurrentScenario = EScenarioType::None;

	// 수화물 수취장처럼 모든 플레이어가 함께 진행하는 퀘스트 상태입니다.
	UPROPERTY(ReplicatedUsing = OnRep_SharedActiveQuests, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Quest", meta = (AllowPrivateAccess = "true"))
	TArray<FQuestRuntimeData> SharedActiveQuests;

	// 현재 진행 중인 공유 서브퀘스트의 SharedActiveQuests 내 배열 인덱스입니다.
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Quest", meta = (AllowPrivateAccess = "true"))
	int32 SharedCurrentSubQuestIndex = INDEX_NONE;

	// AllPlayersCompleted / AnyPlayerCompleted 정책 판단에 쓰는 개인 완료 플레이어 목록입니다.
	UPROPERTY(ReplicatedUsing = OnRep_ScenarioCompletedPlayers, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Scenario", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AMurphyPlayerState>> ScenarioCompletedPlayers;

	// 현재 게임 클리어/오버 상태
	UPROPERTY(ReplicatedUsing = OnRep_GameResultState, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|GameResult", meta = (AllowPrivateAccess = "true"))
	EGameResultState CurrentResultState = EGameResultState::Playing;

	// GameMode의 씬 단위 시점 설정을 클라에 전달하기 위한 복제 값
	UPROPERTY(ReplicatedUsing = OnRep_ChatViewMode, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|View", meta = (AllowPrivateAccess = "true"))
	EChatViewMode ChatViewMode = EChatViewMode::ThirdPersonFocus;

	// 레벨 진입 시 로컬 인트로 재생 (머신당 1회)
	void TryPlayLevelIntro();
	bool bLevelIntroPlayed = false;

	UFUNCTION()
	void OnRep_GameResultState();
};
