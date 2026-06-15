// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SystemMenuUI.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class MURPHY_API USystemMenuUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
protected:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_ExitGame;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_Option;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_Quit;
	
	
protected:
	UFUNCTION()
	void OnExitGameButtonClicked();
	
	UFUNCTION()
	void OnOptionButtonClicked();
	
	UFUNCTION()
	void OnQuitButtonClicked();
};
