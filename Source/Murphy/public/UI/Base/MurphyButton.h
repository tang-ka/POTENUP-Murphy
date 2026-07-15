// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "MurphyButton.generated.h"

class USoundBase;
class SWidget;

UCLASS(Blueprintable, meta = (DisplayName = "Murphy Button"))
class MURPHY_API UMurphyButton : public UButton
{
	GENERATED_BODY()

public:
	explicit UMurphyButton(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void HandleButtonHovered();

	UFUNCTION()
	void HandleButtonUnhovered();

	UFUNCTION()
	void HandleButtonPressed();

	UFUNCTION()
	void HandleButtonReleased();

	void ApplyUniformScale(float InScale);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Scale")
	bool bUseScaleEffect = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Scale", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float NormalScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Scale", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HoverScale = 1.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Scale", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PressedScale = 0.95f;
};
