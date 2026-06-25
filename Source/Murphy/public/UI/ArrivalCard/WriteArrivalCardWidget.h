// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WriteArrivalCardWidget.generated.h"

class UArrivalCardWidget;
class UButton;
/**
 * 
 */
UCLASS()
class MURPHY_API UWriteArrivalCardWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
private:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_Complete;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UArrivalCardWidget> WBP_ArrivalCard;
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnCompleteClicked();
};
