// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/MainHUD.h"


#include "UI/HUD/BagPopupWidget.h"
#include "UI/HUD/MyMicWidget.h"
#include "UI/HUD/PhonePopupWidget.h"

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


