// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionInfoWidget.generated.h"

class UTextBlock;
class UCheckBox;

DECLARE_DELEGATE_TwoParams(FOnSessionInfoSelected, USessionInfoWidget*, bool);

UCLASS()
class MURPHY_API USessionInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 세션 정보를 세팅하고 인덱스를 기록한다 */
	void Init(int32 InSessionIndex, const FString& InSessionName, const FString& InHostName, int32 InCurrentPlayers, int32 InMaxPlayers);

	/** 라디오 버튼 용도 — LobbyUI에서 다른 위젯 선택 시 강제 해제에 사용 */
	void SetChecked(bool bChecked);

	int32 GetSessionIndex() const { return SessionIndex; }

	/** 이 위젯의 체크박스가 켜졌을 때 LobbyUI에 알리는 델리게이트 */
	FOnSessionInfoSelected OnSessionInfoSelected;

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_SessionName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_HostName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayerCountInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Tgl_SessionInfo;

private:
	UFUNCTION()
	void HandleCheckStateChanged(bool bIsChecked);

private:
	int32 SessionIndex = INDEX_NONE;
};


