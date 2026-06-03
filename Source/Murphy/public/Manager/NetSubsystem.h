
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"  // 🟢 추가
#include "Interfaces/IHttpResponse.h" // 🟢 추가
#include "NetSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAIResponseReceived, const FString&, ResponseData);

UCLASS()
class MURPHY_API UNetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// AI 서버로 메세지를 전송하고 응답 받을 델리게이트를 등록
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void SendMessageToAI(const FString& Msg, FOnAIResponseReceived OnResponseDelegate);
	
	// AI 서버로 음성파일 원본 전송 (Multipart)
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void SendVoiceFileToAI(const FString& WAVFilePath, FOnAIResponseReceived OnResponseDelegate);
	
	//X AI 서버로 Base64로 인코딩 하여 전송 (json 방식)
	// UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	// void SendVoiceAsJsonBase64ToAI(const FString& WAVFilePath, FOnAIResponseReceived OnResponseDelegate);
	
	// 진행중인 요청(구독) 강제 취소
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void CancelPendingRequests();
	
	// 실제 서버 응답 수신 시 호출될 내부 함수 (테스트용으로 노출됨)
	void HandleServerResponse(const FString& ResponseData);
	
private:
	// HTTP 응답 콜백
	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	// 현재 대기중인 응답 델리게이트
	UPROPERTY()
	FOnAIResponseReceived PendingResponseDelegate;
};
