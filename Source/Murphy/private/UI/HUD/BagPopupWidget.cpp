// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/BagPopupWidget.h"
#include "UI/Bag/ItemWidget.h"

#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "Framework/MurphyPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Manager/DataManager.h"

void UBagPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindOwnedItemsSource();
	RefreshFromOwnedItems();
}

void UBagPopupWidget::ToggleBag()
{
	if (!Anim_BagSlideUp) return;

	if (!bIsOpen)
	{
		RefreshFromOwnedItems();

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

	UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(GetOwningPlayer(), ItemWidgetClass);
	if (!IsValid(NewItemWidget))
	{
		return;
	}

	NewItemWidget->InitItem(Item);
	Wbx_Items->AddChild(NewItemWidget);
}

bool UBagPopupWidget::HasItem(FName ItemID) const
{
	if (ItemID.IsNone() || !IsValid(Wbx_Items))
	{
		return false;
	}

	for (UWidget* ChildWidget : Wbx_Items->GetAllChildren())
	{
		const UItemWidget* ItemWidget = Cast<UItemWidget>(ChildWidget);
		if (!IsValid(ItemWidget))
		{
			continue;
		}

		if (ItemWidget->GetItemInfo().ItemID == ItemID)
		{
			return true;
		}
	}

	return false;
}

bool UBagPopupWidget::AddItemIfMissing(const FItemTableRow& Item)
{
	if (Item.ItemID.IsNone() || HasItem(Item.ItemID))
	{
		return false;
	}

	AddItem(Item);
	return HasItem(Item.ItemID);
}

void UBagPopupWidget::RefreshFromOwnedItems()
{
	BindOwnedItemsSource();

	if (!IsValid(BoundPlayerState) || !IsValid(Wbx_Items))
	{
		return;
	}

	UDataManager* DataManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDataManager>() : nullptr;
	if (!IsValid(DataManager))
	{
		return;
	}

	for (const FName& ItemID : BoundPlayerState->GetOwnedItemIDs())
	{
		if (ItemID.IsNone())
		{
			continue;
		}

		FItemTableRow* ItemInfo = DataManager->GetItemData(ItemID);
		if (!ItemInfo)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BagPopupWidget] 보유 아이템 Row를 찾을 수 없습니다: %s"), *ItemID.ToString());
			continue;
		}

		AddItemIfMissing(*ItemInfo);
	}
}

void UBagPopupWidget::BindOwnedItemsSource()
{
	AMurphyPlayerState* CurrentPlayerState = GetOwningPlayerState<AMurphyPlayerState>();
	if (!IsValid(CurrentPlayerState) || BoundPlayerState == CurrentPlayerState)
	{
		return;
	}

	if (IsValid(BoundPlayerState))
	{
		BoundPlayerState->OnOwnedItemsUpdated.RemoveDynamic(this, &UBagPopupWidget::RefreshFromOwnedItems);
	}

	BoundPlayerState = CurrentPlayerState;
	BoundPlayerState->OnOwnedItemsUpdated.AddUniqueDynamic(this, &UBagPopupWidget::RefreshFromOwnedItems);
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
