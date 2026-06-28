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
	void SetCaptionInteractionActive(bool bIsActive);
	void ClearCaption();

	UBagPopupWidget* GetBagPopupWidget() const { return WBP_BagPopup; }

#pragma region Test
	/** 외부 진입점: 대화 추가 */
	void AddTranslateDialog(FName InCategoryName, const FDialogEntry& Entry);

	/** 외부 진입점: 카테고리 추가 */
	void AddTranslateCategory(FName InCategoryName, const FText& DisplayName);

	/** 외부 진입점: 연결 상태 */
	void SetTranslateConnecting(bool bIsConnecting);
#pragma endregion

	/** 대화 시작: 활성 카테고리 캐싱 + 카테고리 등록 */
	void BeginTranslateConversation(FName InCategoryName, const FText& InDisplayName);

	/** Agent 대사 블록 추가 (활성 카테고리에) */
	void AddAgentDialog(const FString& InSpeaker, const FString& InText);

	/** User 대사 블록 추가 (활성 카테고리에) */
	void AddUserDialog(const FString& InText);

private:
	UPROPERTY()
	TObjectPtr<UTranslateDialogManager> DialogManager;

	// BeginTranslateConversation에서 설정되는 현재 진행 중인 대화의 카테고리
	FName ActiveTranslateCategory;

	// NPC 상호작용 중일 때만 플레이어 자막을 화면에 표시합니다.
	bool bCaptionInteractionActive = false;

	UFUNCTION()
	void HandlePhoneToggled(bool bIsPhoneOpen);

	// NativeConstruct에서 바인딩 후 SetConnecting 용도로만 사용
	UTranslateAppScreenWidget* GetTranslateScreen() const;
};
