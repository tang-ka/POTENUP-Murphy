// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUI.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class MURPHY_API ULobbyUI : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	virtual void NativeConstruct() override;
	
protected:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_SinglePlay;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_MultiPlay;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_Achievement;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_Option;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> btn_Quit;
	
protected:
	UFUNCTION()
	void OnSinglePlayButtonClicked();
	
	UFUNCTION()
	void OnMultiPlayButtonClicked();
	
	UFUNCTION()
	void OnAchievementButtonClicked();
	
	UFUNCTION()
	void OnOptionButtonClicked();
	
	UFUNCTION()
	void OnQuitButtonClicked();
	
};
