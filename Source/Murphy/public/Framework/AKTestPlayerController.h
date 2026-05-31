
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IHttpRequest.h"
#include "AKTestPlayerController.generated.h"

class AAgentNPCBase;

UCLASS()
class MURPHY_API AAKTestPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	// AgentNPCBase가 Overlap 시 호출해 대화 타겟 NPC를 등록/해제
	void SetActiveNPC(AAgentNPCBase* NewNPC);
	
private:
	// 녹음 완료 델리게이트 바인딩 함수
	UFUNCTION()
	void OnAudioRecordingFinished(const FString& SavedFilePath);
	
	// HTTP 통신 함수
	void SendVoiceFileToServer(const FString& FilePath);		// wav 파일 그대로 통신
	void SendVoiceDataAsJsonBase64(const FString& FilePath);	// Base64로 인코딩 후 통신
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

private:
	UPROPERTY()
	TObjectPtr<AAgentNPCBase> TargetNPC;
};
