// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/MainHUD.h"

#include "GameFramework/PlayerController.h"
#include "UI/HUD/BagPopupWidget.h"
#include "UI/HUD/MyMicWidget.h"
#include "UI/HUD/PhonePopupWidget.h"
#include "UI/HUD/CaptionWidget.h"

void UMainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 폰 토글 델리게이트에 마우스 커서 핸들러 등록
	if (WBP_PhonePopup)
	{
		WBP_PhonePopup->OnPhoneToggled.AddDynamic(this, &UMainHUD::HandlePhoneToggled);
	}
}

void UMainHUD::HandlePhoneToggled(bool bIsPhoneOpen)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (bIsPhoneOpen)
	{
		// 폰이 열릴 때: 마우스 커서 표시 + UI 전용 입력 모드
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
	else
	{
		// 폰이 닫힐 때: 마우스 커서 숨김 + 게임 전용 입력 모드
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UMainHUD::RequestToggleBag()
{
	if (WBP_BagPopup)
	{
		WBP_BagPopup->ToggleBag();
	}
}

void UMainHUD::RequestTogglePhone()
{
	if (WBP_PhonePopup)
	{
		WBP_PhonePopup->TogglePhone();
	}
}

void UMainHUD::UpdateMicState(bool bIsRecording)
{
	if (WBP_MyMic != nullptr)
	{
		WBP_MyMic->SetRecordingState(bIsRecording);
	}
}
void UMainHUD::UpdateCaption(const FString& CaptionText)
{
	if (WBP_PlayerCaption != nullptr)
	{
		WBP_PlayerCaption->SetCaption(CaptionText);
	}
}
