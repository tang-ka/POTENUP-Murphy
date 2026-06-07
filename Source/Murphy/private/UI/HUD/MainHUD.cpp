// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/MainHUD.h"


#include "UI/HUD/BagPopupWidget.h"

void UMainHUD::RequestToggleBag()
{
	if (WBP_BagPopup)
	{
		WBP_BagPopup->ToggleBag();
	}
}


