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

	// 시퀀스(전체 영상) 완료 후 기내 NPC 상호작용 박스를 좌우 반전한다.
	UFUNCTION()
	void HandleCinematicComplete();

	// 기내 시나리오 완료 후 설정된 다음 레벨로 모든 플레이어를 이동시킵니다.
	void CompleteScenarioAndTravel();

protected:
	virtual void BeginPlay() override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// 기내 시나리오 완료 후 이동할 LevelStreamingSettings의 레벨 키입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Murphy|Airplane")
	FName NextLevelKeyAfterScenarioComplete = TEXT("Prologue");

private:
	// 음성 종료 콜백 중복 호출로 트래블이 여러 번 요청되는 것을 막습니다.
	bool bScenarioCompleteTravelRequested = false;
};

