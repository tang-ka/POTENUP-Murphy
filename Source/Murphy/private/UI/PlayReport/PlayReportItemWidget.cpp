// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayReport/PlayReportItemWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

FLinearColor UPlayReportItemWidget::GetColorByScore(int32 Score) const
{
	// 점수 구간별 색상 판별
	if (Score >= 100) return PerfectColor;
	if (Score >= 90)  return ExcellentColor;
	if (Score >= 70)  return GoodColor;
	if (Score >= 50)  return NormalColor;
    
	return PoorColor;
}

void UPlayReportItemWidget::InitializeItem(const FString& Title, int32 Score)
{
	// UI 데이터 및 시각 효과 갱신
	
	// 1. 텍스트 데이터 세팅
	if (txt_CategoryTitle)
	{
		txt_CategoryTitle->SetText(FText::FromString(Title));
	}
	
	if (txt_Score)
	{
		txt_Score->SetText(FText::FromString(FString::Printf(TEXT("%d"), Score)));
	}
	
	// 2. 현재 점수에 맞는 색상 추출
	FLinearColor CurrentThemeColor = GetColorByScore(Score);
	
	// 3. 원형 ProgressBar 머티리얼 제어
	if (img_CircleProgress)
	{
		if (!DynamicProgressMaterial)
		{
			// 동적 머티리얼 인스턴스가 아직 생성되지 않았다면 1회 생성
			DynamicProgressMaterial = img_CircleProgress->GetDynamicMaterial();
		}
		
		// 머티리얼에 0.0~1.0 범위의 비율과 추출한 색상 주입
		if (DynamicProgressMaterial)
		{
			float PercentValue = FMath::Clamp(Score / 100.0f, 0.0f, 1.0f);
			
			DynamicProgressMaterial->SetScalarParameterValue(FName("Progress"), PercentValue);
			// DynamicProgressMaterial->SetScalarParameterValue(FName("PBColor"), );
			
		}
		
	}
	
	
}
