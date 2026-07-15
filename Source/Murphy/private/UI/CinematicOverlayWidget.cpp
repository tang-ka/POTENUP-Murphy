// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/CinematicOverlayWidget.h"

#include "Components/Image.h"
#include "MediaTexture.h"

void UCinematicOverlayWidget::SetBlackOpacity(float Alpha)
{
	BlackAlpha = FMath::Clamp(Alpha, 0.f, 1.f);

	if (Img_Black)
	{
		// rgb는 FadeColor, 알파만 별도 제어.
		Img_Black->SetColorAndOpacity(FLinearColor(FadeColor.R, FadeColor.G, FadeColor.B, BlackAlpha));
	}
}

void UCinematicOverlayWidget::SetMediaOpacity(float Alpha)
{
	if (Img_Media)
	{
		Img_Media->SetRenderOpacity(FMath::Clamp(Alpha, 0.f, 1.f));
	}
}

void UCinematicOverlayWidget::SetFadeColor(const FLinearColor& InColor)
{
	FadeColor = InColor;

	// 색 변경 후 현재 알파 즉시 반영.
	SetBlackOpacity(BlackAlpha);
}

void UCinematicOverlayWidget::SetMediaTexture(UMediaTexture* MediaTexture)
{
	if (!Img_Media)
	{
		return;
	}

	// UMediaTexture는 UTexture2D가 아니므로 SetBrushFromTexture를 쓸 수 없다.
	// FSlateBrush에 ResourceObject로 직접 지정.
	FSlateBrush Brush;
	Brush.SetResourceObject(MediaTexture);
	if (MediaTexture)
	{
		Brush.ImageSize = FVector2D(MediaTexture->GetSurfaceWidth(), MediaTexture->GetSurfaceHeight());
	}
	Brush.DrawAs = ESlateBrushDrawType::Image;

	Img_Media->SetBrush(Brush);
}
