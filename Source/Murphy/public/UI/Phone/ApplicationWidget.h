// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/PhoneDataTypes.h"
#include "ApplicationWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

// 앱 아이콘 버튼 클릭 시 브로드캐스트 (자기 자신을 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAppIconClicked, UApplicationWidget*, Sender);

UCLASS()
class MURPHY_API UApplicationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_AppIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_AppName;

	// DataManager가 동적으로 생성해 할당하는 앱 화면 위젯
	UPROPERTY()
	TObjectPtr<UUserWidget> AppScreen;

	// 아이콘 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Phone")
	FOnAppIconClicked OnAppIconClicked;

	/** DataManager에서 받아온 Row 데이터와 동적 생성된 AppScreen을 할당 */
	void SetAppData(const FPhoneAppRow& Row, UUserWidget* InAppScreen);

private:
	UFUNCTION()
	void OnIconButtonClicked();

	UFUNCTION()
	void OnIconButtonHovered();

	UFUNCTION()
	void OnIconButtonUnhovered();
};
