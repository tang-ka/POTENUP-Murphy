// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/NetSubsystem.h"

#include "Murphy.h"

#include "Json.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"

void UNetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UNetSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	CancelPendingRequests();
}

void UNetSubsystem::SendMessageToAI(const FString& Msg, FOnAIResponseReceived OnResponseDelegate)
{
	// 델리게이트 구독 -> 기존 요청이 있다면 덮어쓰고 새 요청으로 업데이트
	PendingResponseDelegate = OnResponseDelegate;
	
	// todo: 실제 서버 연동 (HTTP Request / WebSocket Send ..) 로직 구현 필요
	PRINTLOGW_JW(TEXT("AI에게 보낸 메시지 : %s"), *Msg);
}

void UNetSubsystem::SendVoiceFileToAI(const FString& WAVFilePath, FOnAIResponseReceived OnResponseDelegate)
{
	PendingResponseDelegate = OnResponseDelegate;
	
	TArray<uint8> RawAudioData;
	if (!FFileHelper::LoadFileToArray(RawAudioData, *WAVFilePath))
	{
		PRINTLOGE_JW(TEXT("WAV 파일 로드 실패: %s"), *WAVFilePath);
		return;
	}
	
	// todo: 하드코딩 부분 데이터 들어오면 변경
	
	// JSON 메타데이터 구성
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	JsonObj->SetStringField(TEXT("session_id"), TEXT("sess_12345"));
	JsonObj->SetStringField(TEXT("user_id"), TEXT("player_01"));
	JsonObj->SetStringField(TEXT("current_scene"), TEXT("immigration"));
	JsonObj->SetStringField(TEXT("npc_current_emotion"), TEXT("Neutral"));
	JsonObj->SetBoolField(TEXT("is_timeout"), false);
	JsonObj->SetNumberField(TEXT("response_time_sec"), 10.0f);
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	
	// multipart/form-data 페이로드 조립
	const FString Boundary = TEXT("----UnrealBoundary") + FString::FromInt(FMath::Rand());
	TArray<uint8> Payload;
	auto AppendStr = [&](const FString& Str)
	{
		FTCHARToUTF8 Conv(*Str);
		Payload.Append((uint8*)Conv.Get(), Conv.Length());
	};
	
	AppendStr(FString::Printf(TEXT("\r\n--%s\r\nContent-Disposition: form-data; name=\"metadata\"\r\nContent-Type: application/json\r\n\r\n%s"), *Boundary, *JsonString));
	AppendStr(FString::Printf(TEXT("\r\n--%s\r\nContent-Disposition: form-data; name=\"audio_file\"; filename=\"user_voice.wav\"\r\nContent-Type: audio/wav\r\n\r\n"), *Boundary));
	Payload.Append(RawAudioData);
	AppendStr(FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary));
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://127.0.0.1:8001/api/chat")); 
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	Request->SetContent(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UNetSubsystem::OnHttpResponseReceived);
	Request->ProcessRequest();
}

/*! X  AI 서버로 Base64로 인코딩 하여 전송 (json 방식)
void UNetSubsystem::SendVoiceAsJsonBase64ToAI(const FString& WAVFilePath, FOnAIResponseReceived OnResponseDelegate)
{
	PendingResponseDelegate = OnResponseDelegate;
	
	TArray<uint8> RawAudioData;
	if (!FFileHelper::LoadFileToArray(RawAudioData, *WAVFilePath))
	{
		PRINTLOGE_JW(TEXT("WAV 파일 로드 실패: %s"), *WAVFilePath);
		return;
	}
	
	// JSON 메타데이터 구성
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	JsonObj->SetStringField(TEXT("session_id"), TEXT("sess_12345"));
	JsonObj->SetStringField(TEXT("user_id"), TEXT("player_01"));
	JsonObj->SetStringField(TEXT("user_tier"), TEXT("Bronze"));
	JsonObj->SetStringField(TEXT("current_scene"), TEXT("immigration"));
	JsonObj->SetStringField(TEXT("npc_current_emotion"), TEXT("Neutral"));
	JsonObj->SetBoolField(TEXT("is_timeout"), false);
	JsonObj->SetNumberField(TEXT("response_time_sec"), 10.0f);
	
	// WAV 바이너리를 Base64 문자열로 인코딩하여 JSON에 추가
	FString AudioBase64 = FBase64::Encode(RawAudioData);
	JsonObj->SetStringField(TEXT("user_audio_base64"), AudioBase64);
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://127.0.0.1:8000/api/chat"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &UNetSubsystem::OnHttpResponseReceived);
	Request->ProcessRequest();
}
*/

void UNetSubsystem::CancelPendingRequests()
{
	// 응답을 기다리지 않도록 델리게이트 구독 취소
	if (PendingResponseDelegate.IsBound())
	{
		PendingResponseDelegate.Unbind();
		PRINTLOGW_JW(TEXT("보류 중인 요청이 취소됨(위임 해제됨)"));
	}
}

void UNetSubsystem::HandleServerResponse(const FString& ResponseData)
{
	// 1. 수신 시 델리게이트가 등록되어 있으면 실행
	if (PendingResponseDelegate.IsBound())
	{
		PendingResponseDelegate.Execute(ResponseData);
		
		// 2. 실행 완료 후 구독 취소 (1회성 구독이 목적이므로 Unbind)
		PendingResponseDelegate.Unbind();
	}
}

void UNetSubsystem::OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		PRINTLOGE_JW(TEXT("서버 응답 오류 - 코드: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
		// 에러가 났더라도 콜백을 빈 문자열로 넘겨서 호출측이 대화상태를 초기화할 수 있게 함
		HandleServerResponse(TEXT(""));
		return;
	}
	
	// 정상 응답이면 페이로드를 전달
	HandleServerResponse(Response->GetContentAsString());
}
