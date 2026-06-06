// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AgentEmojiUI.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UAgentEmojiUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	FSlateBrush Brush = Image_EmotionGuage->GetBrush();
	DynMat = UWidgetBlueprintLibrary::GetDynamicMaterial(Brush);
	DynMat->SetScalarParameterValue(TEXT("Progress"), Progress);
	Image_EmotionGuage->SetBrush(Brush);
}

void UAgentEmojiUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UAgentEmojiUI::SetProgress(float ProgressValue)
{
	Progress = ProgressValue;
	
	if (DynMat != nullptr)
	{
		DynMat->SetScalarParameterValue(TEXT("Progress"), Progress);
	}
}

void UAgentEmojiUI::SetProgressColor(FLinearColor Color)
{
	PBColor = Color;
		
	if (DynMat != nullptr)
	{	
		DynMat->SetVectorParameterValue(TEXT("PBColor"), PBColor);	
	}
}

void UAgentEmojiUI::SetEmoji(UTexture2D* Emoji)
{
	Image_Emoji->SetBrushFromTexture(Emoji);
}

void UAgentEmojiUI::SetDialog(FText Dialog)
{
	Text_Dialog->SetText(Dialog);
}

void UAgentEmojiUI::UpdateProgress()
{
}
