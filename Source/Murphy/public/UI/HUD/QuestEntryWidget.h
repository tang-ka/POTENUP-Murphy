// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Data/GameDataTypes.h"	

#include "QuestEntryWidget.generated.h"

UCLASS()
class MURPHY_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> txt_QuestText;
};
