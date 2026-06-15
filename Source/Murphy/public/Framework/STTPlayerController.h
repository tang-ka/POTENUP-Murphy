#pragma once

#include "CoreMinimal.h"
#include "Framework/MurphyPlayerController.h"
#include "Data/AIDataTypes.h"
#include "STTPlayerController.generated.h"

class USTTWebSocketComponent;
class UInputAction;

/**
 * STT WebSocket 전용 PlayerController
 * 기존 AMurphyPlayerController를 상속받아 레벨스트리밍 및 타겟 NPC 연동(AgentNPCBase) 로직을
 * 그대로 재사용하면서 실시간 STT 통신 로직을 처리합니다.
 */
UCLASS()
class MURPHY_API ASTTPlayerController : public AMurphyPlayerController
{
	GENERATED_BODY()

public:
	ASTTPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// === STT Components & Input ===
	
	// 실시간 음성 데이터를 전송하고 자막/최종텍스트를 수신받을 WebSocket 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "STT")
	TObjectPtr<USTTWebSocketComponent> STTWebSocketComp;

	// 마이크 녹음을 시작하는 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "STT|Input")
	TObjectPtr<UInputAction> IA_STTStart;

	// 마이크 녹음을 중단하고 전송을 확정하는 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "STT|Input")
	TObjectPtr<UInputAction> IA_STTStop;

	// Input Handlers
	void OnSTTStartPressed(const struct FInputActionValue& Value);
	void OnSTTStopPressed(const struct FInputActionValue& Value);

	// === WebSocket Callbacks ===
	UFUNCTION()
	void OnSubtitleUpdated(const FString& Text, bool bIsFinal);

	UFUNCTION()
	void OnFinalTranscriptReady(const FString& FinalText);

	UFUNCTION()
	void OnSTTError(const FString& ErrorType, const FString& Message);

	// === AI Request/Response ===
	UFUNCTION()
	void OnAIRespondReceived(const FAIResponseData& ResponseData);

	// 오디오 스레드에서 생성된 Chunk를 WebSocket으로 전송하기 위한 콜백
	UFUNCTION()
	void OnAudioChunkReady(const TArray<uint8>& PCM16Chunk, bool bIsLastChunk);

	// (임시) 데이터 매니저 연동 전까지 사용할 테스트용 턴 데이터 세팅
	void PrepareTestTurnData();

private:
	FAIRequestData CachedTurnData;
};
