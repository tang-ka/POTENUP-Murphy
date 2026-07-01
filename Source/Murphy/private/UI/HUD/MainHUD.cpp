// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/MainHUD.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Murphy.h"
#include "UI/HUD/BagPopupWidget.h"
#include "UI/HUD/CaptionWidget.h"
#include "UI/HUD/MyMicWidget.h"
#include "UI/HUD/PhonePopupWidget.h"
#include "UI/Phone/ApplicationWidget.h"
#include "UI/Phone/TranslateAppScreenWidget.h"
#include "UI/Phone/TranslateDialogManager.h"
#include "UI/PlayReport/PlayReportMainWidget.h"

void UMainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	DialogManager = LocalPlayer ? LocalPlayer->GetSubsystem<UTranslateDialogManager>() : nullptr;

	if (!DialogManager)
	{
		PRINTLOG_SH(TEXT("TranslateDialogManager LocalPlayerSubsystem is null"));
	}

	if (UTranslateAppScreenWidget* Screen = GetTranslateScreen())
	{
		Screen->InitializeWithManager(DialogManager);
	}

	if (WBP_PhonePopup)
	{
		WBP_PhonePopup->OnPhoneToggled.AddDynamic(this, &UMainHUD::HandlePhoneToggled);
	}

	ClearCaption();
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
	if (!bCaptionInteractionActive || !WBP_PlayerCaption)
	{
		return;
	}

	if (CaptionText.IsEmpty())
	{
		// STT 이벤트는 도착했지만 인식된 텍스트가 아직 없는 상태 → "..."로 표시
		WBP_PlayerCaption->SetCaption(TEXT("..."));
		WBP_PlayerCaption->SetCaptionVisible(true);
		return;
	}

	WBP_PlayerCaption->SetCaption(CaptionText);
	WBP_PlayerCaption->SetCaptionVisible(true);
}

void UMainHUD::SetCaptionInteractionActive(bool bIsActive)
{
	bCaptionInteractionActive = bIsActive;

	if (!WBP_PlayerCaption)
	{
		return;
	}

	if (!bCaptionInteractionActive)
	{
		// 대화 종료 시에만 즉시 숨김 + 텍스트 초기화
		WBP_PlayerCaption->ClearCaption();
	}

	// bIsActive == true 인 경우 여기서 강제로 켜지 않는다.
	// 실제 표시는 STT 텍스트가 도착하는 UpdateCaption()에서만 수행 (STT 나올 때만 켜짐)
}

void UMainHUD::ClearCaption()
{
	bCaptionInteractionActive = false;

	if (WBP_PlayerCaption)
	{
		WBP_PlayerCaption->ClearCaption();
	}
}

void UMainHUD::SetReportVisible(bool bIsVisible)
{
	if (bIsVisible)
	{
		WBP_PlayReportMain->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		WBP_PlayReportMain->SetVisibility(ESlateVisibility::Hidden);
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

void UMainHUD::BeginTranslateConversation(FName InCategoryName, const FText& InDisplayName)
{
	if (!DialogManager)
	{
		return;
	}

	DialogManager->SetActiveCategory(InCategoryName);
	DialogManager->RegisterCategory(InCategoryName, InDisplayName); // 중복 자동 무시
	PRINTLOG_SH(TEXT("Translate Conversation Begin: %s"), *InCategoryName.ToString());
}

void UMainHUD::AddAgentDialog(const FString& InSpeaker, const FString& InText)
{
	if (!DialogManager || DialogManager->GetActiveCategory().IsNone())
	{
		PRINTLOG_SH(TEXT("AddAgentDialog skipped: no active category"));
		return;
	}

	FDialogEntry Entry;
	Entry.Type = EDialogType::Agent;
	Entry.Name = FText::FromString(InSpeaker);
	Entry.Time = FText::FromString(FDateTime::Now().ToString(TEXT("%H:%M")));
	Entry.Content = FText::FromString(InText);

	DialogManager->AddDialog(DialogManager->GetActiveCategory(), Entry);
}

void UMainHUD::AddUserDialog(const FString& InText)
{
	if (!DialogManager || DialogManager->GetActiveCategory().IsNone())
	{
		PRINTLOG_SH(TEXT("AddUserDialog skipped: no active category"));
		return;
	}

	FDialogEntry Entry;
	Entry.Type = EDialogType::User;
	Entry.Time = FText::FromString(FDateTime::Now().ToString(TEXT("%H:%M")));
	Entry.Content = FText::FromString(InText);

	DialogManager->AddDialog(DialogManager->GetActiveCategory(), Entry);
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
