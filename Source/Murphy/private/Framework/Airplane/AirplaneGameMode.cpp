// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Airplane/AirplaneGameMode.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"
#include "Framework/Airplane/AirplaneGameState.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Murphy.h"
#include "Manager/CinematicSequenceSubsystem.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

AAirplaneGameMode::AAirplaneGameMode()
{
	// 기내 시나리오도 멀티 퀘스트 정책을 쓰기 위해 공통 Murphy GameState/PlayerState를 기본값으로 고정합니다.
	GameStateClass = AAirplaneGameState::StaticClass();
	PlayerControllerClass = AMurphyPlayerController::StaticClass();
	PlayerStateClass = AMurphyPlayerState::StaticClass();

	// 기내 씬: 항상 1인칭 자유시점 고정
	ChatViewMode = EChatViewMode::FirstPersonLocked;

	// 기내 전용 폰 BP는 BP_AirplaneGameMode 기본값에서 BoyPawnClass/GirlPawnClass로 할당합니다.
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

	// 시네마틱 재생은 각 머신 GameState(TryPlayLevelIntro)가 로컬로 처리한다.

	// 시네마틱이 없으면 즉시 시나리오를 시작한다 (fallback).
	// 시네마틱이 있으면 각 클라의 완료 통보(NotifyIntroCinematicFinished)에서 시작한다.
	AAirplaneGameState* AirplaneGameState = GetGameState<AAirplaneGameState>();
	if (AirplaneGameState && !AirplaneGameState->GetLevelCinematic())
	{
		AirplaneGameState->StartScenario(EScenarioType::Tutorial_Airplane);
		PRINTLOG_SH(TEXT("[Airplane] LevelCinematic 없음 — 시나리오 즉시 시작"));
	}
}

void AAirplaneGameMode::NotifyIntroCinematicFinished()
{
	// 첫 클라 완료 통보 시 1회 시작. StartScenario가 중복 호출을 가드한다.
	if (AAirplaneGameState* AirplaneGameState = GetGameState<AAirplaneGameState>())
	{
		AirplaneGameState->StartScenario(EScenarioType::Tutorial_Airplane);
		PRINTLOG_SH(TEXT("[Airplane] 인트로 완료 통보 — 시나리오 시작"));
	}
}

void AAirplaneGameMode::HandleWriteArrivalCard()
{
	// 2. NPC 상호작용 박스 X 반전.
	TArray<AActor*> FoundNPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAgentNPCBase::StaticClass(), FoundNPCs);

	for (AActor* Actor : FoundNPCs)
	{
		AAgentNPCBase* NPC = Cast<AAgentNPCBase>(Actor);
		if (!NPC)
		{
			continue;
		}

		UBoxComponent* InteractionBox = NPC->FindComponentByClass<UBoxComponent>();
		if (!InteractionBox)
		{
			PRINTLOG_SH(TEXT("[Airplane] %s의 InteractionBox를 찾지 못함"), *NPC->GetName());
			continue;
		}

		FVector LocalLoc = InteractionBox->GetRelativeLocation();
		LocalLoc.X *= -1.0f;
		InteractionBox->SetRelativeLocation(LocalLoc);

		PRINTLOG_SH(TEXT("[Airplane] %s InteractionBox X 반전 (%f)"), *NPC->GetName(), LocalLoc.X);
	}
}

void AAirplaneGameMode::CompleteScenarioAndTravel()
{
	if (bScenarioCompleteTravelRequested)
	{
		PRINTLOG_SH(TEXT("[Airplane] 시나리오 완료 트래블이 이미 요청되었습니다."));
		return;
	}

	if (NextLevelKeyAfterScenarioComplete.IsNone())
	{
		PRINTLOG_SH(TEXT("[Airplane] NextLevelKeyAfterScenarioComplete가 비어 있어 트래블하지 않습니다."));
		return;
	}

	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>()
		: nullptr;
	if (!LevelSubsystem)
	{
		PRINTLOG_SH(TEXT("[Airplane] LevelStreamingSubsystem이 없어 트래블하지 못했습니다."));
		return;
	}

	bScenarioCompleteTravelRequested = true;

	if (AAirplaneGameState* AirplaneGameState = GetGameState<AAirplaneGameState>())
	{
		// 비행기 대화 시나리오를 성공 종료로 처리한 뒤 다음 맵으로 이동합니다.
		AirplaneGameState->EndScenario(true);
	}

	PRINTLOG_SH(TEXT("[Airplane] 시나리오 완료 -> 다음 레벨 트래블: %s"), *NextLevelKeyAfterScenarioComplete.ToString());
	LevelSubsystem->TravelAllPlayers(NextLevelKeyAfterScenarioComplete);
}
