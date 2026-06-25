// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/ScenarioCategoryButtonWidget.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Murphy.h"

void UScenarioCategoryButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Tgl_Category)
	{
		Tgl_Category->OnCheckStateChanged.AddDynamic(this, &UScenarioCategoryButtonWidget::OnCategoryChanged);
	}
}

void UScenarioCategoryButtonWidget::SetCategoryData(FName InCategoryName, const FText& InDisplayName)
{
	CategoryName = InCategoryName;

	if (Txt_ScenarioName)
	{
		Txt_ScenarioName->SetText(InDisplayName);
	}

	UpdateTextColor(IsCategorySelected());

	PRINTLOG_SH(TEXT("CategoryButton Set: %s"), *InCategoryName.ToString());
}

void UScenarioCategoryButtonWidget::SetSelected(bool bSelected)
{
	bIsUpdatingSelection = true;

	if (Tgl_Category)
	{
		Tgl_Category->SetIsChecked(bSelected);
	}

	UpdateTextColor(bSelected);

	bIsUpdatingSelection = false;
}

bool UScenarioCategoryButtonWidget::IsCategorySelected() const
{
	return Tgl_Category && Tgl_Category->IsChecked();
}

void UScenarioCategoryButtonWidget::OnCategoryChanged(bool bIsChecked)
{
	if (bIsUpdatingSelection)
	{
		return;
	}

	UpdateTextColor(bIsChecked);

	if (bIsChecked)
	{
		OnCategorySelected.Broadcast(CategoryName);
		PRINTLOG_SH(TEXT("Category Selected: %s"), *CategoryName.ToString());
	}
	else
	{
		SetSelected(true);
	}
}

void UScenarioCategoryButtonWidget::UpdateTextColor(bool bIsSelected)
{
	if (Txt_ScenarioName)
	{
		const FLinearColor TextColor = bIsSelected ? FLinearColor::White : FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		Txt_ScenarioName->SetColorAndOpacity(FSlateColor(TextColor));
	}
}
