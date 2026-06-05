// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "QuestEntryWidget.generated.h"

// 퀘스트 타입 Enum
UENUM(BlueprintType)
enum class EQuestType : uint8
{
	MainQuest,
	SubQuest
};


UCLASS()
class MURPHY_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta =(BindWidget))
	UTextBlock* txt_QuestText;
	
	
	
	
	
};
