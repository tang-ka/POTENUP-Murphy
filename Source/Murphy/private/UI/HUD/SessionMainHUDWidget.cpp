// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/SessionMainHUDWidget.h"

#include "Murphy.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Manager/NetworkManagerSubsystem.h"
#include "Manager/UIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"

void USessionMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Boy)
	{
		Btn_Boy->OnClicked.AddDynamic(this, &USessionMainHUDWidget::HandleBtnBoyClicked);
	}

	if (Btn_Girl)
	{
		Btn_Girl->OnClicked.AddDynamic(this, &USessionMainHUDWidget::HandleBtnGirlClicked);
	}

	if (Btn_Ready)
	{
		Btn_Ready->OnClicked.AddDynamic(this, &USessionMainHUDWidget::HandleBtnReadyClicked);
	}

	if (Btn_Start)
	{
		Btn_Start->OnClicked.AddDynamic(this, &USessionMainHUDWidget::HandleBtnStartClicked);
	}

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &USessionMainHUDWidget::HandleBtnExitClicked);
	}
}

void USessionMainHUDWidget::SetPlayerRole(bool bIsHost)
{
	const ESlateVisibility HostVisibility  = bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	const ESlateVisibility GuestVisibility = bIsHost ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;

	if (Btn_Start) { Btn_Start->SetVisibility(HostVisibility);  }
	if (Txt_Start) { Txt_Start->SetVisibility(HostVisibility);  }
	if (Btn_Ready) { Btn_Ready->SetVisibility(GuestVisibility); }
	if (Txt_Ready) { Txt_Ready->SetVisibility(GuestVisibility); }

	PRINTLOG_SH(TEXT("SetPlayerRole — bIsHost: %s"), bIsHost ? TEXT("true") : TEXT("false"));
}

void USessionMainHUDWidget::HandleBtnBoyClicked()
{
	PRINTLOG_SH(TEXT("Btn_Boy Clicked"));
	OnCharacterSelected.ExecuteIfBound(EPlayerCharacterType::BoyCharacter);
}

void USessionMainHUDWidget::HandleBtnGirlClicked()
{
	PRINTLOG_SH(TEXT("Btn_Girl Clicked"));
	OnCharacterSelected.ExecuteIfBound(EPlayerCharacterType::GirlCharacter);
}

void USessionMainHUDWidget::HandleBtnReadyClicked()
{
	bIsReady = !bIsReady;

	if (Txt_Ready)
	{
		const FText NewText = bIsReady
			? FText::FromString(TEXT("준비 취소"))
			: FText::FromString(TEXT("준비 완료"));
		Txt_Ready->SetText(NewText);
	}

	PRINTLOG_SH(TEXT("Btn_Ready Clicked — bIsReady: %s"), bIsReady ? TEXT("true") : TEXT("false"));
	OnReadyRequested.ExecuteIfBound(bIsReady);
}

void USessionMainHUDWidget::HandleBtnStartClicked()
{
	PRINTLOG_SH(TEXT("Btn_Start Clicked"));

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UUIManagerSubsystem* UIManager = LocalPlayer ? LocalPlayer->GetSubsystem<UUIManagerSubsystem>() : nullptr;
	if (!UIManager)
	{
		// UIManager가 없으면 페이드 없이 즉시 시작.
		PRINTLOG_SH(TEXT("HandleBtnStartClicked: UIManagerSubsystem is null — 페이드 없이 즉시 시작."));
		OnStartRequested.ExecuteIfBound();
		return;
	}

	// 페이드 아웃(화면 -> 검정) 완료 후 시작 요청 실행.
	FSimpleDelegate OnFadeOutComplete = FSimpleDelegate::CreateWeakLambda(this, [this]()
	{
		OnStartRequested.ExecuteIfBound();
	});

	// Duration 0 -> UIManagerSettings::DefaultFadeDuration 사용.
	UIManager->FadeOut(0.f, OnFadeOutComplete);
}

void USessionMainHUDWidget::HandleBtnExitClicked()
{
	PRINTLOG_SH(TEXT("Btn_Exit Clicked"));

	APlayerController* OwningPC = GetOwningPlayer();
	if (!OwningPC)
	{
		PRINTLOG_SH(TEXT("HandleBtnExitClicked: OwningPlayer is null."));
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = OwningPC->GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		PRINTLOG_SH(TEXT("HandleBtnExitClicked: NetworkManagerSubsystem is null."));
		return;
	}

	// 싱글플레이는 세션이 없으므로 로비로 직접 이동
	if (NetworkManager->GetSessionState() != ESessionState::InSession)
	{
		PRINTLOG_SH(TEXT("HandleBtnExitClicked: 싱글플레이 — 로비로 직접 이동"));
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/Lv_Lobby")));
		return;
	}

	// 멀티플레이: 세션 종료 후 로비 복귀 (HandleDestroySessionComplete에서 처리)
	NetworkManager->DestroySession();
}
