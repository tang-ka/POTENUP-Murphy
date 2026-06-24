// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SessionInfoWidget.h"

#include "Murphy.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

void USessionInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Tgl_SessionInfo)
	{
		Tgl_SessionInfo->OnCheckStateChanged.AddDynamic(this, &USessionInfoWidget::HandleCheckStateChanged);
	}
}

void USessionInfoWidget::Init(int32 InSessionIndex, const FString& InSessionName, const FString& InHostName, int32 InCurrentPlayers, int32 InMaxPlayers)
{
	SessionIndex = InSessionIndex;

	if (Txt_SessionName)
	{
		Txt_SessionName->SetText(FText::FromString(InSessionName));
	}

	if (Txt_HostName)
	{
		Txt_HostName->SetText(FText::FromString(InHostName));
	}

	if (Txt_PlayerCountInfo)
	{
		const FString PlayerCountStr = FString::Printf(TEXT("%d/%d"), InCurrentPlayers, InMaxPlayers);
		Txt_PlayerCountInfo->SetText(FText::FromString(PlayerCountStr));
	}

	PRINTLOG_SH(TEXT("SessionInfoWidget Init — Index:%d, Session:%s, Host:%s, Players:%d/%d"),
		SessionIndex, *InSessionName, *InHostName, InCurrentPlayers, InMaxPlayers);
}

void USessionInfoWidget::SetChecked(bool bChecked)
{
	if (Tgl_SessionInfo)
	{
		Tgl_SessionInfo->SetIsChecked(bChecked);
	}
}

void USessionInfoWidget::HandleCheckStateChanged(bool bIsChecked)
{
	if (bIsChecked)
	{
		PRINTLOG_SH(TEXT("SessionInfoWidget 선택됨 — Index:%d"), SessionIndex);
	}
	else
	{
		PRINTLOG_SH(TEXT("SessionInfoWidget 해제됨 — Index:%d"), SessionIndex);
	}
	OnSessionInfoSelected.ExecuteIfBound(this, bIsChecked);
}

