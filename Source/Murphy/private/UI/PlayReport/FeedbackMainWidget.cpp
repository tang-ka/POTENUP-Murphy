// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayReport/FeedbackMainWidget.h"

#include "Murphy.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Data/PlayReportData.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/NetworkManagerSubsystem.h"
#include "UI/PlayReport/FeedbackCardWidget.h"

void UFeedbackMainWidget::DisplayFeedback(const FPlayReportData& ReportData)
{
	// === 텍스트 위젯 데이터 바인딩 ===
	// if (txt_Overall)
	// {
	// 	txt_Overall->SetText(FText::FromString(ReportData.OverallSummaryKr)); 
	// }

	if (txt_MainImprovement)
	{
		txt_MainImprovement->SetText(FText::FromString(ReportData.MainImprovement));
	}

	// if (txt_NextPracticePrompt)
	// {
	// 	txt_NextPracticePrompt->SetText(FText::FromString(ReportData.NextPracticePromptKr));
	// }
// 
	// if (txt_NextAnswerExample)
	// {
	// 	txt_NextAnswerExample->SetText(FText::FromString(ReportData.NextAnswerExample));
	// }
	
	// === 피드백 카드 동적 생성 및 부착 ===
	if (sb_FeedbackCards)
	{
		// 1. 기존 카드 초기화 (위젯 재활용 시 겹침 방지)
		sb_FeedbackCards->ClearChildren();

		// 2. 블루프린트에서 클래스가 제대로 할당되었는지 체크
		if (FeedbackCardClass)
		{
			// 3. 배열을 순회하며 카드 생성
			for (const FPlayReportFeedbackCardData& CardData : ReportData.FeedbackCards)
			{
				UFeedbackCardWidget* NewCard = CreateWidget<UFeedbackCardWidget>(this, FeedbackCardClass);
                
				if (NewCard)
				{
					// 카드의 내부 세팅 함수 호출
					NewCard->InitializeCard(CardData);
                    
					// 스크롤 박스에 추가
					sb_FeedbackCards->AddChild(NewCard);
				}
			}
		}
		else
		{
			PRINTLOG_HJ(TEXT("FeedbackCardClass가 설정되지 않았습니다! FeedbackMainWidget BP를 확인하세요."));
		}
	}
}

void UFeedbackMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_GoMain)
	{
		btn_GoMain->OnClicked.AddDynamic(this, &UFeedbackMainWidget::HandleBtnGoMainClicked);
	}
}

void UFeedbackMainWidget::HandleBtnGoMainClicked()
{
	APlayerController* OwningPC = GetOwningPlayer();
	if (!OwningPC)
	{
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = OwningPC->GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		return;
	}

	// 싱글플레이는 세션이 없으므로 메인화면(로비)으로 직접 이동
	if (NetworkManager->GetSessionState() != ESessionState::InSession)
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/Lv_Lobby")));
		return;
	}

	// 멀티플레이: 세션 종료 후 로비 복귀
	NetworkManager->DestroySession();
}
