// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionPlayerStateWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * SessionCharacter 머리 위에 띄우는 닉네임/준비완료 표시 위젯
 */
UCLASS()
class MURPHY_API USessionPlayerStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 플레이어 이름 표시 갱신 */
	void SetPlayerName(const FString& InPlayerName);

	/** 준비 완료 상태 표시 갱신 */
	void SetReadyState(bool bInReady);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayerReady;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ReadyIcon;
};
