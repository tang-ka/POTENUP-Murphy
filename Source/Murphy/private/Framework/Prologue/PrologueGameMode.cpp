// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Prologue/PrologueGameMode.h"

#include "Murphy.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Prologue/PrologueGameState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/CinematicSequenceSubsystem.h"
#include "Manager/LevelStreamingSubsystem.h"

APrologueGameMode::APrologueGameMode()
{
	// 입국심사/수화물 시나리오에서 개인/공유 퀘스트를 라우팅하려면 공통 Murphy GameState가 필요합니다.
	GameStateClass = APrologueGameState::StaticClass();
	PlayerControllerClass = AMurphyPlayerController::StaticClass();
	PlayerStateClass = AMurphyPlayerState::StaticClass();

	// 이후 씬: 3인칭 Focus 기본
	ChatViewMode = EChatViewMode::ThirdPersonFocus;

	// 프롤로그 전용 폰 BP는 BP_PrologueGameMode 기본값에서 BoyPawnClass/GirlPawnClass로 할당합니다.
}

void APrologueGameMode::BeginPlay()
{
	Super::BeginPlay();

	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>();
	if (!LevelSubsystem)
	{
		PRINTLOG_SH(TEXT("BeginPlay: LevelStreamingSubsystem is null"));
		return;
	}

	LevelSubsystem->LoadSubLevel(TEXT("SubLevel_Immigration"), true, false);
	
	// 서버에서 레벨이 보이면 Pawn 스폰
	ULevelStreaming* ImmigrationLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_Immigration"));
	if (ImmigrationLevel)
	{
		ImmigrationLevel->OnLevelShown.AddDynamic(this, &APrologueGameMode::OnImmigrationLevelShown);
	}

	ULevelStreaming* BaggageClaimLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_BaggageClaim"));
	if (BaggageClaimLevel)
	{
		BaggageClaimLevel->OnLevelShown.AddDynamic(this, &APrologueGameMode::OnBaggageClaimLevelShown);
	}
	
	// Immigration 진입 시네마틱 완료 시 퀘스트 시작을 연결한다.
	if (UCinematicSequenceSubsystem* Seq = GetGameInstance()->GetSubsystem<UCinematicSequenceSubsystem>())
	{
		Seq->OnSequenceCompleted.AddUniqueDynamic(this, &APrologueGameMode::HandleSequenceCompleted);
	}
}

void APrologueGameMode::OnImmigrationLevelShown()
{
	// LevelCinematic이 있으면 HandleSequenceCompleted에서 시작하고,
	// 없으면 Airplane과 동일하게 즉시 시작한다 (fallback).
	if (!LevelCinematic)
	{
		StartScenarioIfNeeded(EScenarioType::Prologue_Immigration);
		PRINTLOG_SH(TEXT("[Prologue] LevelCinematic 없음 — Immigration 시나리오 즉시 시작"));
	}

	// Pawn 리포지션 (스폰 위치 설정)

	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>();
	if (!LevelSubsystem)
	{
		return;
	}

	ULevelStreaming* ImmigrationLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_Immigration"));
	if (!ImmigrationLevel)
	{
		return;
	}

	ULevel* LoadedLevel = ImmigrationLevel->GetLoadedLevel();
	if (!LoadedLevel)
	{
		return;
	}

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	AActor** FoundStart = PlayerStarts.FindByPredicate([&](AActor* Actor)
	{
		return Actor->GetLevel() == LoadedLevel;
	});

	if (!FoundStart)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (!Pawn)
		{
			continue;
		}

		Pawn->SetActorLocationAndRotation(
			(*FoundStart)->GetActorLocation(),
			(*FoundStart)->GetActorRotation());
	}
}

void APrologueGameMode::OnBaggageClaimLevelShown()
{	
	if (APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>())
	{
		PrologueGameState->SetBaggageCustomsHoldActorsActive(false);
	}
}

void APrologueGameMode::NotifyBaggageClaimLevelReady(APlayerController* ReadyPlayer)
{
	if (!ReadyPlayer)
	{
		return;
	}

	StartScenarioIfNeeded(EScenarioType::Prologue_Baggage);	
	PRINTLOG_JW(TEXT("[Prologue] BaggageClaim 준비 완료 — Baggage 시나리오 시작 (ReadyPlayer=%s)"),
		*ReadyPlayer->GetName());

	if (APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>())
	{
		PrologueGameState->SetBaggageCustomsHoldActorsActive(false);
	}
}

void APrologueGameMode::HandleAINodeReached(FName NodeId)
{
	if (NodeId != BaggageCustomsHoldNodeId)
	{
		return;
	}

	APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	if (!PrologueGameState || PrologueGameState->GetCurrentScenario() != EScenarioType::Prologue_Baggage)
	{
		return;
	}

	PrologueGameState->SetBaggageCustomsHoldActorsActive(true);
}

void APrologueGameMode::HandleSequenceCompleted()
{
	// Prologue의 서버 전역 LevelCinematic은 Immigration 진입 시퀀스만 담당한다.
	// BaggageClaim 전환 시네마틱은 PlayerController의 로컬 CinematicManager 흐름에서 처리된다.

	APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	if (!PrologueGameState)
	{
		return;
	}

	const EScenarioType Current = PrologueGameState->GetCurrentScenario();

	if (Current == EScenarioType::None)
	{
		StartScenarioIfNeeded(EScenarioType::Prologue_Immigration);
		PRINTLOG_JW(TEXT("[Prologue] 시네마틱 완료 — Immigration 시나리오 시작"));
	}
}

void APrologueGameMode::StartScenarioIfNeeded(EScenarioType ScenarioType)
{
	if (APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>())
	{
		PrologueGameState->StartScenario(ScenarioType);
	}
}
