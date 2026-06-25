
#pragma once

#include "CoreMinimal.h"
#include "Data/AIResultDataTypes.h"
#include "Data/GameDataTypes.h"
#include "Data/PlayReportData.h"
#include "GameFramework/PlayerState.h"
#include "MurphyPlayerState.generated.h"

struct FQuestRuntimeEvent;

/**
 * 모든 레벨에서 공통으로 사용하는 PlayerState
 */

// 데이터가 업데이트되었음을 UI에게 알림
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCardDataUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayReportDataUpdated);

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

#pragma region Play report state
public:
	// 플레이 전체에서 공유할 AI 세션 ID를 가져오고, 없으면 새로 생성합니다.
	UFUNCTION(BlueprintCallable, Category = "Murphy|AI")
	FString GetOrCreateAIPlaySessionId();

	UFUNCTION(BlueprintPure, Category = "Murphy|AI")
	FString GetAIPlaySessionId() const { return AIPlaySessionId; }

	UFUNCTION(BlueprintCallable, Category = "Murphy|AI")
	void SetAIPlaySessionId(const FString& InSessionId);

	// AI 최종 결과를 저장하고 UI 표시용 데이터로 변환합니다.
	UFUNCTION(BlueprintCallable, Category = "Murphy|PlayReport")
	void SaveAIResult(const FAIResultResponse& InResult);

	UFUNCTION(BlueprintPure, Category = "Murphy|PlayReport")
	FAIResultResponse GetLastAIResult() const { return LastAIResult; }

	UFUNCTION(BlueprintPure, Category = "Murphy|PlayReport")
	FPlayReportData GetLastPlayReportData() const { return LastPlayReportData; }

	UFUNCTION(BlueprintPure, Category = "Murphy|PlayReport")
	bool HasPlayReportData() const { return bHasPlayReportData; }

	UPROPERTY(BlueprintAssignable, Category = "Murphy|PlayReport|Delegate")
	FOnPlayReportDataUpdated OnPlayReportDataUpdated;

private:
	// AI 결과 원본을 기존 점수판 UI에서 쓰기 쉬운 데이터로 변환합니다.
	FPlayReportData BuildPlayReportDataFromAIResult(const FAIResultResponse& InResult) const;

	// 플레이 전체에서 공유하는 AI 세션 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|AI", meta = (AllowPrivateAccess = "true"))
	FString AIPlaySessionId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|PlayReport", meta = (AllowPrivateAccess = "true"))
	FAIResultResponse LastAIResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|PlayReport", meta = (AllowPrivateAccess = "true"))
	FPlayReportData LastPlayReportData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|PlayReport", meta = (AllowPrivateAccess = "true"))
	bool bHasPlayReportData = false;
#pragma endregion


#pragma region Session room state
public:
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
	void NotifyPersonalQuestStartEvent(FName TargetID, EQuestCondition EventCondition);
	void NotifyPersonalQuestConditionMet(FName TargetID, EQuestCondition Condition);

	// 현재 진행 중인 서브퀘스트의 PersonalActiveQuests 내 인덱스입니다. (INDEX_NONE = 없음)
	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	int32 GetPersonalCurrentSubQuestIndex() const { return PersonalCurrentSubQuestIndex; }

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

	// 현재 진행 중인 서브퀘스트의 PersonalActiveQuests 내 배열 인덱스입니다.
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Quest", meta = (AllowPrivateAccess = "true"))
	int32 PersonalCurrentSubQuestIndex = INDEX_NONE;

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

