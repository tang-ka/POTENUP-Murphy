// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Airplane/AirplaneGameState.h"

#include "GameFramework/PlayerController.h"

void AAirplaneGameState::RestoreInputModeAfterIntro(APlayerController* LocalPC)
{
	if (!LocalPC)
	{
		return;
	}

	// 기내 씬은 도착카드 등 마우스 UI가 필요하므로 커서 유지.
	LocalPC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false); // 캡처 중 커서 숨김 방지 (없으면 클릭 전까지 커서 안 보임)
	LocalPC->SetInputMode(InputMode);
}

