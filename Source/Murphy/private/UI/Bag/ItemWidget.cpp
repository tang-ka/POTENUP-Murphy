
#include "UI/Bag/ItemWidget.h"
#include "UI/Bag/ItemDetailWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Framework/MurphyPlayerController.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Item)
	{
		Btn_Item->OnClicked.AddDynamic(this, &UItemWidget::OnItemButtonClicked);
	}
}

void UItemWidget::InitItem(const FItemTableRow& Info)
{
	ItemData = Info;

	// 이미지 세팅
	if (Btn_Item)
	{
		const float MaxLimit = 90.f;
		
		FSlateBrush Brush;
		if (UTexture2D* LoadedTexture = Info.ItemIcon.LoadSynchronous())
		{
			float OrigW = LoadedTexture->GetSurfaceWidth();
			float OrigH = LoadedTexture->GetSurfaceHeight();

			float ScaleFactor = FMath::Min(MaxLimit / OrigW, MaxLimit / OrigH);
			// 만약 원본이 350x350보다 작은 이미지일 때, 억지로 확대되는 것을 막음
			ScaleFactor = FMath::Min(1.f, ScaleFactor);

			FVector2D FinalSize = FVector2D(OrigW * ScaleFactor, OrigH * ScaleFactor);

			Brush.SetResourceObject(LoadedTexture);
			Brush.ImageSize = FinalSize;
		}
		
		FButtonStyle Style;
		Style.SetNormal(Brush);
		Style.SetHovered(Brush);
		Style.SetPressed(Brush);
		Btn_Item->SetStyle(Style);
	}
	
	if (Txt_ItemName)
	{
		Txt_ItemName->SetText(Info.ItemName);
	}
}

void UItemWidget::OnItemButtonClicked()
{
	ShowDetailPopup();
}

void UItemWidget::UseItemFromUI()
{
	if (!ItemData.bIsUsable)
	{
		return;
	}

	// 버튼 클릭으로 "사용"이 확정된 시점에 퀘스트를 먼저 통보해 BP 오버라이드 누락을 방지합니다.
	NotifyQuestCondition(EQuestClearCondition::UseItem);
	UseItem();
}

void UItemWidget::UseItem_Implementation()
{
	// todo 기본 구현 - 블루프린트에서 오버라이드하여 실제 사용 로직 추가
	// 예: 사용 효과, 사운드, 이펙트 등
}

void UItemWidget::ShowDetailPopup()
{
	if (!IsValid(ItemDetailWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] ItemDetailWidgetClass가 설정되지 않았습니다."));
		return;
	}

	UItemDetailWidget* DetailWidget = CreateWidget<UItemDetailWidget>(GetOwningPlayer(), ItemDetailWidgetClass);
	if (!IsValid(DetailWidget))
	{
		return;
	}

	DetailWidget->InitDetail(ItemData, this);
	DetailWidget->AddToViewport(10); // Z-Order: BagPopup보다 위에 표시
	
	NotifyQuestCondition(EQuestClearCondition::CheckItem);
}

void UItemWidget::NotifyQuestCondition(EQuestClearCondition Condition) const
{
	if (ItemData.ItemID.IsNone())
	{
		return;
	}

	AMurphyPlayerController* MurphyPC = Cast<AMurphyPlayerController>(GetOwningPlayer());
	if (!IsValid(MurphyPC))
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] 퀘스트 이벤트 !!"));

	// 가방 UI는 직접 RPC를 호출하지 않고, PC가 소유한 QuestEventNotifier 경로를 재사용합니다.
	MurphyPC->NotifyQuestConditionFromLocal(ItemData.ItemID, Condition);
}
