// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceRecorderComponent.generated.h"

class UAudioCapture;

// WAV 저장 완료 시 알림 (나중에 AI 서버 전송 단계에서 연결)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingFinished, const FString&, SavedFilePath);

/**
 * Chunking Mode 전용 델리게이트
 * PCM16 mono 16kHz 청크가 준비될 때마다 발동.
 * bIsLastChunk = true이면 이 청크가 해당 녹음의 마지막 청크.
 * STTWebSocketComponent의 SendAudioChunk()에 연결해서 사용.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioChunkReady, const TArray<uint8>&, PCM16Chunk, bool, bIsLastChunk);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MURPHY_API UVoiceRecorderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceRecorderComponent();

	UFUNCTION(BlueprintCallable, Category = "VoiceRecorder")
	void PlayRecordedSamples();
	
	UFUNCTION(BlueprintCallable, Category = "VoiceRecorder")
	void StartRecording();
	
	UFUNCTION(BlueprintCallable, Category = "VoidRecorder")
	void StopRecording(const FString& FileName, bool bSaveToWav = true);

	/**
	 * @brief Chunking Mode 설정
	 * @param bEnable      true이면 청크 스트리밍 모드. false(기본)이면 기존 WAV 방식.
	 * @param ChunkSizeMs  청크 1개 길이 (밀리초). 기본 100ms = 16kHz 기준 1600 samples.
	 * @note  StartRecording() 호출 전에 설정해야 합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "VoiceRecorder")
	void SetChunkingMode(bool bEnable, int32 ChunkSizeMs = 100);
	
	UFUNCTION(BlueprintCallable, Category = "VoiceRecorder")
	bool IsRecording() const { return bIsRecording; }
	
#pragma region getters
	const TArray<float>& GetRecordedSamples() const { return RecordedSamples; }
	int32 GetSampleRate() const { return CapturedSampleRate; }
	int32 GetNumChannels() const { return CapturedNumChannels; }
#pragma endregion
	
	UPROPERTY(BlueprintAssignable, Category = "VoiceRecorder")
	FOnRecordingFinished OnRecordingFinished;

	/** Chunking Mode 전용: PCM16 청크 준비 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "VoiceRecorder")
	FOnAudioChunkReady OnAudioChunkReady;
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	void SaveToWav(const FString& FileName);
	
private:
	UPROPERTY()
	TObjectPtr<UAudioCapture> AudioCapture;
	
	TArray<float> RecordedSamples;
	bool bIsRecording = false;

	int32 CapturedSampleRate = 48000;
	int32 CapturedNumChannels = 1;

	// ------------------------------------------------------------------
	// Chunking Mode 전용 상태
	// ------------------------------------------------------------------

	/** true이면 Chunking Mode. false(기본)이면 기존 WAV 방식 유지 */
	bool bChunkingMode = false;

	/** 청크 1개에 담을 16kHz 기준 샘플 수 (기본 100ms = 1600) */
	int32 ChunkSizeSamples16k = 1600;

	/** Chunking Mode 중 16kHz PCM16 버퍼 누적용 */
	TArray<int16> ChunkBuffer16k;

	/** float 샘플을 16kHz mono PCM16으로 변환·누적하고 청크 단위로 브로드캐스트 */
	void ProcessAndBroadcastChunk(const float* InAudio, int32 NumSamples, bool bIsLast = false);
};
