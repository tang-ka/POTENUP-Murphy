// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/AgentDialogBlockWidget.h"

#include "Components/TextBlock.h"
#include "Murphy.h"

void UAgentDialogBlockWidget::SetDialogData(const FText& InName, const FText& InTime, const FText& InContent)
{
	if (Txt_Name)
	{
		Txt_Name->SetText(InName);
	}

	if (Txt_Time)
	{
		Txt_Time->SetText(InTime);
	}

	if (Txt_Content)
	{
		Txt_Content->SetText(InContent);
	}

	PRINTLOG_SH(TEXT("AgentDialogBlock Set: Name=%s"), *InName.ToString());
}
