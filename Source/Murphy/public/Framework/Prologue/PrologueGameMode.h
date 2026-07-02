// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "Framework/MurphyGameModeBase.h"
#include "PrologueGameMode.generated.h"

UCLASS()
class MURPHY_API APrologueGameMode : public AMurphyGameModeBase
{
	GENERATED_BODY()
	
public:
	APrologueGameMode();

	void HandleAINodeReached(FName NodeId);

protected:
	virtual void BeginPlay() override;
	
private:
	// 서버에서 Immigration 레벨이 보이면 모든 PC에 Pawn 스폰
	UFUNCTION()
	void OnImmigrationLevelShown();

	UFUNCTION()
	void OnBaggageClaimLevelShown();

	void StartScenarioIfNeeded(EScenarioType ScenarioType);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName BaggageCustomsHoldNodeId = TEXT("BAG_004_STAFF_REDIRECT_TO_CUSTOMS_HOLD");
};
