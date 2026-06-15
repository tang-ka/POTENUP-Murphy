// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUD.generated.h"

class UPhonePopupWidget;
class UMyMicWidget;
class UQuestPanelWidget;
class UQuestEntryWidget;
class UBagPopupWidget;

UCLASS()
class MURPHY_API UMainHUD : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UMyMicWidget> WBP_MyMic;				// 중앙 하단 마이크 UI
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UQuestPanelWidget> WBP_QuestPanel;	// 퀘스트 전체 레이아웃
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UQuestEntryWidget> WBP_QuestEntry;	// 들어오는 퀘스트 데이터에 따라 업데이트
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UBagPopupWidget> WBP_BagPopup;		// 가방 (기본 닫힘)
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UPhonePopupWidget> WBP_PhonePopup;	// 핸드폰 (기본 닫힘)
	
public:
	virtual void NativeConstruct() override;

	// 토글 요청을 MainHUD로 전달할 때 사용할 인터페이스 함수
	void RequestToggleBag();
	void RequestTogglePhone();
	
	void UpdateMicState(bool bIsRecording);

private:
	// 폰 토글 델리게이트 콜백 — 마우스 커서 활성화/비활성화
	UFUNCTION()
	void HandlePhoneToggled(bool bIsPhoneOpen);

public:
	/** ItemBaseActor에서 가방에 아이템 추가 시 사용 */
	UBagPopupWidget* GetBagPopupWidget() const { return WBP_BagPopup; }
	
};
