// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MurphyGameModeBase.h"
#include "Murphy.h"
#include "Data/GameDataTypes.h"
#include "Data/CinematicSequenceData.h"
#include "Framework/MurphyPlayerState.h"
#include "Manager/CinematicSequenceSubsystem.h"
#include "Engine/GameInstance.h"

UClass* AMurphyGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const AMurphyPlayerState* MurphyPS = InController ? InController->GetPlayerState<AMurphyPlayerState>() : nullptr;
	if (!MurphyPS)
	{
		PRINTLOG_SH(TEXT("GetDefaultPawnClassForController 실패 — PlayerState 없음, 기본 폰으로 대체"));
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	switch (MurphyPS->SelectedCharacter)
	{
	case EPlayerCharacterType::BoyCharacter:
		{
			if (BoyPawnClass)
			{
				PRINTLOG_SH(TEXT("폰 클래스 결정 — Boy"));
				return BoyPawnClass;
			}
			break;
		}
	case EPlayerCharacterType::GirlCharacter:
		{
			if (GirlPawnClass)
			{
				PRINTLOG_SH(TEXT("폰 클래스 결정 — Girl"));
				return GirlPawnClass;
			}
			break;
		}
	default:
		{
			break;
		}
	}

	PRINTLOG_SH(TEXT("GetDefaultPawnClassForController 경고 — 선택값(%s)에 맞는 폰 클래스 미설정, 기본 폰으로 대체"), *UEnum::GetValueAsString(MurphyPS->SelectedCharacter));
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AMurphyGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (bCinematicSequenceStarted || !LevelCinematic)
	{
		return;
	}

	// 필요 인원이 모두 도착했을 때 1회 시작 (RPC가 전원에게 가도록).
	if (GetNumPlayers() < RequiredPlayersToStart)
	{
		return;
	}

	bCinematicSequenceStarted = true;

	if (UCinematicSequenceSubsystem* Seq = GetGameInstance()->GetSubsystem<UCinematicSequenceSubsystem>())
	{
		Seq->StartLevelSequence(LevelCinematic);
	}
}
