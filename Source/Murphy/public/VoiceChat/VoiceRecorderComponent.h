// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceRecorderComponent.generated.h"

class UAudioCapture;
// WAV 저장 완료 시 알림 (나중에 AI 서버 전송 단계에서 연결)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingFinished, const FString&, SavedFilePath);
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
	
	UFUNCTION(BlueprintCallable, Category = "VoiceRecorder")
	bool IsRecording() const { return bIsRecording; }
	
#pragma region getters
	const TArray<float>& GetRecordedSamples() const { return RecordedSamples; }
	int32 GetSampleRate() const { return CapturedSampleRate; }
	int32 GetNumChannels() const { return CapturedNumChannels; }
#pragma endregion
	
	UPROPERTY(BlueprintAssignable, Category = "VoiceRecorder")
	FOnRecordingFinished OnRecordingFinished;
	
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
};
