// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RandomDataTypes.generated.h"

/**
 * 
 */

// 방문 장소 (공통 속성)
USTRUCT(BlueprintType)
struct FLocationTextData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NameEN;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NameKR;
};

// 신고 물품 (공통 속성 상속, 텍스쳐 추가)
USTRUCT(BlueprintType)
struct FCustomsItemTextData : public FLocationTextData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemTexture;
	
	
};


