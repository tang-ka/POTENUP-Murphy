// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GameDataTypes.h"
#include "BagPopupWidget.generated.h"

class UWrapBox;
class UWidgetAnimation;
class UItemWidget;

/**
 * 가방 팝업 위젯
 * 아이템 목록을 ScrollBox로 관리하며, 열릴 때 마우스 커서를 활성화합니다.
 */
UCLASS()
class MURPHY_API UBagPopupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 애니메이션 바인딩 시 Transient 키워드 필수
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Anim_BagSlideUp;

	/** 아이템 목록을 표시할 스크롤 박스 (WBP에서 바인딩) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> Wbx_Items;

	/** 아이템 위젯 클래스 (에디터에서 WBP 지정) */
	UPROPERTY(EditAnywhere, Category = "Murphy|Bag")
	TSubclassOf<UItemWidget> ItemWidgetClass;

	// 열려있는지 여부
	bool bIsOpen = false;

public:
	/** Q 키 호출 함수 - 애니메이션 + 마우스 커서 처리 */
	void ToggleBag();

	/**
	 * ItemBaseActor에서 아이템 획득 시 호출
	 * @param Item - 추가할 아이템 정보
	 */
	void AddItem(const FItemTableRow& Item);

	/** Bag UI에 해당 아이템이 이미 표시 중인지 확인합니다. */
	bool HasItem(FName ItemID) const;

	/** 이미 보유 중이면 추가하지 않고, 없을 때만 Bag UI에 추가합니다. */
	bool AddItemIfMissing(const FItemTableRow& Item);

private:
	/** 마우스 커서 및 입력 모드 설정 */
	void SetMouseCursorEnabled(bool bEnabled);
};

