// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelStreamingSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MURPHY_API ULevelStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void TravelAllPlayers(FName LevelKey);
	
private:
	const TMap<FName, TSoftObjectPtr<UWorld>>& GetLevelMap() const;
	
};
