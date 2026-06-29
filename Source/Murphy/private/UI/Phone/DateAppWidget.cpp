// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/DateAppWidget.h"

#include "Components/TextBlock.h"

void UDateAppWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const FDateTime Now = FDateTime::Now();

	const TArray<FString> DayNames =
	{
		TEXT("일요일"),
		TEXT("월요일"),
		TEXT("화요일"),
		TEXT("수요일"),
		TEXT("목요일"),
		TEXT("금요일"),
		TEXT("토요일")
	};

	const int32 DayIndex = static_cast<int32>(Now.GetDayOfWeek());
	const FString DayStr = DayNames[DayIndex];
	const FString DateStr = FString::Printf(TEXT("%d"), Now.GetDay());

	if (Txt_Day)
	{
		Txt_Day->SetText(FText::FromString(DayStr));
	}

	if (Txt_Date)
	{
		Txt_Date->SetText(FText::FromString(DateStr));
	}
}
