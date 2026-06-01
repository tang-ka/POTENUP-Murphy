// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PrologueGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MURPHY_API APrologueGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable)
	void TransitionToBaggageClaim();
	
private:
	UFUNCTION()
	void OnImmigrationLevelLoaded();
	
	UFUNCTION()
	void OnImmigrationLevelShown();
	
	UFUNCTION()
	void OnImmigrationLevelHidden();

	UFUNCTION()
	void OnBaggageClaimLevelShown();
};
