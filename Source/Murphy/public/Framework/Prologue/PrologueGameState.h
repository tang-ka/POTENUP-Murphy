// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MurphyGameStateBase.h"
#include "PrologueGameState.generated.h"

class AAgentNPCBase;
class AItemBaseActor;
class ATriggerBaseActor;
class ULevel;

/**
 * 프롤로그 시나리오 전용 GameState입니다.
 * 입국심사는 개인 퀘스트, 수화물 수취장은 공유 퀘스트 정책을 같은 베이스에서 처리합니다.
 */
UCLASS()
class MURPHY_API APrologueGameState : public AMurphyGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetBaggageCustomsHoldActorsActive(bool bActive);
	void ApplyBaggageCustomsHoldActorState();

protected:
	UFUNCTION()
	void OnRep_BaggageCustomsHoldActorsActive();

private:
	AActor* FindBaggageCustomsHoldActor(FName ActorTag, const FString& NameFragment, TSubclassOf<AActor> ActorClass) const;
	ULevel* GetBaggageClaimLoadedLevel() const;
	void SetTargetActorActive(AActor* Actor, bool bActive) const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_BaggageCustomsHoldActorsActive, VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	bool bBaggageCustomsHoldActorsActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CarrierActorTag = TEXT("BaggageCustomsHold_Carrier");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CustomsNPCActorTag = TEXT("BaggageCustomsHold_NPC");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CustomsTriggerActorTag = TEXT("BaggageCustomsHold_Trigger");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FString CarrierActorNameFallback = TEXT("BP_Carrier");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FString CustomsNPCActorNameFallback = TEXT("BP_dan");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FString CustomsTriggerActorNameFallback = TEXT("BP_NPCOfficer");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CarrierItemID = TEXT("Item_Carrier");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CarrierQuestTargetID = TEXT("Item_Carrier");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CustomsNPCQuestTargetID = TEXT("NPC_CustomsOfficer");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Murphy|Baggage Claim", meta = (AllowPrivateAccess = "true"))
	FName CustomsTriggerQuestTargetID = TEXT("Zone_CustomsOfficer");
};

