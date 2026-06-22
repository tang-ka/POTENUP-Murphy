// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Airplane/AirplaneGameMode.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Framework/Airplane/AirplaneGameState.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Murphy.h"
#include "Data/CinematicTypes.h"
#include "MediaSource.h"

AAirplaneGameMode::AAirplaneGameMode()
{
	// 기내 시나리오도 멀티 퀘스트 정책을 쓰기 위해 공통 Murphy GameState/PlayerState를 기본값으로 고정합니다.
	GameStateClass = AAirplaneGameState::StaticClass();
	PlayerControllerClass = AMurphyPlayerController::StaticClass();
	PlayerStateClass = AMurphyPlayerState::StaticClass();

	// 기내 씬: 항상 1인칭 자유시점 고정
	ChatViewMode = EChatViewMode::FirstPersonLocked;
}

void AAirplaneGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AAirplaneGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer == nullptr)
	{
		PRINTLOG_SH(TEXT("HandleStartingNewPlayer: NewPlayer is null"));
		return;
	}

	// 이동 잠금 (기내 씬에서는 이동 불가)
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(NewPlayer->GetPawn()))
	{
		MurphyPlayer->SetMovementLocked(true);
		PRINTLOG_SH(TEXT("Airplane 진입: 플레이어 이동 입력 잠금"));
	}
	else
	{
		PRINTLOG_SH(TEXT("HandleStartingNewPlayer: MurphyPlayer 캐스팅 실패 (Pawn 없음)"));
	}

	// 시네마틱 재생 요청
	if (AMurphyPlayerController* MurphyPC = Cast<AMurphyPlayerController>(NewPlayer))
	{
		FCinematicPlayRequest Request;
		Request.CinematicId = TEXT("Airplane_Takeoff");
		Request.MediaSource = TSoftObjectPtr<UMediaSource>(FSoftObjectPath(TEXT("/Game/Movies/Temp_Takeoff.Temp_Takeoff")));
		Request.bSkippable  = true;
		Request.Fade.FadeToBlackDuration = 0.f;

		MurphyPC->Client_PlayCinematic(Request, 1);
	}
	else
	{
		PRINTLOG_SH(TEXT("HandleStartingNewPlayer: MurphyPlayerController 캐스팅 실패"));
	}
}


