// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Session/SessionGameMode.h"

#include "Murphy.h"
#include "Framework/Session/SessionGameState.h"

ASessionGameMode::ASessionGameMode()
{
	GameStateClass = ASessionGameState::StaticClass();
}

void ASessionGameMode::BeginPlay()
{
	Super::BeginPlay();

	PRINTLOG_SH(TEXT("SessionGameMode BeginPlay"));
}

void ASessionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ASessionGameState* GS = GetGameState<ASessionGameState>())
	{
		GS->ConnectedPlayerCount++;
		PRINTLOG_SH(TEXT("플레이어 접속 — 현재 인원: %d"), GS->ConnectedPlayerCount);
	}
}

void ASessionGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (ASessionGameState* GS = GetGameState<ASessionGameState>())
	{
		GS->ConnectedPlayerCount = FMath::Max(0, GS->ConnectedPlayerCount - 1);
		PRINTLOG_SH(TEXT("플레이어 퇴장 — 현재 인원: %d"), GS->ConnectedPlayerCount);
	}
}

