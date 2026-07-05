// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MurphyGameModeBase.h"
#include "Murphy.h"
#include "Data/GameDataTypes.h"
#include "Data/CinematicSequenceData.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/MurphyGameStateBase.h"
#include "Manager/CinematicSequenceSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

void AMurphyGameModeBase::InitGameState()
{
	Super::InitGameState();

	if (AMurphyGameStateBase* MurphyGameState = GetGameState<AMurphyGameStateBase>())
	{
		MurphyGameState->SetChatViewMode(ChatViewMode);
	}
}

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

AActor* AMurphyGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	const AMurphyPlayerState* MurphyPS = Player ? Player->GetPlayerState<AMurphyPlayerState>() : nullptr;
	if (!MurphyPS)
	{
		PRINTLOG_SH(TEXT("ChoosePlayerStart — PlayerState 없음, 기본 선택으로 폴백"));
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	FName TargetTag = NAME_None;
	switch (MurphyPS->SelectedCharacter)
	{
	case EPlayerCharacterType::BoyCharacter:
		TargetTag = FName(TEXT("Boy"));
		break;
	case EPlayerCharacterType::GirlCharacter:
		TargetTag = FName(TEXT("Girl"));
		break;
	default:
		break;
	}

	if (TargetTag != NAME_None)
	{
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			if (It->PlayerStartTag == TargetTag)
			{
				PRINTLOG_SH(TEXT("ChoosePlayerStart — 태그(%s) PlayerStart 선택"), *TargetTag.ToString());
				return *It;
			}
		}
		PRINTLOG_SH(TEXT("ChoosePlayerStart — 태그(%s)에 맞는 PlayerStart 없음, 기본 선택으로 폴백"), *TargetTag.ToString());
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AMurphyGameModeBase::TriggerGameOver()
{
	if (AMurphyGameStateBase* GS = GetGameState<AMurphyGameStateBase>())
	{
		GS->SetGameResultState(EGameResultState::GameOver);
		PRINTLOG_SH(TEXT("TriggerGameOver: Game State를 GameOver로 변경했습니다."));
	}
}

void AMurphyGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// 레벨 인트로 시네마틱은 각 머신의 GameState(TryPlayLevelIntro)가 로컬 재생한다.
}

void AMurphyGameModeBase::NotifyIntroCinematicFinished()
{
	// 베이스 기본 동작 없음. 파생 GameMode가 시나리오 시작을 구현한다.
}
