
#pragma once

#include "CoreMinimal.h"
#include "ItemUsableWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "CarrierItemUI.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class MURPHY_API UCarrierItemUI : public UUserWidget, public IItemUsableWidgetInterface
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UImage> Img_RandomCustom;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> Txt_RandomCustom_K;
	UPROPERTY(meta =(BindWidget))
	TObjectPtr<UTextBlock> Txt_RandomCustom_E;
	
	FName CurItemID;
	
public:
	virtual void InitFromItemUse_Implementation(const FItemTableRow& ItemInfo) override;
};
