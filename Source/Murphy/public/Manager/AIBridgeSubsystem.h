
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h" 
#include "Interfaces/IHttpResponse.h"
#include "Data/AIDataTypes.h"
#include "AIBridgeSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAIResponseReceived, const FString&, ResponseData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAIResponseDataReceived, const FAIResponseData&, ResponseData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAIResultReceived, const FAIResultResponse&, ResultData);

UCLASS()
class MURPHY_API UAIBridgeSubsystem : public UGameInstanceSubsystem
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

	// 게임 완료 후 AI 서버에서 최종 점수판 결과를 조회합니다.
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void RequestAIResult(const FString& SessionId, FOnAIResultReceived OnResultDelegate);
	
	// 진행중인 요청(구독) 강제 취소
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void CancelPendingRequests();
	
	// 실제 서버 응답 수신 시 호출될 내부 함수 (테스트용으로 노출됨)
	// void HandleServerResponse(const FString& ResponseData);
	void HandleServerResponseStruct(const FString& ResponseData);
	
private:
	// HTTP 응답 콜백
	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// 최종 결과 조회 HTTP 응답 콜백
	void OnAIResultHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// 최종 결과 JSON을 구조체로 변환해 콜백을 실행합니다.
	void HandleAIResultResponseStruct(const FString& ResponseData);

	UPROPERTY()
	FOnAIResponseDataReceived PendingStructResponseDelegate;

	UPROPERTY()
	FOnAIResultReceived PendingResultResponseDelegate;
};
