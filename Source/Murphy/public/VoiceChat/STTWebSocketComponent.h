// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/STTDataTypes.h"
#include "STTWebSocketComponent.generated.h"

class IWebSocket;

// ================================================================
// 델리게이트 선언
// ================================================================

/** 자막 업데이트 (partial/final 공통). bIsFinal=true이면 최종 자막 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubtitleUpdated, const FString&, Text, bool, bIsFinal);

/** final_transcript + committed=true 수신 시 발동. FinalText를 /respond에 넣을 transcript로 사용 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinalTranscriptReady, const FString&, FinalText);

/** 에러 이벤트 수신 시 발동 (fallback WAV 방식으로 전환 트리거로 사용 가능) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSTTError, const FString&, ErrorType, const FString&, Message);

// ================================================================

/**
 * @brief Realtime STT WebSocket 클라이언트 컴포넌트
 *
 * 통신 흐름:
 *  1. Connect()           → WS 연결 + session_start 전송
 *  2. SendAudioChunk()    → base64 PCM16 chunk 전송 (OnAudioChunkReady 콜백에서 호출)
 *  3. partial_transcript  → OnSubtitleUpdated(Text, false) 발동
 *  4. final_transcript    → OnSubtitleUpdated(Text, true) + OnFinalTranscriptReady(Text) 발동
 *  5. Disconnect()        → WS 닫기
 *
 * 에러 처리:
 *  - contract_error / provider_error / session_cancelled → OnSTTError 발동
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MURPHY_API USTTWebSocketComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USTTWebSocketComponent();

	// ------------------------------------------------------------------
	// 공개 API
	// ------------------------------------------------------------------

	/**
	 * @brief WebSocket 연결 및 session_start 이벤트 전송
	 * @param SessionPayload  현재 턴의 세션 정보 (request_id, session_id 등)
	 */
	UFUNCTION(BlueprintCallable, Category="STT|WebSocket")
	void Connect(const FSTT_SessionStart& SessionPayload);

	/**
	 * @brief PCM16 mono 16kHz 바이트 배열을 base64로 인코딩해 audio_chunk 이벤트 전송
	 * @param PCM16Data  16kHz mono PCM16 raw bytes
	 * @param bCommit    true이면 마지막 청크 (서버에 commit 신호)
	 */
	UFUNCTION(BlueprintCallable, Category="STT|WebSocket")
	void SendAudioChunk(const TArray<uint8>& PCM16Data, bool bCommit);

	/**
	 * @brief WebSocket 연결 해제 및 상태 초기화
	 */
	UFUNCTION(BlueprintCallable, Category="STT|WebSocket")
	void Disconnect();

	/** @return 현재 WebSocket이 연결된 상태인지 */
	UFUNCTION(BlueprintCallable, Category="STT|WebSocket")
	bool IsConnected() const;

	// ------------------------------------------------------------------
	// 델리게이트 (외부에서 바인딩)
	// ------------------------------------------------------------------

	/** 자막 업데이트 이벤트 (partial=false / final=true) */
	UPROPERTY(BlueprintAssignable, Category="STT|WebSocket")
	FOnSubtitleUpdated OnSubtitleUpdated;

	/** final_transcript + committed=true 수신 시 발동 */
	UPROPERTY(BlueprintAssignable, Category="STT|WebSocket")
	FOnFinalTranscriptReady OnFinalTranscriptReady;

	/** WS 에러 또는 세션 취소 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="STT|WebSocket")
	FOnSTTError OnSTTError;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ------------------------------------------------------------------
	// WebSocket 이벤트 핸들러
	// ------------------------------------------------------------------
	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnMessage(const FString& MessageStr);

	// ------------------------------------------------------------------
	// 내부 유틸
	// ------------------------------------------------------------------

	/** session_start JSON을 조립해서 전송 */
	void SendSessionStart();

	/** 서버 메시지 파싱 후 event_type에 따라 델리게이트 발동 */
	void DispatchServerEvent(const FString& EventType, const TSharedPtr<FJsonObject>& JsonObj);

	// ------------------------------------------------------------------
	// 상태
	// ------------------------------------------------------------------
	TSharedPtr<IWebSocket> WebSocket;

	/** 현재 연결에 사용할 세션 페이로드 (Connect 호출 시 저장) */
	FSTT_SessionStart PendingSessionPayload;

	/** 연결 후 전송하는 audio_chunk sequence 카운터 (0은 session_start에서 사용) */
	int32 ChunkSequence = 1;

	/** 현재 WS가 연결된 상태인지 */
	bool bIsConnected = false;
};
