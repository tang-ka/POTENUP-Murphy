#include "UI/Bag/ItemDetailWidget.h"
#include "UI/Bag/ItemWidget.h"
#include "UI/Bag/ItemUsableWidgetInterface.h"

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
	// 1. 퀘스트 통보 + UseItem 이벤트 (기존 경로 유지)
	if (IsValid(OwnerItemWidget))
	{
		OwnerItemWidget->UseItemFromUI();
	}

	// 2. DataTable에 UseWidgetClass가 지정된 아이템이면 해당 위젯을 띄움
	if (CachedItemInfo.UseWidgetClass)
	{
		UUserWidget* UseWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), CachedItemInfo.UseWidgetClass);
		if (IsValid(UseWidget))
		{
			// Interface를 구현한 위젯이면 아이템 정보를 주입 (미구현 위젯은 그냥 열기만 함)
			if (UseWidget->Implements<UItemUsableWidgetInterface>())
			{
				IItemUsableWidgetInterface::Execute_InitFromItemUse(UseWidget, CachedItemInfo);
			}
			// ItemDetail(10)보다 위, 최상위 레이어에 표시
			UseWidget->AddToViewport(20);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ItemDetailWidget] UseWidgetClass(%s) 위젯 생성 실패"), *CachedItemInfo.UseWidgetClass->GetName());
		}
	}

	// 3. 상세 팝업 닫기
	RemoveFromParent();
}

void UItemDetailWidget::OnCloseClicked()
{
	RemoveFromParent();
}
