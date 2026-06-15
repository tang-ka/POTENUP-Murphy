// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MurphyGameModeBase.h"
#include "AirplaneGameMode.generated.h"

/**
 *
 */
UCLASS()
class MURPHY_API AAirplaneGameMode : public AMurphyGameModeBase
{
	GENERATED_BODY()

public:
	AAirplaneGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};

