// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "ArrivalCardWidget.generated.h"

class UEditableText;
class UTextBlock;
/**
 * 
 */
UCLASS()
class MURPHY_API UArrivalCardWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	// Name
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UEditableText> etxt_Surname;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UEditableText> etxt_Givenname;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_SurnameDisplay;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_GivennameDisplay;
	
	// Random Situations
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_VisitLocation;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> txt_CustomsItem;
	
protected:
	virtual void NativeConstruct() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Murphy|ArrivalCard")
	void SetReadOnlyData(const FText& InSurname, const FText& InGivenname);
	
	FText GetSurnameInput() const { return etxt_Surname ? etxt_Surname->GetText() : FText::GetEmpty(); }
	FText GetGivennameInput() const { return etxt_Givenname ? etxt_Givenname->GetText() : FText::GetEmpty(); }
	
private:
	// 텍스트 입력 검사 함수
	UFUNCTION()
	void OnSurnameTextChanged(const FText& Text);
	
	UFUNCTION()
	void OnGivennameTextChanged(const FText& Text);
	
	// AI 데이터가 도착하면 화면을 갱신할 함수
	UFUNCTION()
	void UpdateUI();
	
};
