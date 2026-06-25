// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/UserDialogBlockWidget.h"

#include "Components/TextBlock.h"
#include "Murphy.h"

void UUserDialogBlockWidget::SetDialogData(const FText& InTime, const FText& InContent)
{
	if (Txt_Time)
	{
		Txt_Time->SetText(InTime);
	}

	if (Txt_Content)
	{
		Txt_Content->SetText(InContent);
	}

	PRINTLOG_SH(TEXT("UserDialogBlock Set"));
}
