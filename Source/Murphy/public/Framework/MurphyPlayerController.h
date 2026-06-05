
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IHttpRequest.h"
#include "Data/AIDataTypes.h"
#include "MurphyPlayerController.generated.h"

class AAgentNPCBase;
class UInputAction;

UCLASS()
class MURPHY_API AMurphyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
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

	// SubLevel_Immigration을 언로드하고 SubLevel_BaggageClaim으로 전환 (로컬 클라이언트 전용)
	UFUNCTION(BlueprintCallable, Category="Level Streaming")
	void TransitionToBaggageClaim();

	
private:
	// 녹음 완료 델리게이트 바인딩 함수
	UFUNCTION()
	void OnAudioRecordingFinished(const FString& SavedFilePath);
	
	// NetSubsystem에서 전달해주는 AI 응답 구조체를 받아 처리할 콜백
	UFUNCTION()
	void OnAIResponseReceived(const FAIResponseData& ResponseData);
	
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
	
private:
	UPROPERTY()
	TObjectPtr<AAgentNPCBase> TargetNPC;
	
	// --- AI Session State ---
	FString CurrentSessionId = TEXT("session_001");
	FString CurrentNodeId = TEXT("IMM_002_PURPOSE");
	int32 TurnIndex = 1;
	FString LastNpcMessage = TEXT("What is the purpose of your visit?");
	
	FAI_ScenarioState CurrentScenarioState;
};
