// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhonePopupWidget.generated.h"

/**
 * 
 */
UCLASS()
class MURPHY_API UPhonePopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	// 애니메이션 바인딩 시 Transient 키워드 필수
	UPROPERTY(meta =(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Anim_PhoneSlideUp;
	
	// 열려있는지 여부
	bool bIsOpen = false;
	
public:
	// E 키 호출 함수
	void TogglePhone();
	
	
	
};
