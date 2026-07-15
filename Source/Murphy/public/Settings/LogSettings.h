// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LogSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Log Settings"))
class MURPHY_API ULogSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category="Team Log")
	bool bEnableLog_JW = false;

	UPROPERTY(Config, EditAnywhere, Category="Team Log")
	bool bEnableLog_HJ = false;

	UPROPERTY(Config, EditAnywhere, Category="Team Log")
	bool bEnableLog_SH = false;

	UPROPERTY(Config, EditAnywhere, Category="Team Log")
	bool bEnableLog_CW = true;
};
