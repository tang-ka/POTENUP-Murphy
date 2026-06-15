// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CinematicTypes.generated.h"

class UMediaSource;

/**
 * 시네마틱 재생 상태 머신.
 * Idle -> FadingToBlack -> MediaFadingIn -> Playing
 *      -> MediaFadingOut -> HoldingBlack -> [ReleaseHold] -> FadingFromBlack -> Idle
 *
 * HoldingBlack 에서만 외부 입력(ReleaseHold)을 기다리고, 나머지는 시간 기반 자동 전이.
 */
UENUM(BlueprintType)
enum class ECinematicState : uint8
{
	Idle,
	FadingToBlack,    // 게임 -> 검정
	MediaFadingIn,    // 검정 -> 미디어
	Playing,          // 미디어 풀 노출
	MediaFadingOut,   // 미디어 -> 검정
	HoldingBlack,     // 검정 유지 (서버 합의 대기 구간)
	FadingFromBlack   // 검정 -> 게임
};

/** "검정 경유" 4구간 페이드 파라미터. */
USTRUCT(BlueprintType)
struct FCinematicFadeParams
{
	GENERATED_BODY()

	// 게임화면 -> 검정. 시작이 이미 검정인 케이스는 0으로 스킵.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Fade")
	float FadeToBlackDuration = 0.5f;

	// 검정 -> 미디어
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Fade")
	float MediaFadeInDuration = 0.5f;

	// 미디어 -> 검정 (종료 시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Fade")
	float MediaFadeOutDuration = 0.5f;

	// 검정 -> 게임/새 맵
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Fade")
	float FadeFromBlackDuration = 0.5f;

	// 레터박스 배경 + Hold 커버색
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Fade")
	FLinearColor FadeColor = FLinearColor::Black;
};

/** 재생 명세서. relay/게임코드가 이 한 덩어리를 넘기면 재생된다. */
USTRUCT(BlueprintType)
struct FCinematicPlayRequest
{
	GENERATED_BODY()

	// 콘텐츠 식별 (로깅/디버그용 라벨). 스킵/종료 매칭은 PlayId가 담당.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	FName CinematicId = NAME_None;

	// 미디어 소스 - soft ref라 추후 RPC 직렬화 안전
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TSoftObjectPtr<UMediaSource> MediaSource;

	// 0 = 미디어 자연 종료(OnEndReached)까지. >0 = 강제 길이 캡(초).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bSkippable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	FCinematicFadeParams Fade;
};
