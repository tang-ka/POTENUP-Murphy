
#pragma once

#include "CoreMinimal.h"
#include "ItemUsableWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "CarrierItemUI.generated.h"

class UButton;
class UImage;
class UTextBlock;

UCLASS()
class MURPHY_API UCarrierItemUI : public UUserWidget, public IItemUsableWidgetInterface
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Img_RandomCustom;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_RandomCustom_K;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_RandomCustom_E;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Btn_Close;
	
	FName CurItemID;
	
	UFUNCTION()
	void OnCloseClicked();
	
public:
	virtual void InitFromItemUse_Implementation(const FItemTableRow& ItemInfo) override;

};
