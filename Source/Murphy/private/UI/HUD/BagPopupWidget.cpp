// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/BagPopupWidget.h"
#include "UI/Bag/ItemWidget.h"

#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "GameFramework/PlayerController.h"

void UBagPopupWidget::ToggleBag()
{
	if (!Anim_BagSlideUp) return;

	if (!bIsOpen)
	{
		// 닫힌 상태 → 열기
		PlayAnimation(Anim_BagSlideUp, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		bIsOpen = true;
		SetMouseCursorEnabled(true);
	}
	else
	{
		// 열린 상태 → 닫기
		PlayAnimation(Anim_BagSlideUp, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.0f);
		bIsOpen = false;
		SetMouseCursorEnabled(false);
	}
}

void UBagPopupWidget::AddItem(const FItemTableRow& Item)
{
	if (!IsValid(ItemWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BagPopupWidget] ItemWidgetClass가 설정되지 않았습니다."));
		return;
	}

	if (!IsValid(Wbx_Items))
	{
		return;
	}

	UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(GetWorld(), ItemWidgetClass);
	if (!IsValid(NewItemWidget))
	{
		return;
	}

	NewItemWidget->InitItem(Item);
	Wbx_Items->AddChild(NewItemWidget);
}

void UBagPopupWidget::SetMouseCursorEnabled(bool bEnabled)
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
	{
		return;
	}

	if (bEnabled)
	{
		// 가방 열림 → 마우스 커서 활성화 + UI 입력 허용
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		// 가방 닫힘 → 마우스 커서 비활성화 + 게임 입력으로 복귀
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}
