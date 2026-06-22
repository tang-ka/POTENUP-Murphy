// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GameDataTypes.h"
#include "SessionMainHUDWidget.generated.h"

class UHorizontalBox;
class UButton;
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnCharacterSelected, EPlayerCharacterType);
DECLARE_DELEGATE_OneParam(FOnReadyRequested, bool);
DECLARE_DELEGATE(FOnStartRequested);

UCLASS()
class MURPHY_API USessionMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Host 여부에 따라 Btn_Start / Btn_Ready 노출 전환 */
	void SetPlayerRole(bool bIsHost);

	/** 캐릭터 선택 시 외부에서 구독 */
	FOnCharacterSelected OnCharacterSelected;

	/** Btn_Ready 클릭 시 외부에서 구독 (Guest 전용) */
	FOnReadyRequested OnReadyRequested;

	/** Btn_Start 클릭 시 외부에서 구독 (Host 전용) */
	FOnStartRequested OnStartRequested;

protected:
	virtual void NativeConstruct() override;

#pragma region SelectCharacter
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HB_CharacterList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Boy;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Girl;
#pragma endregion

#pragma region SelectPlace
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlaceName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Start;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Start;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Ready;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Ready;
#pragma endregion
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Exit;

private:
	bool bIsReady = false;

	UFUNCTION()
	void HandleBtnBoyClicked();

	UFUNCTION()
	void HandleBtnGirlClicked();

	UFUNCTION()
	void HandleBtnReadyClicked();

	UFUNCTION()
	void HandleBtnStartClicked();

	UFUNCTION()
	void HandleBtnExitClicked();
};
