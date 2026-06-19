// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MurphyPlayerState.h"

#include "Net/UnrealNetwork.h"

void AMurphyPlayerState::ServerSetArrivalData_Implementation(const FString& InSurname, const FString& InGivenname)
{
	// 서버에서 실행되는 실제 데이터 저장 로직
	SavedSurname = InSurname;
	SavedGivenname = InGivenname;
}

void AMurphyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMurphyPlayerState, SavedSurname);
	DOREPLIFETIME(AMurphyPlayerState, SavedGivenname);
}
