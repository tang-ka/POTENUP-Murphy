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

void UAgentEmojiUI::SetEmoji(UTexture2D* Emoji) const
{
	Image_Emoji->SetBrushFromTexture(Emoji);
}

void UAgentEmojiUI::SetNPCName(FString NPCName) const
{
	Text_NPCName->SetVisibility(ESlateVisibility::HitTestInvisible); // 강제로 보이게 켬
	Text_NPCName->SetText(FText::FromString(NPCName));
	
	// 화면에 정말 이 함수가 불렸는지 빨간 글씨로 띄워봅니다.
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("SetNPCName Called: %s"), *NPCName));
}

void UAgentEmojiUI::SetEmojiVisible(bool bIsVisible)
{
	ESlateVisibility V = ESlateVisibility::Visible;
	if (bIsVisible == false) V = ESlateVisibility::Hidden; 
	
	Image_Emoji->SetVisibility(V);
	Image_EmotionGuage->SetVisibility(V);
}
