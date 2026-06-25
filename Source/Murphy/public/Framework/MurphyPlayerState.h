
#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "GameFramework/PlayerState.h"
#include "MurphyPlayerState.generated.h"

struct FQuestRuntimeEvent;

/**
 * 모든 레벨에서 공통으로 사용하는 PlayerState
 */

// 데이터가 업데이트되었음을 UI에게 알림
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCardDataUpdated);

UCLASS()
class MURPHY_API AMurphyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

protected:
	UFUNCTION()
	void OnRep_SessionRoomState();

	UFUNCTION()
	void OnRep_PersonalScenario();

	UFUNCTION()
	void OnRep_PersonalActiveQuests(TArray<FQuestRuntimeData> OldPersonalActiveQuests);

	UFUNCTION()
	void OnRep_CompletedPersonalScenarios();
	
public:
	// AI 대화 결과 저장용 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Murphy|State")
	FString LastDialogResult;
	
	// AI Agent NPC 호감도
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Murphy|State")
	int32 NPCAffection;
	
	// 시나리오 성공 상태 저장 (필요 시 확장)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Murphy|State")
	bool bPassedCurrentScenario = false;
	

#pragma region Session room state
	// 선택한 캐릭터 (Carry)
	UPROPERTY(ReplicatedUsing = OnRep_SessionRoomState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session")
	EPlayerCharacterType  SelectedCharacter = EPlayerCharacterType::None;
	
	// 준비 완료 여부 (Room only)
	UPROPERTY(ReplicatedUsing = OnRep_SessionRoomState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session")
	bool bIsReady = false;
	
	// 호스트 여부
	UPROPERTY(ReplicatedUsing = OnRep_SessionRoomState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session")
	bool bIsHost = false;
	
	// 룸 상태 복제 시 위젯 갱신용 델리게이트
	FSimpleMulticastDelegate OnSessionRoomStateChanged;	
#pragma endregion 

#pragma region Personal quest state
public:
	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	EScenarioType GetPersonalScenario() const { return PersonalScenario; }

	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	const TArray<FQuestRuntimeData>& GetPersonalActiveQuests() const { return PersonalActiveQuests; }

	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	bool HasCompletedPersonalScenario(EScenarioType ScenarioType) const;

	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	bool IsCurrentPersonalScenarioCompleted() const;

	void StartPersonalScenario(EScenarioType NewScenario, const FScenarioTableRow* ScenarioData);
	void ClearPersonalScenario();
	void NotifyPersonalQuestEvent(FName TargetID, EQuestStartCondition EventCondition);
	void NotifyPersonalQuestStartEvent(FName TargetID, EQuestStartCondition EventCondition);
	void NotifyPersonalQuestConditionMet(FName TargetID, EQuestClearCondition Condition);

	// 이 PlayerState를 소유한 클라이언트에게만 개인 퀘스트 토스트를 띄울 때 사용합니다.
	UPROPERTY(BlueprintAssignable, Category = "Murphy|Quest|Delegates")
	FOnMurphyQuestStarted OnPersonalQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Quest|Delegates")
	FOnMurphyQuestCompleted OnPersonalQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Murphy|Quest|Delegates")
	FOnMurphyQuestStateChanged OnPersonalQuestStateChanged;

private:
	// 서버 즉시 실행과 클라이언트 RepNotify 양쪽에서 같은 퀘스트 알림을 발생시키기 위한 helper입니다.
	void BroadcastQuestRuntimeEvents(const TArray<FQuestRuntimeEvent>& Events);
	void BroadcastQuestDeltaEvents(const TArray<FQuestRuntimeData>& OldActiveQuests);
	void BroadcastQuestStarted(FName QuestID);
	void MarkCurrentPersonalScenarioCompleted();

	// 개인 진행 시나리오에서 현재 플레이어가 진행 중인 시나리오입니다.
	UPROPERTY(ReplicatedUsing = OnRep_PersonalScenario, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Quest", meta = (AllowPrivateAccess = "true"))
	EScenarioType PersonalScenario = EScenarioType::None;

	// 개인 퀘스트 진행도입니다. TMap 복제 이슈를 피하기 위해 QuestID를 가진 배열로 관리합니다.
	UPROPERTY(ReplicatedUsing = OnRep_PersonalActiveQuests, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Quest", meta = (AllowPrivateAccess = "true"))
	TArray<FQuestRuntimeData> PersonalActiveQuests;

	// GameState가 AllPlayersCompleted 정책을 판단할 때 참조하는 개인 시나리오 완료 목록입니다.
	UPROPERTY(ReplicatedUsing = OnRep_CompletedPersonalScenarios, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Quest", meta = (AllowPrivateAccess = "true"))
	TArray<EScenarioType> CompletedPersonalScenarios;
#pragma endregion

#pragma region Arrival Card Data
public:
	// === Arrival Card ===
	// 입국심사서에 입력한 이름 저장
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Murphy|CardData")
	FString SavedSurname;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Murphy|CardData")
	FString SavedGivenname;
	
	// AI가 배정한 장소 ID
	UPROPERTY(ReplicatedUsing = OnRep_ArrivalData, BlueprintReadOnly, Category = "Murphy|CardData")
	FString CurrentLocationID;

	// AI가 배정한 신고물품 ID
	UPROPERTY(ReplicatedUsing = OnRep_ArrivalData, BlueprintReadOnly, Category = "Murphy|CardData")
	FString CurrentItemID;

	// 클라->서버 이름 저장 요청
	UFUNCTION(Server, Reliable)
	void ServerSetArrivalData(const FString& InSurname, const FString& InGivenname);
	
	// 서버->클라 ID 데이터 도착하면 자동 실행
	UFUNCTION()
	void OnRep_ArrivalData();
	
	UPROPERTY(BlueprintAssignable, Category = "Murphy|CardData|Delegate")
	FOnCardDataUpdated OnArrivalDataUpdated;
#pragma endregion Arrival Card Data
	
};

