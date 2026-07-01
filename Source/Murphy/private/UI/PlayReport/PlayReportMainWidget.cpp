// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayReport/PlayReportMainWidget.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/GameDataTypes.h"
#include "Manager/DataManager.h"
#include "UI/PlayReport/FeedbackMainWidget.h"
#include "UI/PlayReport/PlayReportItemWidget.h" 

void UPlayReportMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_Next)
	{
		btn_Next->OnClicked.AddDynamic(this, &UPlayReportMainWidget::OnNextButtonClicked);
	}
	
	if (panel_Report)
	{
		panel_Report->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	
	if (panel_Feedback)
	{
		panel_Feedback->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = true;
		
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
		PC->SetInputMode(InputMode);
	}
}

void UPlayReportMainWidget::DisplayReport(const FPlayReportData& ReportData)
{
	// TODO: 현진 텍스트 색 수정
	// === 성공 여부 표시 ===
	if (txt_Result)
	{
		if (ReportData.bIsGameClear)
		{
			txt_Result->SetText(FText::FromString(TEXT("COMPLETED")));
			txt_Result->SetColorAndOpacity(FSlateColor(FLinearColor(0.409698f,1.0f,0.247795f)));
		}
		else
		{
			txt_Result->SetText(FText::FromString(TEXT("FAILED")));
			txt_Result->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.055f, 0.026f)));
		}
	}
	
	// === 상단 영역 (총점, 티어) 갱신 ===
	if (txt_TotalScore)
	{
		txt_TotalScore->SetText(FText::AsNumber(ReportData.TotalScore));
	}

	if (txt_TierName)
	{
		txt_TierName->SetText(FText::FromString(ReportData.TierName));
	}
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDataManager* DataManager = GI->GetSubsystem<UDataManager>())
		{
			if (FTierUIDataRow* FoundTierRow = DataManager->GetTierData(FName(*ReportData.TierName)))
			{
				// 칭호 적용
				if (txt_TravelerTitle)
				{
					txt_TravelerTitle->SetText(FText::FromString(FoundTierRow->TierTitle));
				}

				// 티어 이미지 적용
				if (img_Tier && FoundTierRow->TierIcon)
				{
					img_Tier->SetBrushFromTexture(FoundTierRow->TierIcon);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DataManager: 해당 티어(%s)를 찾을 수 없습니다!"), *ReportData.TierName);
			}
		}
	}

	// === 하단 세부 항목 점수 ===
	if (item_Comprehension)
	{
		item_Comprehension->InitializeItem(TEXT("Comprehension"), TEXT("이해력"), ReportData.ComprehensionScore);
	}

	if (item_Fluency)
	{
		item_Fluency->InitializeItem(TEXT("Fluency"), TEXT("유창성"), ReportData.FluencyScore);
	}

	if (item_Grammar)
	{
		item_Grammar->InitializeItem(TEXT("Grammar"), TEXT("문법성"), ReportData.GrammarScore);
	}

	if (item_Vocabulary)
	{
		item_Vocabulary->InitializeItem(TEXT("Vocabulary"), TEXT("어휘력"), ReportData.VocabularyScore);
	}

	if (item_Clarity)
	{
		item_Clarity->InitializeItem(TEXT("Clarity"), TEXT("명확성"), ReportData.ClarityScore);
	}

	if (item_ProblemSolving)
	{
		item_ProblemSolving->InitializeItem(TEXT("Problem Solving"), TEXT("문제해결력"), ReportData.ProblemSolvingScore);
	}
	
	if (WBP_FeedbackMain)
	{
		WBP_FeedbackMain->DisplayFeedback(ReportData);
	}
}

void UPlayReportMainWidget::OnNextButtonClicked()
{
	if (panel_Report)
	{
		panel_Report->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (panel_Feedback)
	{
		panel_Feedback->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}
