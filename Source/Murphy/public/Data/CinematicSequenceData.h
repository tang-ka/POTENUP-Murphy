// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/CinematicTypes.h"   // FCinematicFadeParams 재사용
#include "CinematicSequenceData.generated.h"

class UMediaSource;

/** 레벨에서 순서대로 재생할 영상 한 개의 명세. */
USTRUCT(BlueprintType)
struct FCinematicEntry
{
	GENERATED_BODY()

	// 로깅/디버그용 라벨.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	FName CinematicId = NAME_None;

	// 재생할 영상. soft ref라 추후 비동기 로드/직렬화 안전.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TSoftObjectPtr<UMediaSource> Media;

	// 0 = 미디어 자연 종료까지. >0 = 강제 길이 캡(초).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bSkippable = true;

	// 이 클립의 오디오 볼륨 배율 (0 = 무음, 1 = 원음). 클립별 음성 크기 조절용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float VolumeScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	FCinematicFadeParams Fade;
};

/**
 * 한 레벨에 묶이는 시네마틱 시퀀스.
 * GameMode가 들고 있다가 레벨 시작 시 CinematicSequenceSubsystem으로 재생한다.
 */
UCLASS(BlueprintType)
class MURPHY_API UCinematicSequenceData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 순서대로 재생할 영상 목록.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TArray<FCinematicEntry> Entries;

	// 마지막 영상 후 트래블할 레벨 키 (LevelStreamingSettings.LevelMap).
	// None이면 트래블 없이 게임으로 복귀(검정 해제).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	FName NextLevelKey = NAME_None;
};
