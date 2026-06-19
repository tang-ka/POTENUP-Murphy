// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MurphyPlayerState.h"

#include "Net/UnrealNetwork.h"

void AMurphyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMurphyPlayerState, SelectedCharacter);
	DOREPLIFETIME(AMurphyPlayerState, bIsReady);
	DOREPLIFETIME(AMurphyPlayerState, bIsHost);
}

void AMurphyPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	
	// SeamlessTravel 시 인게임 PS로 선택값 이관 (bIsReady는 룸 전용이라 제외)
	if (AMurphyPlayerState* NewPS = Cast<AMurphyPlayerState>(PlayerState))
	{
		NewPS->SelectedCharacter = SelectedCharacter;
		NewPS->bIsHost = bIsHost;
	}
}

void AMurphyPlayerState::OnRep_SessionRoomState()
{
	OnSessionRoomStateChanged.Broadcast();
}
