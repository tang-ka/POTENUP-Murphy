
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/GameDataTypes.h"
#include "ItemUsableWidgetInterface.generated.h"

/**
 * 아이템 사용하기 버튼으로 열리는 WBP가 반드시 구현해야 하는 Interface.
 *
 * 사용 방법:
 *   1. WBP(블루프린트 또는 C++ UUserWidget 서브클래스)에서 이 Interface를 구현(Implement)합니다.
 *   2. InitFromItemUse 이벤트에서 PlayerState, DataManager 등을 통해 서버 데이터를 읽어 UI를 초기화합니다.
 *
 * 호출 측(ItemDetailWidget::OnUseClicked)은 Implements<>로 안전하게 확인 후 Execute_로 호출합니다.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UItemUsableWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class MURPHY_API IItemUsableWidgetInterface
{
	GENERATED_BODY()

public:
	/**
	 * 사용하기 버튼으로 이 위젯이 열릴 때 호출됩니다.
	 * 파라미터로 어떤 아이템으로 열렸는지(ItemID 등) 식별자를 받습니다.
	 * 서버 런타임 데이터(PlayerState, DataManager 등)는 위젯 내부에서 직접 접근하세요.
	 *
	 * @param ItemInfo - 이 위젯을 열게 한 아이템의 DataTable 정보
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Murphy|Item")
	void InitFromItemUse(const FItemTableRow& ItemInfo);
};
