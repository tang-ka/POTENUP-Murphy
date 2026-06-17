// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Animation/WidgetAnimation.h"
#include "LobbyUI.generated.h"

class UOverlay;
class UButton;
class UCanvasPanel;
class UVerticalBox;
class UEditableTextBox;
class USessionInfoWidget;
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
#pragma region LobbyUI
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> Btn_SinglePlay;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> Btn_MultiPlay;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> Btn_Achievement;
	
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UButton> Btn_Option;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quit;
#pragma endregion

#pragma region SessionUI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> Panel_Session;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_SessionList;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_RefreshSessionList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_ToggleSessionSetting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_JoinSession;
#pragma endregion
	
#pragma region SessionInfoSettingUI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> Panel_SessionInfoSetting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_SessionName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_HostName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_CreateSession;
#pragma endregion

#pragma region LoadingUI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Ovl_Loading;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Loading;
#pragma endregion
	
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
	
	UFUNCTION()
	void OnRefreshSessionListButtonClicked();

	UFUNCTION()
	void OnToggleSessionSettingButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnJoinSessionButtonClicked();

	UFUNCTION()
	void OnCreateSessionButtonClicked();

private:
	void HandleFindSessionsComplete(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& Results);
	void OnSessionSelected(USessionInfoWidget* SelectedWidget);
	void ClearSessionList();

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USessionInfoWidget> SessionInfoWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<USessionInfoWidget>> SessionWidgetList;

	int32 SelectedSessionIndex = INDEX_NONE;
};
