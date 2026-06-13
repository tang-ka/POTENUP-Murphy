// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhonePopupWidget.generated.h"

class UWidgetSwitcher;
class UTextBlock;
class UButton;
class UApplicationWidget;

// 폰이 열리거나 닫힐 때 발생하는 델리게이트 (true = 열림, false = 닫힘)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhoneToggled, bool, bIsOpen);

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

	virtual void NativeConstruct() override;
	
public:
	// 폰이 열리거나 닫힐 때 브로드캐스트되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Phone")
	FOnPhoneToggled OnPhoneToggled;

	// E 키 호출 함수
	void TogglePhone();
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Time;

	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UButton> Btn_Search;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Call;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UApplicationWidget> WBP_Travelgram;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UApplicationWidget> WBP_Translate;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UApplicationWidget> WBP_Camera;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UApplicationWidget> WBP_Photos;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> AppScreenSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Home;

private:
	// 앱 아이콘 클릭 시 AppScreenSwitcher 전환
	UFUNCTION()
	void HandleAppIconClicked(UApplicationWidget* ClickedApp);

	UFUNCTION()
	void HandleCallClicked();
	
	// Home 버튼 클릭 시 AppScreenSwitcher 닫기
	UFUNCTION()
	void HandleHomeClicked();

	// 앱 위젯 한 개를 DataManager에서 초기화하는 헬퍼
	void InitAppWidget(UApplicationWidget* Widget, const FName& RowName);

	// Btn_Call을 DataManager에서 초기화하는 헬퍼
	void InitCallWidget();

	// AppScreenSwitcher 표시 + 지정된 화면으로 전환하는 공통 헬퍼
	void ShowAppScreen(UUserWidget* TargetScreen);

	// DataManager에서 생성한 Call 앱 화면
	UPROPERTY()
	TObjectPtr<UUserWidget> CallAppScreen;
};
