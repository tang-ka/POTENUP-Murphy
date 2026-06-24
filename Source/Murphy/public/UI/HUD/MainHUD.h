// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/TranslateTypes.h"
#include "MainHUD.generated.h"

class UCaptionWidget;
class UPhonePopupWidget;
class UMyMicWidget;
class UQuestPanelWidget;
class UQuestEntryWidget;
class UBagPopupWidget;
class UTranslateDialogManager;
class UTranslateAppScreenWidget;

UCLASS()
class MURPHY_API UMainHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMyMicWidget> WBP_MyMic;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UQuestPanelWidget> WBP_QuestPanel;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UQuestEntryWidget> WBP_QuestEntry;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBagPopupWidget> WBP_BagPopup;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPhonePopupWidget> WBP_PhonePopup;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCaptionWidget> WBP_PlayerCaption;

public:
	virtual void NativeConstruct() override;

	void RequestToggleBag();
	void RequestTogglePhone();
	void UpdateMicState(bool bIsRecording);
	void UpdateCaption(const FString& CaptionText);

	UBagPopupWidget* GetBagPopupWidget() const { return WBP_BagPopup; }

	/** 외부 진입점: 대화 추가 */
	void AddTranslateDialog(FName InCategoryName, const FDialogEntry& Entry);

	/** 외부 진입점: 카테고리 추가 */
	void AddTranslateCategory(FName InCategoryName, const FText& DisplayName);

	/** 외부 진입점: 연결 상태 */
	void SetTranslateConnecting(bool bIsConnecting);

private:
	UPROPERTY()
	TObjectPtr<UTranslateDialogManager> DialogManager;

	UFUNCTION()
	void HandlePhoneToggled(bool bIsPhoneOpen);

	// NativeConstruct에서 바인딩 후 SetConnecting 용도로만 사용
	UTranslateAppScreenWidget* GetTranslateScreen() const;
};
