
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
	
	// 진행중인 요청(구독) 강제 취소
	UFUNCTION(BlueprintCallable, Category="Murphy|Net")
	void CancelPendingRequests();
	
	// 실제 서버 응답 수신 시 호출될 내부 함수 (테스트용으로 노출됨)
	void HandleServerResponse(const FString& ResponseData);
	void HandleServerResponseStruct(const FString& ResponseData);
	
private:
	// HTTP 응답 콜백
	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	


	UPROPERTY()
	FOnAIResponseDataReceived PendingStructResponseDelegate;};
