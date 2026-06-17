// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LobbyUI.h"

#include "Murphy.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/VerticalBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Manager/NetworkManagerSubsystem.h"
#include "UI/SessionInfoWidget.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_SinglePlay)
	{
		Btn_SinglePlay->OnClicked.AddDynamic(this, &ULobbyUI::OnSinglePlayButtonClicked);
	}
	if (Btn_MultiPlay)
	{
		Btn_MultiPlay->OnClicked.AddDynamic(this, &ULobbyUI::OnMultiPlayButtonClicked);
	}
	if (Btn_Achievement)
	{
		Btn_Achievement->OnClicked.AddDynamic(this, &ULobbyUI::OnAchievementButtonClicked);
	}
	if (Btn_Option)
	{
		Btn_Option->OnClicked.AddDynamic(this, &ULobbyUI::OnOptionButtonClicked);
	}
	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &ULobbyUI::OnQuitButtonClicked);
	}
	if (Btn_RefreshSessionList)
	{
		Btn_RefreshSessionList->OnClicked.AddDynamic(this, &ULobbyUI::OnRefreshSessionListButtonClicked);
	}
	if (Btn_ToggleSessionSetting)
	{
		Btn_ToggleSessionSetting->OnClicked.AddDynamic(this, &ULobbyUI::OnToggleSessionSettingButtonClicked);
	}
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &ULobbyUI::OnBackButtonClicked);
	}
	if (Btn_JoinSession)
	{
		Btn_JoinSession->OnClicked.AddDynamic(this, &ULobbyUI::OnJoinSessionButtonClicked);
	}

	if (Panel_Session)
	{
		Panel_Session->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Panel_SessionInfoSetting)
	{
		Panel_SessionInfoSetting->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Btn_CreateSession)
	{
		Btn_CreateSession->OnClicked.AddDynamic(this, &ULobbyUI::OnCreateSessionButtonClicked);
	}
}

void ULobbyUI::OnSinglePlayButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULevelStreamingSubsystem* LevelSubsystem = GI->GetSubsystem<ULevelStreamingSubsystem>())
		{
			LevelSubsystem->TravelAllPlayers(FName("Airplane"));
		}
	}
}

void ULobbyUI::OnMultiPlayButtonClicked()
{
	if (Panel_Session)
	{
		Panel_Session->SetVisibility(ESlateVisibility::Visible);
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		PRINTLOG_SH(TEXT("OnMultiPlayButtonClicked: GameInstance is null."));
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = GI->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		PRINTLOG_SH(TEXT("OnMultiPlayButtonClicked: NetworkManagerSubsystem is null."));
		return;
	}

	NetworkManager->OnFindSessionsCompleted.AddUObject(this, &ULobbyUI::HandleFindSessionsComplete);
	NetworkManager->FindSessions(30, true);

	if (Ovl_Loading)
	{
		Ovl_Loading->SetVisibility(ESlateVisibility::Visible);
	}
	if (Anim_Loading)
	{
		PlayAnimation(Anim_Loading, 0.0f, 0);
	}

	PRINTLOG_SH(TEXT("멀티플레이 버튼 클릭 — 세션 검색 시작"));
}

void ULobbyUI::OnAchievementButtonClicked()
{
}

void ULobbyUI::OnOptionButtonClicked()
{
}

void ULobbyUI::OnQuitButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
}

void ULobbyUI::OnRefreshSessionListButtonClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		PRINTLOG_SH(TEXT("OnMultiPlayButtonClicked: GameInstance is null."));
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = GI->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		PRINTLOG_SH(TEXT("OnMultiPlayButtonClicked: NetworkManagerSubsystem is null."));
		return;
	}

	NetworkManager->OnFindSessionsCompleted.AddUObject(this, &ULobbyUI::HandleFindSessionsComplete);
	NetworkManager->FindSessions(30, true);

	if (Ovl_Loading)
	{
		Ovl_Loading->SetVisibility(ESlateVisibility::Visible);
	}
	if (Anim_Loading)
	{
		PlayAnimation(Anim_Loading, 0.0f, 0);
	}
}

void ULobbyUI::OnToggleSessionSettingButtonClicked()
{
	if (!Panel_SessionInfoSetting)
	{
		return;
	}

	const bool bIsVisible = Panel_SessionInfoSetting->GetVisibility() == ESlateVisibility::Visible;
	Panel_SessionInfoSetting->SetVisibility(bIsVisible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	PRINTLOG_SH(TEXT("Panel_SessionInfoSetting 토글 — %s"), bIsVisible ? TEXT("OFF") : TEXT("ON"));
}

void ULobbyUI::OnBackButtonClicked()
{
	if (Panel_Session)
	{
		Panel_Session->SetVisibility(ESlateVisibility::Collapsed);
	}

	ClearSessionList();
	SelectedSessionIndex = INDEX_NONE;

	PRINTLOG_SH(TEXT("Back 버튼 클릭 — Panel_Session 닫힘"));
}

void ULobbyUI::OnJoinSessionButtonClicked()
{
	if (SelectedSessionIndex == INDEX_NONE)
	{
		PRINTLOG_SH(TEXT("참가할 세션이 선택되지 않았습니다."));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		PRINTLOG_SH(TEXT("OnJoinSessionButtonClicked: GameInstance is null."));
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = GI->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		PRINTLOG_SH(TEXT("OnJoinSessionButtonClicked: NetworkManagerSubsystem is null."));
		return;
	}

	PRINTLOG_SH(TEXT("세션 참가 시도 — Index:%d"), SelectedSessionIndex);
	NetworkManager->JoinSessionByIndex(SelectedSessionIndex);
}

void ULobbyUI::HandleFindSessionsComplete(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& Results)
{
	// 델리게이트 해제 (일회성)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UNetworkManagerSubsystem* NetworkManager = GI->GetSubsystem<UNetworkManagerSubsystem>())
		{
			NetworkManager->OnFindSessionsCompleted.RemoveAll(this);
		}
	}

	if (Ovl_Loading)
	{
		Ovl_Loading->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Anim_Loading)
	{
		StopAnimation(Anim_Loading);
	}

	ClearSessionList();
	SelectedSessionIndex = INDEX_NONE;

	if (!bWasSuccessful)
	{
		PRINTLOG_SH(TEXT("세션 검색 실패 — 목록 갱신 안 함"));
		return;
	}

	if (!VB_SessionList)
	{
		PRINTLOG_SH(TEXT("HandleFindSessionsComplete: VB_SessionList is null."));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = GI->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		return;
	}

	if (!SessionInfoWidgetClass)
	{
		PRINTLOG_SH(TEXT("HandleFindSessionsComplete: WBP_SessionInfo Widget 클래스가 설정되지 않았습니다."));
		return;
	}

	for (int32 i = 0; i < Results.Num(); ++i)
	{
		FSessionInfo Info = NetworkManager->ExtractSessionInfo(Results[i]);

		USessionInfoWidget* Widget = CreateWidget<USessionInfoWidget>(GetOwningPlayer(), SessionInfoWidgetClass);
		if (!Widget)
		{
			continue;
		}

		Widget->Init(i, Info.SessionName, Info.HostName, 0, Info.MaxPlayers);
		Widget->OnSessionInfoSelected.BindUObject(this, &ULobbyUI::OnSessionSelected);

		VB_SessionList->AddChild(Widget);
		SessionWidgetList.Add(Widget);
	}

	PRINTLOG_SH(TEXT("세션 목록 갱신 완료 — %d개"), SessionWidgetList.Num());
}

void ULobbyUI::OnSessionSelected(USessionInfoWidget* SelectedWidget)
{
	if (!SelectedWidget)
	{
		return;
	}

	// 라디오 버튼: 나머지 모두 해제
	if (SessionWidgetList.IsValidIndex(SelectedSessionIndex))
	{
		SessionWidgetList[SelectedSessionIndex]->SetChecked(false);
	}
	
	SelectedSessionIndex = SelectedWidget->GetSessionIndex();
	PRINTLOG_SH(TEXT("세션 선택 — Index:%d"), SelectedSessionIndex);
}

void ULobbyUI::OnCreateSessionButtonClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		PRINTLOG_SH(TEXT("OnCreateSessionButtonClicked: GameInstance is null."));
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = GI->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		PRINTLOG_SH(TEXT("OnCreateSessionButtonClicked: NetworkManagerSubsystem is null."));
		return;
	}

	FSessionInfo Info;
	Info.SessionName = Input_SessionName ? Input_SessionName->GetText().ToString() : TEXT("");
	Info.HostName    = Input_HostName    ? Input_HostName->GetText().ToString()    : TEXT("");
	Info.MaxPlayers  = 2;
	Info.bIsLAN      = true;

	PRINTLOG_SH(TEXT("세션 생성 요청 — SessionName:%s, HostName:%s"), *Info.SessionName, *Info.HostName);
	NetworkManager->CreateSession(Info);
}

void ULobbyUI::ClearSessionList()
{
	if (VB_SessionList)
	{
		VB_SessionList->ClearChildren();
	}

	SessionWidgetList.Empty();
	PRINTLOG_SH(TEXT("세션 목록 초기화"));
}
