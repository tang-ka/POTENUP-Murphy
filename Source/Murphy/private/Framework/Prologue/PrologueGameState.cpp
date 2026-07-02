// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Prologue/PrologueGameState.h"

#include "Actors/Characters/AgentNPCBase.h"
#include "Actors/Items/ItemBaseActor.h"
#include "Actors/Triggers/TriggerBaseActor.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Murphy.h"
#include "Net/UnrealNetwork.h"

void APrologueGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APrologueGameState, bBaggageCustomsHoldActorsActive);
}

void APrologueGameState::SetBaggageCustomsHoldActorsActive(bool bActive)
{
	if (HasAuthority())
	{
		bBaggageCustomsHoldActorsActive = bActive;
		ForceNetUpdate();
	}

	ApplyBaggageCustomsHoldActorState();
}

void APrologueGameState::ApplyBaggageCustomsHoldActorState()
{
	AItemBaseActor* CarrierItem = Cast<AItemBaseActor>(
		FindBaggageCustomsHoldActor(CarrierActorTag, CarrierActorNameFallback, AItemBaseActor::StaticClass()));
	AAgentNPCBase* CustomsNPC = Cast<AAgentNPCBase>(
		FindBaggageCustomsHoldActor(CustomsNPCActorTag, CustomsNPCActorNameFallback, AAgentNPCBase::StaticClass()));
	ATriggerBaseActor* CustomsTrigger = Cast<ATriggerBaseActor>(
		FindBaggageCustomsHoldActor(CustomsTriggerActorTag, CustomsTriggerActorNameFallback, ATriggerBaseActor::StaticClass()));

	if (CarrierItem)
	{
		CarrierItem->ConfigureQuestItem(CarrierItemID, CarrierQuestTargetID);
		SetTargetActorActive(CarrierItem, bBaggageCustomsHoldActorsActive);
	}

	if (CustomsNPC)
	{
		CustomsNPC->SetQuestTargetID(CustomsNPCQuestTargetID);
		CustomsNPC->bIsScenarioCompleted = false;
		SetTargetActorActive(CustomsNPC, bBaggageCustomsHoldActorsActive);
	}

	if (CustomsTrigger)
	{
		CustomsTrigger->SetQuestTargetID(CustomsTriggerQuestTargetID);
		SetTargetActorActive(CustomsTrigger, bBaggageCustomsHoldActorsActive);
	}

	if (bBaggageCustomsHoldActorsActive && (!CarrierItem || !CustomsNPC || !CustomsTrigger))
	{
		PRINTLOGE_JW(
			TEXT("[PrologueGameState] Baggage Customs Hold 활성화 대상 누락. Carrier:%s NPC:%s Trigger:%s"),
			CarrierItem ? TEXT("OK") : TEXT("Missing"),
			CustomsNPC ? TEXT("OK") : TEXT("Missing"),
			CustomsTrigger ? TEXT("OK") : TEXT("Missing"));
	}
}

void APrologueGameState::OnRep_BaggageCustomsHoldActorsActive()
{
	ApplyBaggageCustomsHoldActorState();
}

AActor* APrologueGameState::FindBaggageCustomsHoldActor(FName ActorTag, const FString& NameFragment, TSubclassOf<AActor> ActorClass) const
{
	if (!GetWorld() || !ActorClass)
	{
		return nullptr;
	}

	ULevel* BaggageLevel = GetBaggageClaimLoadedLevel();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!IsValid(Actor) || (BaggageLevel && Actor->GetLevel() != BaggageLevel))
		{
			continue;
		}

		if (!ActorTag.IsNone() && Actor->ActorHasTag(ActorTag))
		{
			return Actor;
		}
	}

	for (AActor* Actor : FoundActors)
	{
		if (!IsValid(Actor) || (BaggageLevel && Actor->GetLevel() != BaggageLevel) || NameFragment.IsEmpty())
		{
			continue;
		}

		if (Actor->GetName().Contains(NameFragment)
			|| (Actor->GetClass() && Actor->GetClass()->GetName().Contains(NameFragment)))
		{
			return Actor;
		}
	}

	return nullptr;
}

ULevel* APrologueGameState::GetBaggageClaimLoadedLevel() const
{
	UGameInstance* GameInstance = GetGameInstance();
	ULevelStreamingSubsystem* LevelSubsystem = GameInstance ? GameInstance->GetSubsystem<ULevelStreamingSubsystem>() : nullptr;
	if (!LevelSubsystem)
	{
		return nullptr;
	}

	ULevelStreaming* BaggageLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_BaggageClaim"));
	return BaggageLevel ? BaggageLevel->GetLoadedLevel() : nullptr;
}

void APrologueGameState::SetTargetActorActive(AActor* Actor, bool bActive) const
{
	if (!IsValid(Actor))
	{
		return;
	}

	Actor->SetActorHiddenInGame(!bActive);
	Actor->SetActorEnableCollision(bActive);
	Actor->SetActorTickEnabled(bActive);
}
