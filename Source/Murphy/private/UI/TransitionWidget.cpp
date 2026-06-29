// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/TransitionWidget.h"

#include "Murphy.h"
#include "Components/Image.h"

void UTransitionWidget::StartFade(float From, float To, float Duration, const FLinearColor& Color, FSimpleDelegate OnComplete)
{
	FromAlpha = FMath::Clamp(From, 0.f, 1.f);
	ToAlpha = FMath::Clamp(To, 0.f, 1.f);
	FadeColor = Color;
	FadeDuration = Duration;
	Elapsed = 0.f;

	// 재진입: 이전 페이드 콜백은 버리고 새 목표로 갱신.
	OnFadeComplete = OnComplete;

	// 페이드 진행 중에는 입력을 막는다. (이전 페이드 인 완료로 Collapsed 였을 수 있어 복구)
	SetVisibility(ESlateVisibility::Visible);

	// 시작 알파 즉시 반영 (AddToViewport 직후 깜빡 방지).
	ApplyAlpha(FromAlpha);

	// 0초 구간은 즉시 통과 (1프레임 깜빡 방지).
	if (FadeDuration <= KINDA_SMALL_NUMBER)
	{
		bFading = false;
		ApplyAlpha(ToAlpha);
		CompleteFade();
		return;
	}

	bFading = true;
}

void UTransitionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bFading)
	{
		return;
	}

	Elapsed += InDeltaTime;

	const float T = FMath::Clamp(Elapsed / FadeDuration, 0.f, 1.f);
	const float Alpha = FMath::Lerp(FromAlpha, ToAlpha, T);
	ApplyAlpha(Alpha);

	if (Elapsed >= FadeDuration)
	{
		bFading = false;
		ApplyAlpha(ToAlpha);
		CompleteFade();
	}
}

void UTransitionWidget::ApplyAlpha(float Alpha)
{
	if (Img_Fade)
	{
		const float Clamped = FMath::Clamp(Alpha, 0.f, 1.f);
		Img_Fade->SetColorAndOpacity(FLinearColor(FadeColor.R, FadeColor.G, FadeColor.B, Clamped));
	}
}

void UTransitionWidget::CompleteFade()
{
	// 페이드 인 완료(완전 투명)면 화면/입력을 가릴 필요가 없으므로 접는다.
	// 페이드 아웃 완료(검정)면 곧 트래블하므로 가시(입력 차단) 상태를 유지한다.
	if (ToAlpha <= KINDA_SMALL_NUMBER)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	// 콜백 내부에서 StartFade 재진입해도 안전하도록 지역 복사 후 먼저 비운다.
	FSimpleDelegate Local = OnFadeComplete;
	OnFadeComplete.Unbind();
	Local.ExecuteIfBound();
}
