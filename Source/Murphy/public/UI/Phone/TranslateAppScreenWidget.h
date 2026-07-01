// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/TranslateTypes.h"
#include "TranslateAppScreenWidget.generated.h"

class UScrollBox;
class UHorizontalBox;
class UVerticalBox;
class UImage;
class UWidgetAnimation;
class UUserDialogBlockWidget;
class UAgentDialogBlockWidget;
class UScenarioCategoryButtonWidget;
class UTranslateDialogManager;

UCLASS()
class MURPHY_API UTranslateAppScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HB_ScenarioCategory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> Scroll_Dialog;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_Dialog;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Connecting;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Connecting;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Translate")
	TSubclassOf<UUserDialogBlockWidget> UserDialogBlockClass;

	UPROPERTY(EditDefaultsOnly, Category = "Translate")
	TSubclassOf<UAgentDialogBlockWidget> AgentDialogBlockClass;

	UPROPERTY(EditDefaultsOnly, Category = "Translate")
	TSubclassOf<UScenarioCategoryButtonWidget> ScenarioCategoryButtonClass;

	UPROPERTY(EditDefaultsOnly, Category = "Translate")
	FLinearColor ConnectingColor = FLinearColor(0.f, 1.f, 0.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Translate")
	FLinearColor DisconnectedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);

public:
	void InitializeWithManager(UTranslateDialogManager* InDialogManager);

	UFUNCTION()
	void HandleDialogAdded(FName InCategoryName, const FDialogEntry& Entry);

	UFUNCTION()
	void HandleCategoryAdded(FName InCategoryName, const FText& DisplayName);

	void SetConnecting(bool bIsConnecting);

private:
	UPROPERTY()
	TObjectPtr<UTranslateDialogManager> DialogManager;

	UPROPERTY()
	TArray<TObjectPtr<UUserDialogBlockWidget>> UserDialogWidgetPool;

	UPROPERTY()
	TArray<TObjectPtr<UAgentDialogBlockWidget>> AgentDialogWidgetPool;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> ActiveDialogWidgets;

	UPROPERTY()
	TMap<FName, TObjectPtr<UScenarioCategoryButtonWidget>> CategoryButtonMap;

	FName CurrentCategory;

	UUserWidget* AcquireDialogWidget(const FDialogEntry& Entry);
	UUserDialogBlockWidget* AcquireUserDialogWidget();
	UAgentDialogBlockWidget* AcquireAgentDialogWidget();
	UScenarioCategoryButtonWidget* AddCategoryButton(FName InCategoryName, const FText& DisplayName);
	void ApplyDialogData(UUserWidget* DialogWidget, const FDialogEntry& Entry);
	void HideActiveDialogWidgets();
	void RefreshDialog(FName InCategoryName);
	void AppendDialog(const FDialogEntry& Entry);
	void SyncExistingCategories();
	void UpdateCategoryButtonSelection(FName InSelectedCategoryName);

	UFUNCTION()
	void OnCategoryButtonSelected(FName InCategoryName);
};
