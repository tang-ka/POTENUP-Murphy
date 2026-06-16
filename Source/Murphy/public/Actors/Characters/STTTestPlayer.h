// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Characters/MurphyPlayer.h"
#include "Data/AIDataTypes.h"
#include "STTTestPlayer.generated.h"

struct FInputActionValue;
class UInputAction;
class USTTWebSocketComponent;

/**
 * @brief Realtime STT WebSocket 테스트 전용 플레이어 캐릭터
 *
 * AMurphyPlayer를 상속하므로:
 *  - 이동/카메라/가방/폰/아이템 상호작용은 부모 구현 그대로 사용
 *  - VoiceRecorderComponent는 부모에서 이미 생성된 것을 공유
 *  - IA_Record 바인딩(WAV 방식)은 Super 그대로 유지
 *  - IA_STTStart / IA_STTStop 두 개의 새 InputAction으로 STT 흐름 추가
 *
 * 에디터에서 할 일:
 *  1. IA_STTStart DataAsset 생성 → STTTestPlayer BP의 IA_STTStart에 할당
 *  2. IA_STTStop  DataAsset 생성 → STTTestPlayer BP의 IA_STTStop에 할당
 *  3. IMC에 두 IA를 원하는 키에 바인딩
 */
UCLASS()
class MURPHY_API ASTTTestPlayer : public AMurphyPlayer
{
	GENERATED_BODY()

public:
	ASTTTestPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ------------------------------------------------------------------
	// [신규] STT WebSocket 전용 InputAction 핸들러
	// ------------------------------------------------------------------

	/** IA_STTStart Started → ChunkingMode ON + WS 연결 + 녹음 시작 */
	void OnSTTStartPressed(const FInputActionValue& Value);

	/** IA_STTStop Started → 녹음 정지 + 마지막 청크 commit=true flush + WS final 대기 */
	void OnSTTStopPressed(const FInputActionValue& Value);

	// ------------------------------------------------------------------
	// STT 델리게이트 콜백 (STTWebSocketComponent에서 발동)
	// ------------------------------------------------------------------

	/** VoiceRecorderComponent.OnAudioChunkReady → STTWebSocket으로 전달 */
	UFUNCTION()
	void OnAudioChunkReady(const TArray<uint8>& PCM16Chunk, bool bIsLastChunk);

	/** partial/final 자막 이벤트 → 로그 출력 (추후 자막 UI 연결) */
	UFUNCTION()
	void OnSubtitleUpdated(const FString& Text, bool bIsFinal);

	/** final_transcript + committed=true → SendToAIWithTranscript 호출 */
	UFUNCTION()
	void OnFinalTranscriptReady(const FString& FinalText);

	/** WS 에러 이벤트 → 로그 출력 + 상태 복구 */
	UFUNCTION()
	void OnSTTError(const FString& ErrorType, const FString& Message);

	/** SendToAIWithTranscript 응답 콜백 (DYNAMIC_DELEGATE는 UFUNCTION 필수) */
	UFUNCTION()
	void OnAIRespondReceived(const FAIResponseData& ResponseData);

public:
	// ------------------------------------------------------------------
	// [신규] STT 전용 InputAction UPROPERTY
	// 에디터에서 BP에 DataAsset을 할당하고, IMC에 키를 바인딩한다.
	// ------------------------------------------------------------------

	/** STT 세션 시작 InputAction (에디터에서 생성 후 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input|STT")
	TObjectPtr<UInputAction> IA_STTStart;

	/** STT 세션 종료 InputAction (에디터에서 생성 후 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input|STT")
	TObjectPtr<UInputAction> IA_STTStop;

	/** STT WebSocket 클라이언트 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|STT")
	TObjectPtr<USTTWebSocketComponent> STTWebSocketComp;

private:
	/** 현재 테스트용 TurnData 기본값 채우기 */
	void PrepareTestTurnData();

	/**
	 * STT WebSocket 세션에서 사용할 Turn JSON.
	 * OnSTTStartPressed 시 PrepareTestTurnData()로 채워지고,
	 * OnFinalTranscriptReady에서 /respond에 전달된다.
	 */
	FAIRequestData CachedTurnData;
};
