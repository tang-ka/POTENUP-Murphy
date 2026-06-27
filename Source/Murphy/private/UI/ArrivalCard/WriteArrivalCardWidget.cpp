// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ArrivalCard/WriteArrivalCardWidget.h"

#include "Components/Button.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Airplane/AirplaneGameMode.h"
#include "UI/ArrivalCard/ArrivalCardWidget.h"

void UWriteArrivalCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_Complete)
	{
		btn_Complete->OnClicked.AddDynamic(this, &UWriteArrivalCardWidget::OnCompleteClicked);
	}
}

void UWriteArrivalCardWidget::OnCompleteClicked()
{
	if (WBP_ArrivalCard)
	{
		// 플레이어가 입력한 텍스트를 FString으로 가져오기
		FString ExtractedSurname = WBP_ArrivalCard->GetSurnameInput().ToString();
		FString ExtractedGivenname = WBP_ArrivalCard->GetGivennameInput().ToString();
		
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (AMurphyPlayerState* PS = PC->GetPlayerState<AMurphyPlayerState>())
			{
				// 서버로 데이터 전송
				PS->ServerSetArrivalData(ExtractedSurname, ExtractedGivenname);
			}
		}
	}
	
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
	
	RemoveFromParent();
	// 시나리오 진행
}
