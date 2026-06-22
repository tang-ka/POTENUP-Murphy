
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GameDataTypes.h"
#include "ItemDetailWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UItemWidget;

/**
 * 가방에서 아이템 클릭 시 화면 중앙에 표시되는 아이템 상세 팝업
 * bIsUsable 여부와 관계없이 "사용" 버튼 항상 노출 (비활성화 처리는 WBP에서)
 */
UCLASS()
class MURPHY_API UItemDetailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 팝업을 열기 전 아이템 데이터와 호출한 ItemWidget을 초기화
	 * @param Info - 표시할 아이템 정보
	 * @param InOwnerWidget - 이 팝업을 띄운 ItemWidget (UseItem 호출 대리자)
	 */
	void InitDetail(const FItemTableRow& Info, UItemWidget* InOwnerWidget);

protected:
	virtual void NativeConstruct() override;

	// === WBP Binding Widget ===
	/** 아이템 이름 텍스트 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Img_ItemImage;
	
	/** 아이템 이름 텍스트 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_ItemName;

	/** 아이템 설명 텍스트 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_ItemDescription;

	/** 사용 버튼 (bIsUsable=false 이면 비활성화 상태로 표시) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Btn_Use;

	/** 닫기 버튼 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Btn_Close;

public:
	/** 사용하기 버튼을 누르면 띄울 위젯 */ // 1. 임국심사서 -> ArrivalCardWidget, 2. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> UseWidget;
	
private:
	/** 이 팝업을 띄운 ItemWidget - 사용 처리 위임 */
	UPROPERTY()
	TObjectPtr<UItemWidget> OwnerItemWidget;

	/** 현재 표시 중인 아이템 정보 */
	FItemTableRow CachedItemInfo;

	/** 사용 버튼 클릭 콜백 */
	UFUNCTION()
	void OnUseClicked();

	/** 닫기 버튼 클릭 콜백 */
	UFUNCTION()
	void OnCloseClicked();
};
