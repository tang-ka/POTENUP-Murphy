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
	
	// 시퀀스 완료 시 퀘스트 시작 + NPC 반전 후처리를 연결한다.
	if (UCinematicSequenceSubsystem* Seq = GetGameInstance()->GetSubsystem<UCinematicSequenceSubsystem>())
	{
		Seq->OnSequenceCompleted.AddUniqueDynamic(this, &AAirplaneGameMode::HandleSequenceCompleted);
	}
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

	// 시네마틱 시퀀스 시작은 베이스(AMurphyGameModeBase)가 LevelCinematic DA로 처리한다.
	
	// 시네마틱이 없으면 즉시 시나리오를 시작한다 (fallback).
	// LevelCinematic이 있는 경우는 HandleSequenceCompleted에서 StartScenario가 호출된다.
	if (!LevelCinematic)
	{
		if (AAirplaneGameState* AirplaneGameState = GetGameState<AAirplaneGameState>())
		{
			AirplaneGameState->StartScenario(EScenarioType::Tutorial_Airplane);
			PRINTLOG_SH(TEXT("[Airplane] LevelCinematic 없음 — 시나리오 즉시 시작"));
		}
	}
}

void AAirplaneGameMode::HandleSequenceCompleted()
{
	// 1. 시네마틱이 끝난 시점에 시나리오(퀘스트)를 시작한다.
	if (AAirplaneGameState* AirplaneGameState = GetGameState<AAirplaneGameState>())
	{
		AirplaneGameState->StartScenario(EScenarioType::Tutorial_Airplane);
		PRINTLOG_SH(TEXT("[Airplane] 시네마틱 완료 — 시나리오 시작"));
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
