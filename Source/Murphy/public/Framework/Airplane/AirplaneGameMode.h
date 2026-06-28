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

	// 시퀀스(전체 영상) 완료 후 기내 NPC 상호작용 박스를 좌우 반전한다.
	UFUNCTION()
	void HandleCinematicComplete();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};

