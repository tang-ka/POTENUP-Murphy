
#include "UI/Bag/ItemWidget.h"
#include "UI/Bag/ItemDetailWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

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
	if (ItemData.bIsUsable)
	{
		// 즉시 사용
		UseItem();
	}
	else
	{
		// 상세 팝업 표시
		ShowDetailPopup();
	}
}

void UItemWidget::UseItem_Implementation()
{
	// todo 기본 구현 - 블루프린트에서 오버라이드하여 실제 사용 로직 추가
	// 예: 사용 효과, 사운드, 이펙트 등

	// todo 퀘스트 통과 
}

void UItemWidget::ShowDetailPopup()
{
	if (!IsValid(ItemDetailWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemWidget] ItemDetailWidgetClass가 설정되지 않았습니다."));
		return;
	}

	UItemDetailWidget* DetailWidget = CreateWidget<UItemDetailWidget>(GetWorld(), ItemDetailWidgetClass);
	if (!IsValid(DetailWidget))
	{
		return;
	}

	DetailWidget->InitDetail(ItemData, this);
	DetailWidget->AddToViewport(10); // Z-Order: BagPopup보다 위에 표시
	
	// todo 퀘스트 통과 
}
