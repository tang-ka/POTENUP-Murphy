// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ArrivalCard/WriteArrivalCardWidget.h"

#include "Components/Button.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Airplane/AirplaneGameMode.h"
#include "UI/ArrivalCard/ArrivalCardWidget.h"

void UWriteArrivalCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 커서/입력모드는 레벨 블루프린트(Airplane)가 위젯 생성 시 전담한다. 여기서 세팅하지 않는다.

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
		
			if (AMurphyPlayerController* MurphyPC = Cast<AMurphyPlayerController>(PC))
			{
				// 주의: TEXT("Item_ArrivalCard") 부분은 해당 퀘스트의 실제 TargetID 이름으로 변경해 주세요.
				MurphyPC->NotifyQuestConditionFromLocal(TEXT("Item_ArrivalCard"), EQuestCondition::GetItem);

				// 서버의 박스 반전은 복제되지 않으므로, 완료한 플레이어가 자기 로컬에서
				// 가장 가까운(=짝) NPC 박스만 직접 반전한다.
				MurphyPC->FlipNearestAirplaneNPCBoxLocal();
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
