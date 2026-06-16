
#pragma once

#include "CoreMinimal.h"
#include "STTDataTypes.generated.h"

// ================================================================
// [STT WebSocket] Client → Server 이벤트 구조체
// Contract: dev_c_realtime_stt.v1
// ================================================================

/**
 * @brief WebSocket 세션 시작 이벤트 (항상 첫 번째 메시지)
 */
USTRUCT(BlueprintType)
struct FSTT_SessionStart
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString contract_version = TEXT("dev_c_realtime_stt.v1");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString event_type = TEXT("session_start");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString session_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 turn_index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 sequence = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString chapter_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString scene_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString current_node_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString provider = TEXT("elevenlabs_relay");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString language_hint = TEXT("en");
};

/**
 * @brief 마이크 PCM16 청크 이벤트 (base64 인코딩 후 전송)
 * 마지막 실제 음성 청크에는 commit = true 설정
 */
USTRUCT(BlueprintType)
struct FSTT_AudioChunk
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString contract_version = TEXT("dev_c_realtime_stt.v1");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString event_type = TEXT("audio_chunk");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString session_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 turn_index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 sequence = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString provider = TEXT("elevenlabs_relay");

	/** base64 인코딩된 PCM16 mono 16kHz 데이터 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString audio_base64;

	/** true이면 마지막 청크. 별도 무음 sentinel 없이 실제 마지막 청크에 사용 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	bool commit = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 sample_rate_hz = 16000;
};

// ================================================================
// [STT WebSocket] Server → Client 이벤트 구조체
// ================================================================

/**
 * @brief 자막 텍스트 블록 (partial / final 공통 사용)
 */
USTRUCT(BlueprintType)
struct FSTT_Subtitle
{
	GENERATED_BODY()

	/** 현재까지 인식된 텍스트 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString text;

	/** true이면 최종 확정 자막 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	bool is_final = false;

	/** 자막 표시 방식: "replace" = 기존 자막을 교체 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString display_mode = TEXT("replace");
};

/**
 * @brief 서버 → Unreal partial 자막 이벤트
 * committed = false. partial마다 /respond를 호출하지 않는다.
 */
USTRUCT(BlueprintType)
struct FSTT_PartialTranscript
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString event_type; // "partial_transcript"

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString session_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 turn_index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FSTT_Subtitle subtitle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	bool committed = false;
};

/**
 * @brief 서버 → Unreal final 자막 이벤트
 * committed = true 일 때 /respond 엔드포인트 호출 트리거
 */
USTRUCT(BlueprintType)
struct FSTT_FinalTranscript
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString event_type; // "final_transcript"

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString session_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	int32 turn_index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FSTT_Subtitle subtitle;

	/** true이면 최종 발화 확정 → /respond 호출 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	bool committed = false;

	/** 다음에 호출할 엔드포인트 힌트 (예: "POST /api/game/ai/respond") */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString target_endpoint;
};

/**
 * @brief 에러 이벤트 (contract_error / provider_error / session_cancelled)
 */
USTRUCT(BlueprintType)
struct FSTT_ErrorEvent
{
	GENERATED_BODY()

	/** "contract_error" | "provider_error" | "session_cancelled" */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString event_type;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString message;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="STT|WebSocket")
	FString reason;
};
