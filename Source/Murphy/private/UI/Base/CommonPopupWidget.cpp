// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Base/CommonPopupWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "TimerManager.h"

void UCommonPopupWidget::NativeDestruct()
{
    // 팝업이 닫힐 때 타이머 클리어 (잔여 타이머로 인한 오류 방지)
    if (LifeTimeHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(LifeTimeHandle);
    }
    
    Super::NativeDestruct();
}

void UCommonPopupWidget::Setup(const FUIPopupDesc& InDesc)
{
    Desc = InDesc;

    if (Txt_Title)
    {
        Txt_Title->SetText(Desc.Title);
    }

    if (Txt_Content)
    {
        Txt_Content->SetText(Desc.Content);
    }

    // 버튼 텍스트 주입
    if (Btn_Okay)
    {
        if (Txt_OkayLabel)
        {
            Txt_OkayLabel->SetText(Desc.OkayText);
        }
        Btn_Okay->OnClicked.AddDynamic(this, &UCommonPopupWidget::HandleOkay);
    }

    if (Btn_Cancel)
    {
        if (Txt_CancelLabel)
        {
            Txt_CancelLabel->SetText(Desc.CancelText);
        }
        Btn_Cancel->OnClicked.AddDynamic(this, &UCommonPopupWidget::HandleCancel);
    }

    // 버튼 구성 적용
    ApplyButtonLayout(Desc.Buttons);

    // LifeTime > 0 이면 자동 소멸
    if (Desc.LifeTime > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            LifeTimeHandle,
            FTimerDelegate::CreateLambda([this]()
            {
                RemoveFromParent();
            }),
            Desc.LifeTime,
            false);
    }
}

void UCommonPopupWidget::ApplyButtonLayout(EPopupButtons InButtons)
{
    switch (InButtons)
    {
        case EPopupButtons::None:
        {
            // 버튼 영역 통째로 제거 → Content만 남고 레이아웃 축소
            if (Box_Buttons)
            {
                Box_Buttons->SetVisibility(ESlateVisibility::Collapsed);
            }
            break;
        }

        case EPopupButtons::OkayOnly:
        {
            if (Box_Buttons)
            {
                Box_Buttons->SetVisibility(ESlateVisibility::Visible);
            }
            if (Btn_Okay)
            {
                Btn_Okay->SetVisibility(ESlateVisibility::Visible);
            }
            if (Btn_Cancel)
            {
                // Collapsed → 자리 차지 안 함, Okay가 자동 중앙 정렬
                Btn_Cancel->SetVisibility(ESlateVisibility::Collapsed);
            }
            break;
        }

        case EPopupButtons::OkayCancel:
        {
            if (Box_Buttons)
            {
                Box_Buttons->SetVisibility(ESlateVisibility::Visible);
            }
            if (Btn_Okay)
            {
                Btn_Okay->SetVisibility(ESlateVisibility::Visible);
            }
            if (Btn_Cancel)
            {
                Btn_Cancel->SetVisibility(ESlateVisibility::Visible);
            }
            break;
        }
    }
}

void UCommonPopupWidget::HandleOkay()
{
    Desc.OnOkay.ExecuteIfBound();
    RemoveFromParent();
}

void UCommonPopupWidget::HandleCancel()
{
    Desc.OnCancel.ExecuteIfBound();
    RemoveFromParent();
}