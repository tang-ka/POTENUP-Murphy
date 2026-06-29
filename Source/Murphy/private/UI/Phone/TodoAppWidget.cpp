// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/TodoAppWidget.h"

#include "Components/CheckBox.h"
#include "Components/Image.h"

void UTodoAppWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Img_StrikeThrough1)
	{
		Img_StrikeThrough1->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Img_StrikeThrough2)
	{
		Img_StrikeThrough2->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Img_StrikeThrough3)
	{
		Img_StrikeThrough3->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Check_First)
	{
		Check_First->OnCheckStateChanged.AddDynamic(this, &UTodoAppWidget::OnFirstCheckChanged);
	}

	if (Check_Second)
	{
		Check_Second->OnCheckStateChanged.AddDynamic(this, &UTodoAppWidget::OnSecondCheckChanged);
	}

	if (Check_Third)
	{
		Check_Third->OnCheckStateChanged.AddDynamic(this, &UTodoAppWidget::OnThirdCheckChanged);
	}
}

void UTodoAppWidget::OnFirstCheckChanged(bool bIsChecked)
{
	if (Img_StrikeThrough1)
	{
		Img_StrikeThrough1->SetVisibility(bIsChecked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UTodoAppWidget::OnSecondCheckChanged(bool bIsChecked)
{
	if (Img_StrikeThrough2)
	{
		Img_StrikeThrough2->SetVisibility(bIsChecked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UTodoAppWidget::OnThirdCheckChanged(bool bIsChecked)
{
	if (Img_StrikeThrough3)
	{
		Img_StrikeThrough3->SetVisibility(bIsChecked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
