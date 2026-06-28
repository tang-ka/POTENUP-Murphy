
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IHttpRequest.h"
#include "Data/AIDataTypes.h"
#include "Data/CinematicTypes.h"
#include "Data/GameDataTypes.h"
#include "Data/STTDataTypes.h"
#include "MurphyPlayerController.generated.h"

class AAgentNPCBase;
class UInputAction;
class UQuestEventNotifyComponent;

UCLASS()
class MURPHY_API AMurphyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMurphyPlayerController();
	
	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// === 통합 테스트 커맨드 ===
	UFUNCTION(Exec)
	void Test_StartScenario(int32 ScenarioIndex);
	UFUNCTION(Exec)
	void Test_EndScenarioAndTravel(FName NextLevelKey);

	UFUNCTION(Exec)
	void Test_SimulateAIResponse(const FString& SimulatedJSONResponse);
	
public:
	// AgentNPCBase가 Overlap 시 호출해 대화 타겟 NPC를 등록/해제
	void SetActiveNPC(AAgentNPCBase* NewNPC);
	AAgentNPCBase* GetTargetNPC() const { return TargetNPC; }

	// UI처럼 ActorComponent를 직접 소유하지 않는 호출자가 퀘스트 완료 조건을 통보하는 로컬 진입점입니다.
	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest")
	bool NotifyQuestConditionFromLocal(FName TargetID, EQuestCondition Condition);

	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	UQuestEventNotifyComponent* GetQuestEventNotifier() const { return QuestEventNotifier; }

	// SubLevel_Immigration을 언로드하고 SubLevel_BaggageClaim으로 전환 (로컬 클라이언트 전용)
	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	void TransitionToBaggageClaim();

	// 퀘스트 시작 조건만 서버로 전달합니다. NPC 접근처럼 완료와 분리해야 할 때 사용합니다.
	UFUNCTION(Server, Reliable)
	void ServerNotifyQuestStartEvent(FName TargetID, EQuestCondition StartCondition);

	// 퀘스트 완료 조건을 서버로 전달합니다. 서버 GameState가 개인/공유 정책에 따라 라우팅합니다.
	UFUNCTION(Server, Reliable)
	void ServerNotifyQuestConditionMet(FName TargetID, EQuestCondition Condition);

private:
	void BindLocalQuestStateSources();
	void EnsurePrologueRequiredItemsInBag();
	void RefreshLocalBagFromOwnedItems();

	//. 테스트 전용: N 키 입력을 받아 현재 진행 중인 서브퀘스트를 서버에서 강제 완료합니다.
	void HandleAdvanceSubQuestTestKey();

	//. 테스트 전용: 현재 시나리오의 진행 저장소를 찾아 현재 서브퀘스트 완료 이벤트를 발생시킵니다.
	bool AdvanceCurrentSubQuestForTest();

	//. GameState/PlayerState의 TArray 저장소에서 RequiredQuestIDs 순서상 현재 진행 중인 SubQuest를 찾습니다.
	bool ResolveCurrentSubQuestForTest(const TArray<FName>& QuestOrder, const TArray<FQuestRuntimeData>& ActiveQuests, FName& OutQuestID, FName& OutTargetID, EQuestCondition& OutClearCondition) const;

	//. 테스트 전용: 클라이언트 키 입력으로 서버 권한 퀘스트 상태를 변경하기 위한 RPC입니다.
	UFUNCTION(Server, Reliable)
	void ServerAdvanceCurrentSubQuestForTest();

	// 녹음 완료 델리게이트 바인딩 함수
	UFUNCTION()
	void OnAudioRecordingFinished(const FString& SavedFilePath);
	
public:
	// 공통: 현재 세션 상태를 바탕으로 AI 요청 데이터를 생성
	FAIRequestData GenerateAIRequestData();

	// Realtime STT용: 현재 세션 상태를 바탕으로 /stt/stream 세션 시작 데이터까지 생성
	bool BuildRealtimeSTTTurnData(FAIRequestData& OutRequestData, FSTT_SessionStart& OutSessionPayload);

	// Realtime STT final transcript를 /respond로 전달
	bool SendRealtimeSTTTranscriptToAI(const FAIRequestData& RequestData, const FString& FinalText);

	// 지정된 결과 레벨 진입 시 AI 최종 점수판 신호를 전송합니다.
	UFUNCTION(BlueprintCallable, Category = "Murphy|PlayReport")
	void NotifyAIResultTriggerLevelEntered(FName EnteredLevelName);

	// 현재 AI 플레이 세션으로 최종 점수판 신호를 즉시 전송합니다.
	UFUNCTION(BlueprintCallable, Category = "Murphy|PlayReport")
	void TriggerFinalScoreboardSignal();

	// NetSubsystem에서 전달해주는 AI 응답 구조체를 받아 처리할 콜백
	UFUNCTION()
	void OnAIResponseReceived(const FAIResponseData& ResponseData);

	// 최종 결과 조회 API 응답을 PlayerState에 저장합니다.
	UFUNCTION()
	void OnAIResultReceived(const FAIResultResponse& ResultData);

	// 최종 점수판 신호 응답 후 결과 조회 API를 호출합니다.
	UFUNCTION()
	void OnFinalScoreboardSignalResponse(const FAIResponseData& ResponseData);
	
	// 1분 타임아웃 시 AgentNPCBase가 호출할 함수
	void SendTimeoutAudioToAI();
	
#pragma region Level Enter Toast
	void SubscribeLevelEnterEvents();

	UFUNCTION()
	void OnImmigrationLevelShown();

	UFUNCTION()
	void OnImmigrationLevelHidden();

	UFUNCTION()
	void OnBaggageClaimLevelShown();
#pragma endregion
	
	UFUNCTION(Server, Reliable)
	void Server_RequestReposition(const FName& SubLevelName);

	UFUNCTION(Server, Reliable)
	void ServerStartScenarioForTest(EScenarioType NewScenario);

	UFUNCTION(Server, Reliable)
	void ServerEndScenarioForTest(bool bSuccess);

	// Airplane 등 진입 시 서버가 발급한 PlayId로 로컬에서 시네마틱 재생.
	UFUNCTION(Client, Reliable)
	void Client_PlayCinematic(const FCinematicPlayRequest& Request, int32 PlayId);
	
private:
	FString GetOrCreateAIPlaySessionId();
	FAIRequestData GenerateFinalScoreboardSignalRequestData();
	bool IsAIResultTriggerResponse(const FAIResponseData& ResponseData) const;
	bool ShouldTriggerFinalScoreboardForLevel(FName EnteredLevelName) const;
	void RequestAIResultForSession(const FString& SessionId);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|PlayReport", meta = (AllowPrivateAccess = "true"))
	FName FinalScoreboardTriggerLevelName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|PlayReport", meta = (AllowPrivateAccess = "true"))
	FString FinalScoreboardNodeId = TEXT("ALPHA_999_FINAL_SCOREBOARD");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|PlayReport", meta = (AllowPrivateAccess = "true"))
	bool bFinalScoreboardSignalSent = false;

	UPROPERTY()
	TObjectPtr<AAgentNPCBase> TargetNPC;

	// 가방 UI 등 비Actor 호출부도 같은 퀘스트 통보 경로를 쓰도록 PlayerController가 소유합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UQuestEventNotifyComponent> QuestEventNotifier;
};
