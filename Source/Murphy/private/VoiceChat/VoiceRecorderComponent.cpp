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
		PRINTLOGW_JW(TEXT("No samples to play."));
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

	PRINTLOGW_JW(TEXT("Playing %d samples (SR=%d, Ch=%d)"),
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
		PRINTLOGW_JW(TEXT("Failed to create UAudioCapture"));
		return;
	}

	AudioCapture->AddGeneratorDelegate(
		[this](const float* InAudio, int32 NumSamples)
		{
			if (!bIsRecording)
			{
				return;
			}

			if (bChunkingMode)
			{
				// Chunking Mode: 16kHz Resample + PCM16 변환 후 청크 단위로 브로드캐스트
				ProcessAndBroadcastChunk(InAudio, NumSamples, false);
			}
			else
			{
				// 기존 WAV 방식: float 샘플 그대로 누적
				RecordedSamples.Append(InAudio, NumSamples);
			}
		});

	CapturedSampleRate = AudioCapture->GetSampleRate();
	CapturedNumChannels = AudioCapture->GetNumChannels();

	AudioCapture->StartCapturingAudio();
	bIsRecording = true;

	PRINTLOGW_JW(TEXT("Started recording audio: SampleRate=%d, NumChannels=%d"), CapturedSampleRate, CapturedNumChannels);
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

	PRINTLOGW_JW(TEXT("Stopped recording audio. Total recorded samples: %d"), RecordedSamples.Num());

	if (RecordedSamples.Num() == 0 && !bChunkingMode)
	{
		PRINTLOGW_JW(TEXT("No audio samples were recorded."));
		return;
	}

	if (bSaveToWav)
	{
		SaveToWav(FileName);
	}

	if (bChunkingMode)
	{
		// 나머지 버퍼를 마지막 청크로 flush (commit=true)
		const float DummySilence[1] = { 0.0f };
		ProcessAndBroadcastChunk(DummySilence, 0, /*bIsLast=*/true);
		ChunkBuffer16k.Reset();
		bChunkingMode = false; // 다음 StartRecording 전에 다시 설정해야 함
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

void UVoiceRecorderComponent::SetChunkingMode(bool bEnable, int32 ChunkSizeMs)
{
	bChunkingMode = bEnable;

	// 16kHz 기준 1ms = 16 samples
	ChunkSizeSamples16k = FMath::Max(1, ChunkSizeMs * 16);
	ChunkBuffer16k.Reset();

	PRINTLOGW_JW(TEXT("[VoiceRecorder] ChunkingMode=%s, ChunkSize=%dms (%d samples @16kHz)"),
		bEnable ? TEXT("ON") : TEXT("OFF"), ChunkSizeMs, ChunkSizeSamples16k);
}

void UVoiceRecorderComponent::ProcessAndBroadcastChunk(const float* InAudio, int32 NumSamples, bool bIsLast)
{
	// -------------------------------------------------------
	// 1) 다운샘플링: CapturedSampleRate / CapturedNumChannels → 16kHz mono
	//    선형 보간 방식. 오디오 품질보다 실시간 지연 최소화 우선.
	// -------------------------------------------------------
	if (NumSamples > 0 && CapturedSampleRate > 0 && CapturedNumChannels > 0)
	{
		const float RatioToMono = 1.0f / CapturedNumChannels;
		const float ResampleRatio = static_cast<float>(CapturedSampleRate) / 16000.0f;

		// 원본 프레임 수 (채널 합산 전)
		const int32 NumFrames = NumSamples / CapturedNumChannels;
		// 다운샘플 후 예상 출력 프레임 수
		const int32 OutFrames = FMath::CeilToInt(NumFrames / ResampleRatio);

		for (int32 OutIdx = 0; OutIdx < OutFrames; ++OutIdx)
		{
			// 선형 보간을 위한 원본 인덱스
			const float SrcF = OutIdx * ResampleRatio;
			const int32 SrcA = FMath::Clamp(static_cast<int32>(SrcF), 0, NumFrames - 1);
			const int32 SrcB = FMath::Clamp(SrcA + 1, 0, NumFrames - 1);
			const float T    = SrcF - SrcA;

			// 멀티채널 → mono 평균
			float SampleA = 0.0f;
			float SampleB = 0.0f;
			for (int32 Ch = 0; Ch < CapturedNumChannels; ++Ch)
			{
				SampleA += InAudio[SrcA * CapturedNumChannels + Ch];
				SampleB += InAudio[SrcB * CapturedNumChannels + Ch];
			}
			SampleA *= RatioToMono;
			SampleB *= RatioToMono;

			// float → int16 변환
			const float Mixed    = FMath::Lerp(SampleA, SampleB, T);
			const float Clamped  = FMath::Clamp(Mixed, -1.0f, 1.0f);
			const int16 PCM16Val = static_cast<int16>(Clamped * 32767.0f);

			ChunkBuffer16k.Add(PCM16Val);
		}
	}

	// -------------------------------------------------------
	// 2) 버퍼가 청크 사이즈 이상이거나 마지막 청크이면 브로드캐스트
	// -------------------------------------------------------
	bool bForceCommit = bIsLast && (ChunkBuffer16k.Num() == 0); // 빈 버퍼라도 마지막이면 강제 전송

	while (ChunkBuffer16k.Num() >= ChunkSizeSamples16k || (bIsLast && ChunkBuffer16k.Num() > 0) || bForceCommit)
	{
		const bool bLastChunkNow = bIsLast && (ChunkBuffer16k.Num() <= ChunkSizeSamples16k);
		const int32 SendCount    = FMath::Min(ChunkSizeSamples16k, ChunkBuffer16k.Num());

		TArray<uint8> ByteChunk;
		if (SendCount > 0)
		{
			// int16[] → uint8[] (바이트 배열로 변환)
			ByteChunk.SetNumUninitialized(SendCount * sizeof(int16));
			FMemory::Memcpy(ByteChunk.GetData(), ChunkBuffer16k.GetData(), ByteChunk.Num());

			// 전송한 샘플 제거
			ChunkBuffer16k.RemoveAt(0, SendCount, EAllowShrinking::No);
		}

		bForceCommit = false; // 한 번 보냈으니 해제

		// 게임 스레드 안전 브로드캐스트
		// AddGeneratorDelegate는 오디오 스레드에서 호출되므로 AsyncTask 사용
		TArray<uint8> ChunkCopy = MoveTemp(ByteChunk);
		const bool bLastCopy    = bLastChunkNow;
		AsyncTask(ENamedThreads::GameThread, [this, ChunkCopy = MoveTemp(ChunkCopy), bLastCopy]()
		{
			OnAudioChunkReady.Broadcast(ChunkCopy, bLastCopy);
		});

		if (bLastChunkNow)
		{
			break;
		}
	}
}

