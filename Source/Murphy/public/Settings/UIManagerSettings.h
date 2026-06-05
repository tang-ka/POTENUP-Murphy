// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UIManagerSettings.generated.h"

class ULevelEnterToastPopupWidget;
class UCommonPopupWidget;
class UUserWidget;

/**
 * UIManagerSubsystem이 사용하는 위젯 클래스와 레이어 설정을 에디터에서 관리하는 설정 클래스
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="UIManager Settings"))
class MURPHY_API UUIManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ====== 위젯 클래스 ======

	/** 팝업(다이얼로그) 위젯 기본 클래스 */
	UPROPERTY(Config, EditAnywhere, Category="Widget Classes")
	TSoftClassPtr<UCommonPopupWidget> PopupClass;

	/** 토스트(알림) 위젯 클래스 */
	UPROPERTY(Config, EditAnywhere, Category="Widget Classes")
	TSoftClassPtr<UUserWidget> ToastClass;

	/** Level Enter 토스트(알림) 위젯 클래스 */
	UPROPERTY(Config, EditAnywhere, Category="Widget Classes")
	TSoftClassPtr<ULevelEnterToastPopupWidget> LevelEnterToastClass;
	
	// ====== 토스트 기본값 ======

	/** 토스트 메시지 기본 표시 시간 (초) */
	UPROPERTY(Config, EditAnywhere, Category="Toast", meta=(ClampMin="0.5", ClampMax="30.0"))
	float DefaultToastLifeTime = 3.f;

	// ====== 페이드 기본값 ======

	/** 기본 페이드 아웃/인 시간 (초) */
	UPROPERTY(Config, EditAnywhere, Category="Fade", meta=(ClampMin="0.0", ClampMax="5.0"))
	float DefaultFadeDuration = 0.5f;
};

