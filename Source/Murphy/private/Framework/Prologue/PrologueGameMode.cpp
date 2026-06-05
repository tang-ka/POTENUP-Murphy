// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Prologue/PrologueGameMode.h"

#include "Murphy.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/LevelStreamingSubsystem.h"

APrologueGameMode::APrologueGameMode()
{
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
}

void APrologueGameMode::OnImmigrationLevelShown()
{
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
