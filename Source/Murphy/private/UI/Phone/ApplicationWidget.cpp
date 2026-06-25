// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/ApplicationWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Murphy.h"

void UApplicationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_AppIcon)
	{
		Btn_AppIcon->OnClicked.AddDynamic(this, &UApplicationWidget::OnIconButtonClicked);
		Btn_AppIcon->OnHovered.AddDynamic(this, &UApplicationWidget::OnIconButtonHovered);
		Btn_AppIcon->OnUnhovered.AddDynamic(this, &UApplicationWidget::OnIconButtonUnhovered);
	}
}

void UApplicationWidget::SetAppData(const FPhoneAppRow& Row, UUserWidget* InAppScreen)
{
	// 앱 이름 설정
	if (Txt_AppName)
	{
		Txt_AppName->SetText(Row.AppName);
		AppName = FName(*Row.AppName.ToString());
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
			FSlateBrush PressedBrush = NormalBrush;
			PressedBrush.TintColor   = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.7f));

			FButtonStyle Style = Btn_AppIcon->GetStyle();
			Style.Normal  = NormalBrush;
			Style.Hovered = NormalBrush;
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
	
	if (Btn_AppIcon)
	{
		FWidgetTransform Transform = Btn_AppIcon->GetRenderTransform();
		Transform.Scale = FVector2D(0.95f, 0.95f);
		Btn_AppIcon->SetRenderTransform(Transform);
	}
}

void UApplicationWidget::OnIconButtonHovered()
{
	PRINTLOG_SH(TEXT("AppIcon Hovered"));

	if (Btn_AppIcon)
	{
		FWidgetTransform Transform = Btn_AppIcon->GetRenderTransform();
		Transform.Scale = FVector2D(1.05f, 1.05f);
		Btn_AppIcon->SetRenderTransform(Transform);
	}
}

void UApplicationWidget::OnIconButtonUnhovered()
{
	PRINTLOG_SH(TEXT("AppIcon Unhovered"));

	if (Btn_AppIcon)
	{
		FWidgetTransform Transform = Btn_AppIcon->GetRenderTransform();
		Transform.Scale = FVector2D(1.0f, 1.0f);
		Btn_AppIcon->SetRenderTransform(Transform);
	}
}

