
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CaptionWidget.generated.h"

class UTextBlock;

UCLASS()
class MURPHY_API UCaptionWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_Caption;			
	
public:
	UFUNCTION(BlueprintCallable)
	void SetCaption(const FString& CaptionText);
	
};
