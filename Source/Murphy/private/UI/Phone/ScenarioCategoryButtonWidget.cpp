// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/ScenarioCategoryButtonWidget.h"

#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
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

	PRINTLOG_SH(TEXT("CategoryButton Set: %s"), *InCategoryName.ToString());
}

void UScenarioCategoryButtonWidget::OnCategoryChanged(bool bIsChecked)
{
	if (bIsChecked)
	{
		OnCategorySelected.Broadcast(CategoryName);
		PRINTLOG_SH(TEXT("Category 선택: %s"), *CategoryName.ToString());
	}
}
