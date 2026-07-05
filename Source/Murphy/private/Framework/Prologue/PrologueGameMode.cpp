// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Prologue/PrologueGameMode.h"

#include "Murphy.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Prologue/PrologueGameState.h"
#include "Data/CinematicSequenceData.h"
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

	// 인트로 시네마틱 완료 감지·재생은 각 머신 GameState가 로컬로 처리하고,
	// 완료 통보는 PlayerController RPC → NotifyIntroCinematicFinished로 들어온다.
}

void APrologueGameMode::OnImmigrationLevelShown()
{
	// LevelCinematic이 있으면 완료 통보(NotifyIntroCinematicFinished)에서 시작하고,
	// 없으면 기본적으로 즉시 시작하지 않는다. Prologue Immigration은 진입 시네마틱이 퀘스트 시작 게이트다.
	const APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	const UCinematicSequenceData* LevelCinematic = PrologueGameState ? PrologueGameState->GetLevelCinematic() : nullptr;
	if (!LevelCinematic)
	{
		if (bRequireImmigrationCinematicBeforeScenario)
		{
			PRINTLOG_SH(TEXT("[Prologue] LevelCinematic 없음 — Immigration 시나리오 자동 시작 보류"));
		}
		else
		{
			StartScenarioIfNeeded(EScenarioType::Prologue_Immigration);
			PRINTLOG_SH(TEXT("[Prologue] LevelCinematic 없음 — Immigration 시나리오 즉시 시작"));
		}
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

void APrologueGameMode::NotifyImmigrationLevelReady(APlayerController* ReadyPlayer)
{
	if (!ReadyPlayer)
	{
		return;
	}

	const APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	const UCinematicSequenceData* LevelCinematic = PrologueGameState ? PrologueGameState->GetLevelCinematic() : nullptr;
	if (LevelCinematic || bRequireImmigrationCinematicBeforeScenario)
	{
		PRINTLOG_JW(TEXT("[Prologue] Immigration 준비 완료 — 시나리오 시작은 진입 시네마틱 완료를 기다림 (ReadyPlayer=%s)"),
			*ReadyPlayer->GetName());
		return;
	}

	StartScenarioIfNeeded(EScenarioType::Prologue_Immigration);
	PRINTLOG_JW(TEXT("[Prologue] Immigration 준비 완료 — LevelCinematic 없음 fallback으로 Immigration 시나리오 시작 (ReadyPlayer=%s)"),
		*ReadyPlayer->GetName());
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

void APrologueGameMode::NotifyIntroCinematicFinished()
{
	// Prologue의 LevelCinematic은 Immigration 진입 시퀀스만 담당한다.
	// BaggageClaim 전환 시네마틱은 PlayerController의 로컬 CinematicManager 흐름에서 처리된다.
	// 첫 클라 완료 통보 시 1회만 시나리오 시작을 예약한다.
	if (bImmigrationScenarioStartRequested)
	{
		return;
	}

	APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	if (!PrologueGameState)
	{
		return;
	}

	const UCinematicSequenceData* LevelCinematic = PrologueGameState->GetLevelCinematic();
	if (!LevelCinematic)
	{
		PRINTLOG_SH(TEXT("[Prologue] 인트로 완료 통보 — LevelCinematic 없음, Immigration 시나리오 시작 보류"));
		return;
	}

	if (PrologueGameState->GetCurrentScenario() != EScenarioType::None)
	{
		return;
	}

	bImmigrationScenarioStartRequested = true;

	const float StartDelay = GetImmigrationScenarioStartDelay();
	if (StartDelay > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(
			ImmigrationScenarioStartTimerHandle,
			this,
			&APrologueGameMode::StartImmigrationScenarioAfterCinematic,
			StartDelay,
			false);
		PRINTLOG_JW(TEXT("[Prologue] 인트로 완료 — 화면 복귀 후 Immigration 시나리오 시작 대기 (Delay=%.2f)"), StartDelay);
		return;
	}

	StartImmigrationScenarioAfterCinematic();
}

void APrologueGameMode::StartImmigrationScenarioAfterCinematic()
{
	APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	if (!PrologueGameState || PrologueGameState->GetCurrentScenario() != EScenarioType::None)
	{
		return;
	}

	StartScenarioIfNeeded(EScenarioType::Prologue_Immigration);
	PRINTLOG_JW(TEXT("[Prologue] 시네마틱 완료 — Immigration 시나리오 시작"));
}

float APrologueGameMode::GetImmigrationScenarioStartDelay() const
{
	const APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>();
	const UCinematicSequenceData* LevelCinematic = PrologueGameState ? PrologueGameState->GetLevelCinematic() : nullptr;
	if (!LevelCinematic || LevelCinematic->Entries.IsEmpty())
	{
		return 0.f;
	}

	const FCinematicEntry& LastEntry = LevelCinematic->Entries.Last();
	return FMath::Max(0.f, LastEntry.Fade.FadeFromBlackDuration);
}

void APrologueGameMode::StartScenarioIfNeeded(EScenarioType ScenarioType)
{
	if (APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>())
	{
		PrologueGameState->StartScenario(ScenarioType);
	}
}
