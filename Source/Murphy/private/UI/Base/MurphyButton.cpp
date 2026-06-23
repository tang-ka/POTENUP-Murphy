// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Base/MurphyButton.h"

#include "Kismet/GameplayStatics.h"
#include "Murphy.h"
#include "Sound/SoundBase.h"

UMurphyButton::UMurphyButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UMurphyButton::RebuildWidget()
{
	TSharedRef<SWidget> RebuiltWidget = Super::RebuildWidget();

	OnHovered.RemoveDynamic(this, &UMurphyButton::HandleButtonHovered);
	OnUnhovered.RemoveDynamic(this, &UMurphyButton::HandleButtonUnhovered);
	OnPressed.RemoveDynamic(this, &UMurphyButton::HandleButtonPressed);
	OnReleased.RemoveDynamic(this, &UMurphyButton::HandleButtonReleased);

	OnHovered.AddDynamic(this, &UMurphyButton::HandleButtonHovered);
	OnUnhovered.AddDynamic(this, &UMurphyButton::HandleButtonUnhovered);
	OnPressed.AddDynamic(this, &UMurphyButton::HandleButtonPressed);
	OnReleased.AddDynamic(this, &UMurphyButton::HandleButtonReleased);

	return RebuiltWidget;
}

void UMurphyButton::HandleButtonHovered()
{
	PRINTLOG_SH(TEXT("MurphyButton Hovered"));

	if (bUseScaleEffect)
	{
		ApplyUniformScale(HoverScale);
	}

	if (bUseSoundEffect)
	{
		PlayButtonSound(HoverSound);
	}
}

void UMurphyButton::HandleButtonUnhovered()
{
	PRINTLOG_SH(TEXT("MurphyButton Unhovered"));

	if (bUseScaleEffect)
	{
		ApplyUniformScale(NormalScale);
	}
}

void UMurphyButton::HandleButtonPressed()
{
	PRINTLOG_SH(TEXT("MurphyButton Pressed"));

	if (bUseScaleEffect)
	{
		ApplyUniformScale(PressedScale);
	}

	if (bUseSoundEffect)
	{
		PlayButtonSound(PressedSound);
	}
}

void UMurphyButton::HandleButtonReleased()
{
	PRINTLOG_SH(TEXT("MurphyButton Released"));

	if (bUseScaleEffect)
	{
		ApplyUniformScale(IsHovered() ? HoverScale : NormalScale);
	}
}

void UMurphyButton::ApplyUniformScale(float InScale)
{
	SetRenderScale(FVector2D(InScale, InScale));
}

void UMurphyButton::PlayButtonSound(USoundBase* InSound) const
{
	if (!InSound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, InSound);
}
