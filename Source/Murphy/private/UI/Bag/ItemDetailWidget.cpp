

#include "UI/Bag/ItemDetailWidget.h"
#include "UI/Bag/ItemWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UItemDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 콜백 바인딩
	if (Btn_Use)
	{
		Btn_Use->OnClicked.AddDynamic(this, &UItemDetailWidget::OnUseClicked);
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UItemDetailWidget::OnCloseClicked);
	}
}

void UItemDetailWidget::InitDetail(const FItemTableRow& Info, UItemWidget* InOwnerWidget)
{
	CachedItemInfo = Info;
	OwnerItemWidget = InOwnerWidget;

	// 이미지 세팅
	const FVector2D MaxLimit = FVector2D(370.f, 360.f);
	if (UTexture2D* LoadedTexture = Info.ItemIcon.LoadSynchronous())
	{
		float OrigW = LoadedTexture->GetSurfaceWidth();
		float OrigH = LoadedTexture->GetSurfaceHeight();

		float ScaleFactor = FMath::Min(MaxLimit.X / OrigW, MaxLimit.Y / OrigH);
		// 만약 원본이 350x350보다 작은 이미지일 때, 억지로 확대되는 것을 막음
		ScaleFactor = FMath::Min(1.f, ScaleFactor);

		FVector2D FinalSize = FVector2D(OrigW * ScaleFactor, OrigH * ScaleFactor);

		// 3. 브러시 생성 및 적용
		FSlateBrush Brush;
		Brush.SetResourceObject(LoadedTexture);
		Brush.ImageSize = FinalSize;

		Img_ItemImage->SetBrush(Brush);
	}
	
	
	// 텍스트 세팅
	if (Txt_ItemName)
	{
		Txt_ItemName->SetText(Info.ItemName);
	}

	if (Txt_ItemDescription)
	{
		Txt_ItemDescription->SetText(Info.ItemDescription);
	}

	// 사용 불가 아이템은 사용 버튼 비활성화
	if (Btn_Use)
	{
		Btn_Use->SetIsEnabled(Info.bIsUsable);
	}
}

void UItemDetailWidget::OnUseClicked()
{
	// 사용 처리를 OwnerItemWidget에 위임
	if (IsValid(OwnerItemWidget))
	{
		OwnerItemWidget->UseItem();
	}

	// 팝업 닫기
	RemoveFromParent();
}

void UItemDetailWidget::OnCloseClicked()
{
	RemoveFromParent();
}
