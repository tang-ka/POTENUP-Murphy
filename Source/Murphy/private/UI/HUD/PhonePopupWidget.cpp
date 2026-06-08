// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PhonePopupWidget.h"

void UPhonePopupWidget::TogglePhone()
{
	if (!Anim_PhoneSlideUp) return;
	
	if (!bIsOpen)
	{
		// 닫힌 상태
		PlayAnimation(Anim_PhoneSlideUp, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		bIsOpen = true;
	}
	else
	{
		// 열린 상태
		PlayAnimation(Anim_PhoneSlideUp, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.0f);
		bIsOpen = false;
	}
}

