
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PhoneDataTypes.generated.h"

USTRUCT(BlueprintType)
struct FPhoneAppRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "App")
	FText AppName;

	// 에디터에서 picker로 선택 → 레퍼런스 추적 정상
	UPROPERTY(EditAnywhere, Category = "App")
	TSoftObjectPtr<UTexture2D> AppIcon;

	// 비어있으면 가짜 앱
	UPROPERTY(EditAnywhere, Category = "App")
	TSoftClassPtr<UUserWidget> AppScreenWidgetClass;
};