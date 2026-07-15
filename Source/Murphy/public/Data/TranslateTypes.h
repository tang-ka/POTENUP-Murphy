// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TranslateTypes.generated.h"

UENUM(BlueprintType)
enum class EDialogType : uint8
{
	User,
	Agent
};

USTRUCT(BlueprintType)
struct FDialogEntry
{
	GENERATED_BODY()

	UPROPERTY()
	EDialogType Type = EDialogType::User;

	UPROPERTY()
	FText Name;     // Agent만 사용

	UPROPERTY()
	FText Time;

	UPROPERTY()
	FText Content;
};
