// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceChat/VoiceRecorderComponent.h"

#include "AudioCapture.h"
#include "Murphy.h"
#include "SampleBuffer.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SampleBufferIO.h"
#include "Sound/SoundWaveProcedural.h"

// Sets default values for this component's properties
UVoiceRecorderComponent::UVoiceRecorderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVoiceRecorderComponent::PlayRecordedSamples()
{
	if (RecordedSamples.Num() == 0)
	{
		PRINTLOG_SH(TEXT("No samples to play."));
		return;
	}

	TArray<uint8> PCMData;
	PCMData.SetNumUninitialized(RecordedSamples.Num() * sizeof(int16));
	int16* PCM16 = reinterpret_cast<int16*>(PCMData.GetData());

	for (int32 i = 0; i < RecordedSamples.Num(); ++i)
	{
		float Clamped = FMath::Clamp(RecordedSamples[i], -1.0f, 1.0f);
		PCM16[i] = static_cast<int16>(Clamped * 32767.0f);
	}

	USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>();
	SoundWave->SetSampleRate(CapturedSampleRate);
	SoundWave->NumChannels = CapturedNumChannels;
	SoundWave->Duration = static_cast<float>(RecordedSamples.Num()) / (CapturedSampleRate * CapturedNumChannels);
	SoundWave->SoundGroup = SOUNDGROUP_Default;
	SoundWave->bLooping = false;

	SoundWave->QueueAudio(PCMData.GetData(), PCMData.Num());

	UGameplayStatics::PlaySound2D(GetWorld(), SoundWave);

	PRINTLOG_SH(TEXT("Playing %d samples (SR=%d, Ch=%d)"),
		RecordedSamples.Num(), CapturedSampleRate, CapturedNumChannels);
}

void UVoiceRecorderComponent::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	RecordedSamples.Reset();

	AudioCapture = UAudioCaptureFunctionLibrary::CreateAudioCapture();
	if (!AudioCapture)
	{
		PRINTLOG_SH(TEXT("Failed to create UAudioCapture"));
		return;
	}

	AudioCapture->AddGeneratorDelegate(
		[this](const float* InAudio, int32 NumSamples)
		{
			if (bIsRecording)
			{
				RecordedSamples.Append(InAudio, NumSamples);
			}
		});

	CapturedSampleRate = AudioCapture->GetSampleRate();
	CapturedNumChannels = AudioCapture->GetNumChannels();

	AudioCapture->StartCapturingAudio();
	bIsRecording = true;

	PRINTLOG_SH(TEXT("Started recording audio: SampleRate=%d, NumChannels=%d"), CapturedSampleRate, CapturedNumChannels);
}

void UVoiceRecorderComponent::StopRecording(const FString& FileName, bool bSaveToWav)
{
	if (!bIsRecording)
	{
		return;
	}

	bIsRecording = false;

	if (AudioCapture)
	{
		AudioCapture->StopCapturingAudio();
	}

	PRINTLOG_SH(TEXT("Stopped recording audio. Total recorded samples: %d"), RecordedSamples.Num());

	if (RecordedSamples.Num() == 0)
	{
		PRINTLOG_SH(TEXT("No audio samples were recorded."));
		return;
	}

	if (bSaveToWav)
	{
		SaveToWav(FileName);
	}

	// AI 서버 전송 담당 팀원은 GetRecordedSamples()로 메모리 버퍼를 바로 가져가면 됨
}

void UVoiceRecorderComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsRecording && AudioCapture)
	{
		AudioCapture->StopCapturingAudio();
		bIsRecording = false;
	}

	Super::EndPlay(EndPlayReason);
}

void UVoiceRecorderComponent::SaveToWav(const FString& FileName)
{
	const FString SaveDir = FPaths::ProjectSavedDir();

	Audio::FSampleBuffer SampleBuffer(
		RecordedSamples.GetData(),
		RecordedSamples.Num(),
		CapturedNumChannels,
		CapturedSampleRate);

	TSharedPtr<Audio::FSoundWavePCMWriter> Writer = MakeShared<Audio::FSoundWavePCMWriter>();
	const FString FullPath = SaveDir + FileName + TEXT(".wav");

	Writer->BeginWriteToWavFile(SampleBuffer, FileName, SaveDir,
	                            [this, FullPath, Writer]() // Writer 캡처로 콜백 끝날 때까지 수명 유지
	                            {
		                            UE_LOG(LogTemp, Log, TEXT("[VoiceRecorder] Saved: %s"), *FullPath);
		                            OnRecordingFinished.Broadcast(FullPath);
	                            });
}
