// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Airplane/AirplaneGameMode.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Murphy.h"

void AAirplaneGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer == nullptr)
	{
		PRINTLOG_SH(TEXT("HandleStartingNewPlayer: NewPlayer is null"));
		return;
	}

	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(NewPlayer->GetPawn()))
	{
		MurphyPlayer->SetMovementLocked(true);
		PRINTLOG_SH(TEXT("Airplane 진입: 플레이어 이동 입력 잠금"));
	}
	else
	{
		PRINTLOG_SH(TEXT("HandleStartingNewPlayer: MurphyPlayer 캐스팅 실패 (Pawn 없음)"));
	}
}
