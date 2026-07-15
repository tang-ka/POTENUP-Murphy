// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Session/SessionGameState.h"

#include "Murphy.h"
#include "Net/UnrealNetwork.h"

void ASessionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASessionGameState, ConnectedPlayerCount);
	DOREPLIFETIME(ASessionGameState, MaxPlayerCount);
	DOREPLIFETIME(ASessionGameState, SessionName);
	DOREPLIFETIME(ASessionGameState, SelectedDestination);
}

void ASessionGameState::OnRep_SelectedDestination()
{
	OnSelectedDestinationChanged.Broadcast();
}

