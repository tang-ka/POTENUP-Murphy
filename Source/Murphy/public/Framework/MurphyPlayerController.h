
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IHttpRequest.h"
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
	void Test_SendAIMessage(const FString& Message);
	UFUNCTION(Exec)
	void Test_SimulateAIResponse(const FString& SimulatedJSONResponse);

public:
	// AgentNPCBase가 Overlap 시 호출해 대화 타겟 NPC를 등록/해제
	void SetActiveNPC(AAgentNPCBase* NewNPC);
	AAgentNPCBase* GetTargetNPC() const { return TargetNPC; }

private:
	// 녹음 완료 델리게이트 바인딩 함수
	UFUNCTION()
	void OnAudioRecordingFinished(const FString& SavedFilePath);
	
	//! HTTP 통신 함수 -> NetSubsystem으로 넘김 
	// void SendVoiceFileToServer(const FString& WAVFilePath);		// wav 파일 그대로 통신
	// void SendVoiceDataAsJsonBase64(const FString& WAVFilePath);	// Base64로 인코딩 후 통신
	// void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	// NetSubsystem에서 전달해주는 AI 응답(JSON)을 받아 처리할 콜백
	UFUNCTION()
	void OnAIResponseReceived(const FString& ResponseData);
	
private:
	UPROPERTY()
	TObjectPtr<AAgentNPCBase> TargetNPC;
	
};
