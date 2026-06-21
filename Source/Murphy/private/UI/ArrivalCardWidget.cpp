// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ArrivalCardWidget.h"

#include "Components/EditableText.h"
#include "Components/TextBlock.h"

void UArrivalCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (txt_SurnameDisplay)
	{
		txt_SurnameDisplay->SetVisibility(ESlateVisibility::Visible);
	}
	if (txt_GivennameDisplay)
	{
		txt_GivennameDisplay->SetVisibility(ESlateVisibility::Visible);
	}
	if (etxt_Surname)
	{
		etxt_Surname->OnTextChanged.AddDynamic(this, &UArrivalCardWidget::OnSurnameTextChanged);
	}
	if (etxt_Givenname)
	{
		etxt_Givenname->OnTextChanged.AddDynamic(this, &UArrivalCardWidget::OnGivennameTextChanged);
	}
}

void UArrivalCardWidget::SetReadOnlyData(const FText& InSurname, const FText& InGivenname)
{
	if (!etxt_Surname || !etxt_Givenname || !txt_SurnameDisplay || !txt_GivennameDisplay) return;
	
	// 텍스트 복사
	txt_SurnameDisplay->SetText(InSurname);
	txt_GivennameDisplay->SetText(InGivenname);
	
	etxt_Surname->SetVisibility(ESlateVisibility::Collapsed);
	etxt_Givenname->SetVisibility(ESlateVisibility::Collapsed);
	txt_SurnameDisplay->SetVisibility(ESlateVisibility::Visible);
	txt_GivennameDisplay->SetVisibility(ESlateVisibility::Visible);
}

void UArrivalCardWidget::OnSurnameTextChanged(const FText& Text)
{
	FString InputStr = Text.ToString();
	FString FilteredStr = "";
	int32 MaxLength = 20;	// 최대 글자 수
	
	// 한 글자씩 검사 -> 영문만 통과
	for (TCHAR Char : InputStr)
	{
		if ((Char >= 'A' && Char <= 'Z') || (Char >= 'a' && Char <= 'z'))
		{
			FilteredStr.AppendChar(Char);
		}
	}
	
	// 글자 수 제한 (MaxLength 초과 시 뒷부분 잘라내기)
	if (FilteredStr.Len() > MaxLength)
	{
		FilteredStr = FilteredStr.Left(MaxLength);
	}
	
	// 필터링된 결과가 원본과 다를 때만 SetText 실행 (무한루프 방지)
	if (!InputStr.Equals(FilteredStr, ESearchCase::CaseSensitive))
	{
		etxt_Surname->SetText(FText::FromString(FilteredStr));
	}
}

void UArrivalCardWidget::OnGivennameTextChanged(const FText& Text)
{
	FString InputStr = Text.ToString();
	FString FilteredStr = "";
	int32 MaxLength = 20;	// 최대 글자 수
	
	// 한 글자씩 검사 -> 영문만 통과
	for (TCHAR Char : InputStr)
	{
		if ((Char >= 'A' && Char <= 'Z') || (Char >= 'a' && Char <= 'z'))
		{
			FilteredStr.AppendChar(Char);
		}
	}
	
	// 글자 수 제한 (MaxLength 초과 시 뒷부분 잘라내기)
	if (FilteredStr.Len() > MaxLength)
	{
		FilteredStr = FilteredStr.Left(MaxLength);
	}
	
	// 필터링된 결과가 원본과 다를 때만 SetText 실행 (무한루프 방지)
	if (!InputStr.Equals(FilteredStr, ESearchCase::CaseSensitive))
	{
		etxt_Givenname->SetText(FText::FromString(FilteredStr));
	}
}
