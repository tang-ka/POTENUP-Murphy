// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TransitionWidget.generated.h"

class UImage;

/**
 * 풀스크린 페이드 트랜지션 위젯.
 * - Img_Fade : 화면 가득 채운 단색 이미지. 색은 FadeColor, 알파만 보간.
 *
 * 페이드 구동은 위젯이 NativeTick에서 자기완결적으로 수행한다.
 * UIManagerSubsystem(LocalPlayerSubsystem)은 틱이 없으므로 구동을 위젯으로 가져온다.
 *
 * 의미 규약:
 *  - FadeOut : 화면 -> 검정 (알파 0 -> 1)
 *  - FadeIn  : 검정 -> 화면 (알파 1 -> 0)
 *
 * NOTE: World 소속이라 OpenLevel(하드 트래블)을 관통하지 못한다.
 *       맵 전환 커버는 추후 MoviePlayer 레이어로 별도 처리(범위 밖).
 */
UCLASS(Abstract, Blueprintable)
class MURPHY_API UTransitionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * From -> To 알파를 Duration 동안 보간. 완료 시 OnComplete 실행.
	 * Duration <= 0 이면 즉시 To 적용 후 콜백.
	 */
	void StartFade(float From, float To, float Duration, const FLinearColor& Color, FSimpleDelegate OnComplete);

	/** 현재 페이드 진행 중인지. */
	bool IsFading() const { return bFading; }

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_Fade;

private:
	// 알파만 적용 (rgb는 FadeColor 유지).
	void ApplyAlpha(float Alpha);

	// 진행 중인 페이드 완료 처리 (델리게이트 실행 + 상태 정리).
	void CompleteFade();

	bool bFading = false;
	float FromAlpha = 0.f;
	float ToAlpha = 0.f;
	float FadeDuration = 0.f;
	float Elapsed = 0.f;
	FLinearColor FadeColor = FLinearColor::Black;
	FSimpleDelegate OnFadeComplete;
};
