// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/ApplicationWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UApplicationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_AppIcon)
	{
		Btn_AppIcon->OnClicked.AddDynamic(this, &UApplicationWidget::OnIconButtonClicked);
	}
}

void UApplicationWidget::SetAppData(const FPhoneAppRow& Row, UUserWidget* InAppScreen)
{
	// 앱 이름 설정
	if (Txt_AppName)
	{
		Txt_AppName->SetText(Row.AppName);
	}

	// Btn_AppIcon의 Normal / Hovered / Pressed 브러시에 텍스처 할당
	if (Btn_AppIcon && !Row.AppIcon.IsNull())
	{
		if (UTexture2D* Tex = Row.AppIcon.LoadSynchronous())
		{
			const FVector2D IconSize = FVector2D(Tex->GetSizeX(), Tex->GetSizeY());

			// Normal: 텍스처 이미지
			FSlateBrush NormalBrush;
			NormalBrush.SetResourceObject(Tex);
			NormalBrush.ImageSize = IconSize;
			NormalBrush.DrawAs   = ESlateBrushDrawType::Image;

			// Hovered: RoundedBox + 흰색 Outline (CornerRadii=20, Width=1)
			FSlateBrush HoveredBrush;
			HoveredBrush.SetResourceObject(Tex);
			HoveredBrush.ImageSize                    = IconSize;
			HoveredBrush.DrawAs                       = ESlateBrushDrawType::RoundedBox;
			HoveredBrush.OutlineSettings.CornerRadii  = FVector4(20.f, 20.f, 20.f, 20.f);
			HoveredBrush.OutlineSettings.Color        = FLinearColor::White;
			HoveredBrush.OutlineSettings.Width        = 1.f;
			HoveredBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;

			// Pressed: Hovered와 동일 + Tint (1, 1, 1, 0.7)
			FSlateBrush PressedBrush = HoveredBrush;
			PressedBrush.TintColor   = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.7f));

			FButtonStyle Style = Btn_AppIcon->GetStyle();
			Style.Normal  = NormalBrush;
			Style.Hovered = HoveredBrush;
			Style.Pressed = PressedBrush;
			Btn_AppIcon->SetStyle(Style);
		}
	}

	// DataManager가 생성한 앱 화면 위젯 저장
	AppScreen = InAppScreen;
}

void UApplicationWidget::OnIconButtonClicked()
{
	OnAppIconClicked.Broadcast(this);
}
