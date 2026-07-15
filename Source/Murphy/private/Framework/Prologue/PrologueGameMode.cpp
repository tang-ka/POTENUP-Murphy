// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Prologue/PrologueGameMode.h"

#include "Murphy.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Prologue/PrologueGameState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "UObject/ConstructorHelpers.h"

APrologueGameMode::APrologueGameMode()
{
	// 입국심사/수화물 시나리오에서 개인/공유 퀘스트를 라우팅하려면 공통 Murphy GameState가 필요합니다.
	GameStateClass = APrologueGameState::StaticClass();
	PlayerControllerClass = AMurphyPlayerController::StaticClass();
	PlayerStateClass = AMurphyPlayerState::StaticClass();

	// 이후 씬: 3인칭 Focus 기본
	ChatViewMode = EChatViewMode::ThirdPersonFocus;

	// 프롤로그 전용 폰 BP를 캐릭터 선택값에 따라 스폰하도록 바인딩 (Boy=Player1, Girl=Player2)
	static ConstructorHelpers::FClassFinder<APawn> BoyPawnFinder(TEXT("/Game/Blueprints/Characters/Player/BP_Player1.BP_Player1_C"));
	if (BoyPawnFinder.Succeeded())
	{
		BoyPawnClass = BoyPawnFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<APawn> GirlPawnFinder(TEXT("/Game/Blueprints/Characters/Player/BP_player2.BP_Player2_C"));
	if (GirlPawnFinder.Succeeded())
	{
		GirlPawnClass = GirlPawnFinder.Class;
	}
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
}

void APrologueGameMode::OnImmigrationLevelShown()
{
	StartScenarioIfNeeded(EScenarioType::Prologue_Immigration);

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
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn) continue;

		Pawn->SetActorLocationAndRotation(
			(*FoundStart)->GetActorLocation(),
			(*FoundStart)->GetActorRotation());
	}
}

void APrologueGameMode::OnBaggageClaimLevelShown()
{
	StartScenarioIfNeeded(EScenarioType::Prologue_Baggage);
}

void APrologueGameMode::StartScenarioIfNeeded(EScenarioType ScenarioType)
{
	if (APrologueGameState* PrologueGameState = GetGameState<APrologueGameState>())
	{
		PrologueGameState->StartScenario(ScenarioType);
	}
}
