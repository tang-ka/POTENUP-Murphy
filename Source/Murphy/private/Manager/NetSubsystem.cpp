// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/NetSubsystem.h"

#include "Murphy.h"

#include "Json.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"

void UNetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UNetSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	CancelPendingRequests();
}

void UNetSubsystem::SendToAI(const FAIRequestData& RequestData, const FString& WAVFilePath, FOnAIResponseDataReceived OnResponseDelegate)
{
	PendingStructResponseDelegate = OnResponseDelegate;
	
	TArray<uint8> RawAudioData;
	if (!FFileHelper::LoadFileToArray(RawAudioData, *WAVFilePath))
	{
		PRINTLOGE_JW(TEXT("WAV 파일 로드 실패: %s"), *WAVFilePath);
		HandleServerResponseStruct(TEXT(""));
		return;
	}
	
	// FAIRequestData 구조체를 JSON 문자열로 직렬화
	FString JsonString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(FAIRequestData::StaticStruct(), &RequestData, JsonString, 0, 0))
	{
		PRINTLOGE_JW(TEXT("RequestData JSON 직렬화 실패"));
		HandleServerResponseStruct(TEXT(""));
		return;
	}
	
	// multipart/form-data 페이로드 조립
	const FString Boundary = TEXT("----UnrealBoundary") + FString::FromInt(FMath::Rand());
	TArray<uint8> Payload;
	auto AppendStr = [&](const FString& Str)
	{
		FTCHARToUTF8 Conv(*Str);
		Payload.Append((uint8*)Conv.Get(), Conv.Length());
	};
	
	AppendStr(FString::Printf(TEXT("--%s\r\nContent-Disposition: form-data; name=\"turn\"\r\nContent-Type: application/json\r\n\r\n%s"), *Boundary, *JsonString));
	AppendStr(FString::Printf(TEXT("\r\n--%s\r\nContent-Disposition: form-data; name=\"audio\"; filename=\"user_voice.wav\"\r\nContent-Type: audio/wav\r\n\r\n"), *Boundary));
	Payload.Append(RawAudioData);
	AppendStr(FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary));
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	// Request->SetURL(TEXT("http://127.0.0.1:8001/api/chat"));
	Request->SetURL(TEXT("http://172.16.15.36:8000/api/game/ai/respond"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	Request->SetContent(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UNetSubsystem::OnHttpResponseReceived);
	Request->ProcessRequest();
}

void UNetSubsystem::CancelPendingRequests()
{
	if (PendingStructResponseDelegate.IsBound())
	{
		PendingStructResponseDelegate.Unbind();
		PRINTLOGW_JW(TEXT("보류 중인 Struct 요청이 취소됨(위임 해제됨)"));
	}
}



void UNetSubsystem::HandleServerResponseStruct(const FString& ResponseData)
{
	if (PendingStructResponseDelegate.IsBound())
	{
		FAIResponseData OutStruct;
		bool bSuccess = false;
		
		if (!ResponseData.IsEmpty())
		{
			bSuccess = FJsonObjectConverter::JsonObjectStringToUStruct<FAIResponseData>(ResponseData, &OutStruct, 0, 0);
			if (!bSuccess)
			{
				PRINTLOGE_JW(TEXT("JSON 파싱 실패: %s"), *ResponseData);
			}
		}
		
		PendingStructResponseDelegate.Execute(OutStruct);
		PendingStructResponseDelegate.Unbind();
	}
}

void UNetSubsystem::OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ResponseStr = TEXT("");
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		FString ErrorMsg = Response.IsValid() ? Response->GetContentAsString() : TEXT("No Response");
		PRINTLOGE_JW(TEXT("서버 응답 오류 - 코드: %d, 사유: %s"), Response.IsValid() ? Response->GetResponseCode() : -1, *ErrorMsg);
	}
	else
	{
		ResponseStr = Response->GetContentAsString();
	}
	
	// 정상 응답이면 페이로드를 전달 (에러 시 빈 문자열 전달)
	HandleServerResponseStruct(ResponseStr);
}