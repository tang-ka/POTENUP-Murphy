
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GameDataTypes.h"
#include "ItemWidget.generated.h"

class UTextBlock;
class UButton;
class UItemDetailWidget;

/**
 * 가방(BagPopupWidget) 내 개별 아이템 슬롯 위젯
 * 클릭 시 상세 팝업을 열어 CheckItem을 통보하고, 사용 버튼으로 UseItem을 통보합니다.
 */
UCLASS()
class MURPHY_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	/**
	 * 가방에 아이템 추가 시 호출하여 데이터 초기화
	 * @param Info - 표시할 아이템 정보
	 */
	void InitItem(const FItemTableRow& Info);
	
	/** 아이템 버튼 클릭 시 상세 팝업을 엽니다. */
	UFUNCTION()
	void OnItemButtonClicked();

	/** UI 버튼 경로에서 아이템 사용과 퀘스트 통보를 함께 처리합니다. */
	UFUNCTION(BlueprintCallable, Category = "Murphy|Item")
	void UseItemFromUI();
	
	/**
	 * 아이템 사용 처리 (즉시 사용 경로 A, 팝업 버튼 경로 B 공용)
	 * 실제 사용 로직은 블루프린트에서 오버라이드 가능
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Murphy|Item")
	void UseItem();
	virtual void UseItem_Implementation();

	/** 현재 보유 중인 아이템 정보 반환 */
	UFUNCTION(BlueprintPure, Category = "Murphy|Item")
	const FItemTableRow& GetItemInfo() const { return ItemData; }

protected:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Item;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Txt_ItemName;
	
private:
	/** 현재 아이템 데이터 */
	FItemTableRow ItemData;

	/** 상세 팝업 클래스 (에디터에서 WBP 지정) */
	UPROPERTY(EditAnywhere, Category = "Murphy|Item")
	TSubclassOf<UItemDetailWidget> ItemDetailWidgetClass;

	/** 상세 팝업 표시 (B 경로) */
	void ShowDetailPopup();

	/** Bag 아이템 조작을 PlayerController의 QuestEventNotifier에 통보합니다. */
	void NotifyQuestCondition(EQuestClearCondition Condition) const;
};
