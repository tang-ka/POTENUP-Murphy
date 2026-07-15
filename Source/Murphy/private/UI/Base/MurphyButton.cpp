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
	if (bUseScaleEffect)
	{
		ApplyUniformScale(HoverScale);
	}
}

void UMurphyButton::HandleButtonUnhovered()
{
	if (bUseScaleEffect)
	{
		ApplyUniformScale(NormalScale);
	}
}

void UMurphyButton::HandleButtonPressed()
{
	if (bUseScaleEffect)
	{
		ApplyUniformScale(PressedScale);
	}
}

void UMurphyButton::HandleButtonReleased()
{
	if (bUseScaleEffect)
	{
		ApplyUniformScale(IsHovered() ? HoverScale : NormalScale);
	}
}

void UMurphyButton::ApplyUniformScale(float InScale)
{
	SetRenderScale(FVector2D(InScale, InScale));
}
