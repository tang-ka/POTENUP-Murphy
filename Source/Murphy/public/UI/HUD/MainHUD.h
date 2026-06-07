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
	TObjectPtr<UMyMicWidget> WBP_MyMic;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UQuestPanelWidget> WBP_QuestPanel;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UQuestEntryWidget> WBP_QuestEntry;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UBagPopupWidget> WBP_BagPopup;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UPhonePopupWidget> WBP_PhonePopup;
	
	

public:
	// 가방 토글 요청을 MainHUD로 전달할 때 사용할 인터페이스 함수
	void RequestToggleBag();
	
};
