// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserDialogBlockWidget.generated.h"

class UTextBlock;

UCLASS()
class MURPHY_API UUserDialogBlockWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Time;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Content;

	void SetDialogData(const FText& InTime, const FText& InContent);
};
