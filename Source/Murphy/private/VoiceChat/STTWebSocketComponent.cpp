// Fill out your copyright notice in the Description page of Project Settings.

#include "VoiceChat/STTWebSocketComponent.h"

#include "Murphy.h"
#include "Misc/Base64.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Settings/MurphyNetSettings.h"

// WebSocket 엔드포인트 경로. 호스트는 UMurphyNetSettings에서 가져온다.
static const FString STT_WS_PATH = TEXT("/api/game/ai/stt/stream");
static const FString STT_WS_PROTOCOL = TEXT("");

// ----------------------------------------------------------------

USTTWebSocketComponent::USTTWebSocketComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ----------------------------------------------------------------
// 공개 API
// ----------------------------------------------------------------

void USTTWebSocketComponent::Connect(const FSTT_SessionStart& SessionPayload)
{
	if (bIsConnected)
	{
		PRINTLOGW_JW(TEXT("[STTWebSocket] 이미 연결 중입니다. 먼저 Disconnect()를 호출하세요."));
		return;
	}

	PendingSessionPayload = SessionPayload;
	ChunkSequence = 1; // 0은 session_start

	// WebSocket 모듈 확인
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
	{
		FModuleManager::Get().LoadModule(TEXT("WebSockets"));
	}

	const FString WsUrl = GetDefault<UMurphyNetSettings>()->GetWebSocketBase() + STT_WS_PATH;
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WsUrl, STT_WS_PROTOCOL);
	if (!WebSocket.IsValid())
	{
		PRINTLOGE_JW(TEXT("[STTWebSocket] WebSocket 생성 실패: %s"), *WsUrl);
		OnSTTError.Broadcast(TEXT("connection_error"), TEXT("WebSocket 생성 실패"));
		return;
	}

	// 이벤트 바인딩
	WebSocket->OnConnected().AddUObject(this, &USTTWebSocketComponent::OnConnected);
	WebSocket->OnConnectionError().AddUObject(this, &USTTWebSocketComponent::OnConnectionError);
	WebSocket->OnClosed().AddUObject(this, &USTTWebSocketComponent::OnClosed);
	WebSocket->OnMessage().AddUObject(this, &USTTWebSocketComponent::OnMessage);

	WebSocket->Connect();
	PRINTLOGW_JW(TEXT("[STTWebSocket] 연결 시도: %s"), *WsUrl);
}

void USTTWebSocketComponent::SendAudioChunk(const TArray<uint8>& PCM16Data, bool bCommit)
{
	if (!bIsConnected || !WebSocket.IsValid())
	{
		PRINTLOGW_JW(TEXT("[STTWebSocket] 연결되지 않은 상태에서 청크 전송 시도 무시"));
		return;
	}

	// base64 인코딩
	const FString Base64Audio = FBase64::Encode(PCM16Data.GetData(), PCM16Data.Num());

	// JSON 조립
	const TSharedRef<FJsonObject> JsonObj = MakeShared<FJsonObject>();
	JsonObj->SetStringField(TEXT("contract_version"), TEXT("dev_c_realtime_stt.v1"));
	JsonObj->SetStringField(TEXT("event_type"),       TEXT("audio_chunk"));
	JsonObj->SetStringField(TEXT("request_id"),       PendingSessionPayload.request_id);
	JsonObj->SetStringField(TEXT("session_id"),       PendingSessionPayload.session_id);
	JsonObj->SetNumberField(TEXT("turn_index"),       PendingSessionPayload.turn_index);
	JsonObj->SetNumberField(TEXT("sequence"),         ChunkSequence++);
	JsonObj->SetStringField(TEXT("provider"),         TEXT("elevenlabs_relay"));
	JsonObj->SetStringField(TEXT("audio_base64"),     Base64Audio);
	JsonObj->SetBoolField  (TEXT("commit"),           bCommit);
	JsonObj->SetNumberField(TEXT("sample_rate_hz"),   16000);

	FString JsonStr;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
	FJsonSerializer::Serialize(JsonObj, Writer);

	WebSocket->Send(JsonStr);

	if (bCommit)
	{
		PRINTLOGW_JW(TEXT("[STTWebSocket] 마지막 청크 전송 (commit=true, seq=%d)"), ChunkSequence - 1);
	}
}

void USTTWebSocketComponent::Disconnect()
{
	if (WebSocket.IsValid())
	{
		WebSocket->Close();
	}
	bIsConnected = false;
	ChunkSequence = 1;
	PRINTLOGW_JW(TEXT("[STTWebSocket] 연결 종료 요청"));
}

bool USTTWebSocketComponent::IsConnected() const
{
	return bIsConnected;
}

// ----------------------------------------------------------------
// 컴포넌트 생명주기
// ----------------------------------------------------------------

void USTTWebSocketComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Disconnect();
	Super::EndPlay(EndPlayReason);
}

// ----------------------------------------------------------------
// WebSocket 이벤트 핸들러
// ----------------------------------------------------------------

void USTTWebSocketComponent::OnConnected()
{
	bIsConnected = true;
	PRINTLOGW_JW(TEXT("[STTWebSocket] 연결 성공. session_start 전송"));
	SendSessionStart();
}

void USTTWebSocketComponent::OnConnectionError(const FString& Error)
{
	bIsConnected = false;
	PRINTLOGE_JW(TEXT("[STTWebSocket] 연결 오류: %s"), *Error);
	OnSTTError.Broadcast(TEXT("connection_error"), Error);
}

void USTTWebSocketComponent::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	bIsConnected = false;
	PRINTLOGW_JW(TEXT("[STTWebSocket] 연결 닫힘 - Code:%d, Reason:%s, Clean:%s"),
		StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
}

void USTTWebSocketComponent::OnMessage(const FString& MessageStr)
{
	// JSON 파싱
	TSharedPtr<FJsonObject> JsonObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		PRINTLOGE_JW(TEXT("[STTWebSocket] 메시지 JSON 파싱 실패: %s"), *MessageStr);
		return;
	}

	FString EventType;
	if (!JsonObj->TryGetStringField(TEXT("event_type"), EventType))
	{
		PRINTLOGW_JW(TEXT("[STTWebSocket] event_type 필드 없음: %s"), *MessageStr);
		return;
	}

	DispatchServerEvent(EventType, JsonObj);
}

// ----------------------------------------------------------------
// 내부 유틸
// ----------------------------------------------------------------

void USTTWebSocketComponent::SendSessionStart()
{
	if (!WebSocket.IsValid() || !bIsConnected)
	{
		return;
	}

	// session_start JSON 수동 조립 (sequence=0 고정)
	const TSharedRef<FJsonObject> JsonObj = MakeShared<FJsonObject>();
	JsonObj->SetStringField(TEXT("contract_version"), TEXT("dev_c_realtime_stt.v1"));
	JsonObj->SetStringField(TEXT("event_type"),       TEXT("session_start"));
	JsonObj->SetStringField(TEXT("request_id"),       PendingSessionPayload.request_id);
	JsonObj->SetStringField(TEXT("session_id"),       PendingSessionPayload.session_id);
	JsonObj->SetNumberField(TEXT("turn_index"),        PendingSessionPayload.turn_index);
	JsonObj->SetNumberField(TEXT("sequence"),          0);
	JsonObj->SetStringField(TEXT("chapter_id"),       PendingSessionPayload.chapter_id);
	JsonObj->SetStringField(TEXT("scene_id"),         PendingSessionPayload.scene_id);
	JsonObj->SetStringField(TEXT("current_node_id"),  PendingSessionPayload.current_node_id);
	JsonObj->SetStringField(TEXT("provider"),         TEXT("elevenlabs_relay"));
	JsonObj->SetStringField(TEXT("language_hint"),    TEXT("en"));
	JsonObj->SetNumberField(TEXT("sample_rate_hz"),   16000);
	JsonObj->SetNumberField(TEXT("channels"),         1);

	FString JsonStr;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
	FJsonSerializer::Serialize(JsonObj, Writer);

	WebSocket->Send(JsonStr);
	PRINTLOGW_JW(TEXT("[STTWebSocket] session_start 전송: req_id=%s"), *PendingSessionPayload.request_id);
}

void USTTWebSocketComponent::DispatchServerEvent(const FString& EventType, const TSharedPtr<FJsonObject>& JsonObj)
{
	if (EventType == TEXT("partial_transcript"))
	{
		// subtitle.text 추출
		FString SubtitleText;
		const TSharedPtr<FJsonObject>* SubtitleObj = nullptr;
		if (JsonObj->TryGetObjectField(TEXT("subtitle"), SubtitleObj) && SubtitleObj)
		{
			(*SubtitleObj)->TryGetStringField(TEXT("text"), SubtitleText);
		}
		PRINTLOGW_JW(TEXT("[STTWebSocket] partial_transcript: \"%s\""), *SubtitleText);
		OnSubtitleUpdated.Broadcast(SubtitleText, false);
	}
	else if (EventType == TEXT("final_transcript"))
	{
		FString SubtitleText;
		bool bCommitted = false;
		const TSharedPtr<FJsonObject>* SubtitleObj = nullptr;

		if (JsonObj->TryGetObjectField(TEXT("subtitle"), SubtitleObj) && SubtitleObj)
		{
			(*SubtitleObj)->TryGetStringField(TEXT("text"), SubtitleText);
		}
		JsonObj->TryGetBoolField(TEXT("committed"), bCommitted);

		PRINTLOGW_JW(TEXT("[STTWebSocket] final_transcript: \"%s\" (committed=%s)"),
			*SubtitleText, bCommitted ? TEXT("true") : TEXT("false"));

		OnSubtitleUpdated.Broadcast(SubtitleText, true);

		if (bCommitted)
		{
			// committed=true → /respond 엔드포인트 호출 트리거
			OnFinalTranscriptReady.Broadcast(SubtitleText);
		}
	}
	else if (EventType == TEXT("contract_error")
		  || EventType == TEXT("provider_error")
		  || EventType == TEXT("session_cancelled"))
	{
		FString ErrorMessage;
		JsonObj->TryGetStringField(TEXT("message"), ErrorMessage);
		PRINTLOGE_JW(TEXT("[STTWebSocket] 에러 이벤트 [%s]: %s"), *EventType, *ErrorMessage);
		OnSTTError.Broadcast(EventType, ErrorMessage);
	}
	else
	{
		PRINTLOGW_JW(TEXT("[STTWebSocket] 알 수 없는 event_type: %s"), *EventType);
	}
}
