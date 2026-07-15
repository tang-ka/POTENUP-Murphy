// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CinematicSettings.generated.h"

class UCinematicOverlayWidget;

/**
 * CinematicManagerSubsystem이 사용하는 커버 위젯/기본값을 에디터에서 관리하는 설정 클래스.
 * Project Settings > Game > "Cinematic Settings" 에서 지정.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Cinematic Settings"))
class MURPHY_API UCinematicSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 풀스크린 커버 위젯 클래스 (WBP_CinematicOverlay). */
	UPROPERTY(Config, EditAnywhere, Category="Widget Classes")
	TSoftClassPtr<UCinematicOverlayWidget> OverlayWidgetClass;
};
