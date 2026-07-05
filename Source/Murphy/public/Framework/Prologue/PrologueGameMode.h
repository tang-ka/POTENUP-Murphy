// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "Framework/MurphyGameModeBase.h"
#include "TimerManager.h"
#include "PrologueGameMode.generated.h"

class APlayerController;

UCLASS()
class MURPHY_API APrologueGameMode : public AMurphyGameModeBase
{
	GENERATED_BODY()
	
public:
	APrologueGameMode();

	void HandleAINodeReached(FName NodeId);

	void NotifyImmigrationLevelReady(APlayerController* ReadyPlayer);
	void NotifyBaggageClaimLevelReady(APlayerController* ReadyPlayer);
	
protected:
	virtual void BeginPlay() override;

	// 인트로 시네마틱 완료 통보 시 Immigration 시나리오 시작을 결정한다.
	virtual void NotifyIntroCinematicFinished() override;

private:
	// 서버에서 Immigration 레벨이 보이면 모든 PC에 Pawn 스폰
	UFUNCTION()
	void OnImmigrationLevelShown();

	UFUNCTION()
	void OnBaggageClaimLevelShown();

	void StartScenarioIfNeeded(EScenarioType ScenarioType);
	void StartImmigrationScenarioAfterCinematic();
	float GetImmigrationScenarioStartDelay() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Immigration", meta = (AllowPrivateAccess = "true"))
	bool bRequireImmigrationCinematicBeforeScenario = true;

	FTimerHandle ImmigrationScenarioStartTimerHandle;

	// 첫 클라 완료 통보로만 1회 시나리오 시작하도록 막는 가드
	bool bImmigrationScenarioStartRequested = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName BaggageCustomsHoldNodeId = TEXT("BAG_004_STAFF_REDIRECT_TO_CUSTOMS_HOLD");
};
