// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UILayerTypes.generated.h"

/**
 * UI 위젯 렌더 레이어.
 * 값이 클수록 화면 앞에 렌더된다.
 * AddToViewport 시에는 GetUILayerZOrder()로 실제 ZOrder를 구한다.
 */
UENUM(BlueprintType)
enum class EUILayer : uint8
{
	Game,         // 시나리오 HUD (최하단)
	Persistent,   // 항상 떠 있는 UI (HUD)
	Menu,         // 설정·일시정지
	Modal,        // 확인 팝업 (입력 차단)
	Notification, // 토스트·알림 (입력 비차단)
	System,       // 로딩·페이드
	Cinematic     // 풀스크린 시네마틱 커버 (최상단)
};

/**
 * EUILayer → AddToViewport ZOrder 값 반환.
 * 레이어 간 간격을 넉넉히 둬서 같은 레이어 내 미세 조정 여지를 남긴다.
 */
inline int32 GetUILayerZOrder(EUILayer Layer)
{
	switch (Layer)
	{
	case EUILayer::Game:         return 0;
	case EUILayer::Persistent:   return 100;
	case EUILayer::Menu:         return 200;
	case EUILayer::Modal:        return 300;
	case EUILayer::Notification: return 400;
	case EUILayer::System:       return 500;
	case EUILayer::Cinematic:    return 10000;
	default:                     return 0;
	}
}

