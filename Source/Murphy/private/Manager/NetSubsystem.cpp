// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/NetSubsystem.h"

#include "Murphy.h"

#include "Json.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "Settings/MurphyNetSettings.h"

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
	
	// 경로가 비어있다면 1분을 초과한 상황 -> 44byte짜리 빈 wav 생성 (AI 파싱 에러 방지)
	if (WAVFilePath.IsEmpty())
	{
		const uint8 DummyWavHeader[44] = {
			'R', 'I', 'F', 'F', 
			0x24, 0x7D, 0x00, 0x00,  // File size - 8 = 32036 바이트
			'W', 'A', 'V', 'E', 
			'f', 'm', 't', ' ', 
			16, 0, 0, 0,             // fmt chunk size
			1, 0,                    // AudioFormat (PCM)
			1, 0,                    // NumChannels (1)
			0x80, 0x3E, 0x00, 0x00,  // SampleRate (16000 Hz)
			0x00, 0x7D, 0x00, 0x00,  // ByteRate (32000)
			2, 0,                    // BlockAlign
			16, 0,                   // BitsPerSample (16)
			'd', 'a', 't', 'a', 
			0x00, 0x7D, 0x00, 0x00   // Data size (32000 바이트)
		};
		RawAudioData.Append(DummyWavHeader, 44);
		
		// [핵심] 헤더 뒤에 32000바이트의 0(무음 파형 데이터)을 꽉 채워줍니다!
		RawAudioData.AddZeroed(32000); 
		
		PRINTLOGW_JW(TEXT("빈 파일 요청: 파이썬 에러 방지용 1초 무음 WAV(32044바이트)를 생성해 전송합니다."));
	}
	else if (!FFileHelper::LoadFileToArray(RawAudioData, *WAVFilePath))
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
	
	const UMurphyNetSettings* NetSettings = GetDefault<UMurphyNetSettings>();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(NetSettings->GetHttpBase() + TEXT("/api/game/ai/respond"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	Request->SetContent(Payload);
	Request->SetTimeout(300.f); // AI 응답에 시간이 걸릴 수 있으므로 타임아웃을 120초로 증가
	Request->OnProcessRequestComplete().BindUObject(this, &UNetSubsystem::OnHttpResponseReceived);
	Request->ProcessRequest();
}

void UNetSubsystem::SendToAIWithTranscript(const FAIRequestData& RequestData, const FString& Transcript, FOnAIResponseDataReceived OnResponseDelegate)
{
	PendingStructResponseDelegate = OnResponseDelegate;

	// -----------------------------------------------------------
	// 1) FAIRequestData → JSON Object 변환
	// -----------------------------------------------------------
	TSharedPtr<FJsonObject> TurnJsonObj = MakeShared<FJsonObject>();
	if (!FJsonObjectConverter::UStructToJsonObject(FAIRequestData::StaticStruct(), &RequestData, TurnJsonObj.ToSharedRef(), 0, 0))
	{
		PRINTLOGE_JW(TEXT("[NetSub|STT] RequestData JSON 직렬화 실패"));
		HandleServerResponseStruct(TEXT(""));
		return;
	}

	// -----------------------------------------------------------
	// 2) 최상위 래퍼 조립: { "turn": <TurnJson>, "audio": { "transcript": "<Transcript>" } }
	//    Codex Prompt 참고: audio.transcript 필드로 STT 결과를 전달
	// -----------------------------------------------------------
	const TSharedRef<FJsonObject> AudioObj = MakeShared<FJsonObject>();
	AudioObj->SetStringField(TEXT("transcript"), Transcript);

	const TSharedRef<FJsonObject> RootObj = MakeShared<FJsonObject>();
	RootObj->SetObjectField(TEXT("turn"),  TurnJsonObj);
	RootObj->SetObjectField(TEXT("audio"), AudioObj);

	FString JsonBody;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(RootObj, Writer);

	// -----------------------------------------------------------
	// 3) HTTP POST 요청 (Content-Type: application/json)
	// -----------------------------------------------------------
	const UMurphyNetSettings* NetSettings = GetDefault<UMurphyNetSettings>();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(NetSettings->GetHttpBase() + TEXT("/api/game/ai/respond"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonBody);
	Request->SetTimeout(300.f);
	Request->OnProcessRequestComplete().BindUObject(this, &UNetSubsystem::OnHttpResponseReceived);
	Request->ProcessRequest();

	PRINTLOGW_JW(TEXT("[NetSub|STT] SendToAIWithTranscript 전송: transcript=\"%s\""), *Transcript);
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