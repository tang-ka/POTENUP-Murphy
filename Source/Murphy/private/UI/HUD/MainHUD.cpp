// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/MainHUD.h"

#include "GameFramework/PlayerController.h"
#include "Murphy.h"
#include "UI/HUD/BagPopupWidget.h"
#include "UI/HUD/CaptionWidget.h"
#include "UI/HUD/MyMicWidget.h"
#include "UI/HUD/PhonePopupWidget.h"
#include "UI/Phone/ApplicationWidget.h"
#include "UI/Phone/TranslateAppScreenWidget.h"
#include "UI/Phone/TranslateDialogManager.h"

void UMainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	DialogManager = NewObject<UTranslateDialogManager>(this);

	if (UTranslateAppScreenWidget* Screen = GetTranslateScreen())
	{
		Screen->InitializeWithManager(DialogManager);
	}

	if (WBP_PhonePopup)
	{
		WBP_PhonePopup->OnPhoneToggled.AddDynamic(this, &UMainHUD::HandlePhoneToggled);
	}
}

void UMainHUD::HandlePhoneToggled(bool bIsPhoneOpen)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	if (bIsPhoneOpen)
	{
		PC->SetShowMouseCursor(true);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
	else
	{
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
	if (WBP_MyMic)
	{
		WBP_MyMic->SetRecordingState(bIsRecording);
	}
}

void UMainHUD::UpdateCaption(const FString& CaptionText)
{
	if (WBP_PlayerCaption)
	{
		WBP_PlayerCaption->SetCaption(CaptionText);
	}
}

void UMainHUD::AddTranslateDialog(FName InCategoryName, const FDialogEntry& Entry)
{
	if (!DialogManager)
	{
		return;
	}

	DialogManager->AddDialog(InCategoryName, Entry);
}

void UMainHUD::AddTranslateCategory(FName InCategoryName, const FText& DisplayName)
{
	if (!DialogManager)
	{
		return;
	}

	DialogManager->RegisterCategory(InCategoryName, DisplayName);
}

void UMainHUD::SetTranslateConnecting(bool bIsConnecting)
{
	if (UTranslateAppScreenWidget* Screen = GetTranslateScreen())
	{
		Screen->SetConnecting(bIsConnecting);
	}
}

UTranslateAppScreenWidget* UMainHUD::GetTranslateScreen() const
{
	if (!WBP_PhonePopup || !WBP_PhonePopup->WBP_Translate)
	{
		return nullptr;
	}

	return Cast<UTranslateAppScreenWidget>(WBP_PhonePopup->WBP_Translate->AppScreen);
}
