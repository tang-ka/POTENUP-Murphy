
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h" 
#include "Interfaces/IHttpResponse.h"
#include "Data/AIDataTypes.h"
#include "NetSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAIResponseReceived, const FString&, ResponseData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAIResponseDataReceived, const FAIResponseData&, ResponseData);

UCLASS()
class MURPHY_API UNetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// [New] AI 서버로 RequestData 구조체와 음성파일 전송 (Multipart)
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void SendToAI(const FAIRequestData& RequestData, const FString& WAVFilePath, FOnAIResponseDataReceived OnResponseDelegate);

	/**
	 * [New] Realtime STT WebSocket 방식 전용:
	 * WAV 파일 없이 final transcript 텍스트만으로 /respond 호출.
	 * JSON body 형식: { "turn": <FAIRequestData>, "audio": { "transcript": "<Transcript>" } }
	 * @param RequestData       WS 세션 중 사전 준비해 둔 turn 데이터
	 * @param Transcript        STT WebSocket에서 확정된 final transcript 텍스트
	 * @param OnResponseDelegate 응답 수신 시 콜백
	 */
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void SendToAIWithTranscript(const FAIRequestData& RequestData, const FString& Transcript, FOnAIResponseDataReceived OnResponseDelegate);
	
	// 진행중인 요청(구독) 강제 취소
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void CancelPendingRequests();
	
	// 실제 서버 응답 수신 시 호출될 내부 함수 (테스트용으로 노출됨)
	// void HandleServerResponse(const FString& ResponseData);
	void HandleServerResponseStruct(const FString& ResponseData);
	
private:
	// HTTP 응답 콜백
	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	


	UPROPERTY()
	FOnAIResponseDataReceived PendingStructResponseDelegate;};
