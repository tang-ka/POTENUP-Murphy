#pragma once

#include "CoreMinimal.h"
#include "UITypes.generated.h"

UENUM(BlueprintType)
enum class EUILayer : uint8
{
	Game,         // 시나리오 HUD (최하단)
	Persistent,   // 항상 떠 있는 UI (HUD)
	Menu,         // 설정·일시정지
	Modal,        // 확인 팝업 (입력 차단)
	Notification, // 토스트·알림 (입력 비차단)
	System        // 로딩·페이드 (최상단)
};
