// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/BagPopupWidget.h"


void UBagPopupWidget::ToggleBag()
{
	if (!Anim_SlideUp) return;
	
	if (!bIsOpen)
	{
		// 닫힌 상태
		PlayAnimation(Anim_SlideUp, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		bIsOpen = true;
	}
	else
	{
		// 열린 상태
		PlayAnimation(Anim_SlideUp, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.0f);
		bIsOpen = false;
	}
}
