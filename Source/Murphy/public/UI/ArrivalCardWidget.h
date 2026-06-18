// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArrivalCardWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class MURPHY_API UArrivalCardWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
private:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_VisitLocation;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_CustomsItem;
	
public:
	virtual void NativeConstruct() override;
	
};
