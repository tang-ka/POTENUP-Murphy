// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Session/SessionGameMode.h"

#include "Murphy.h"
#include "Actors/Characters/SessionCharacter.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Session/SessionGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/LevelStreamingSubsystem.h"

ASessionGameMode::ASessionGameMode()
{
	GameStateClass = ASessionGameState::StaticClass();
	PlayerControllerClass = AMurphyPlayerController::StaticClass();
	PlayerStateClass = AMurphyPlayerState::StaticClass();

	bUseSeamlessTravel = true;
}

void ASessionGameMode::BeginPlay()
{
	Super::BeginPlay();
}

FString ASessionGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	const FString Nickname = UGameplayStatics::ParseOption(Options, TEXT("Name"));
	if (!Nickname.IsEmpty() && NewPlayerController->PlayerState)
	{
		if (AMurphyPlayerState* PS = NewPlayerController->GetPlayerState<AMurphyPlayerState>())
		{
			NewPlayerController->PlayerState->SetPlayerName(Nickname);
			PRINTLOG_SH(TEXT("닉네임 설정: %s"), *Nickname);
		}
	}

	return Result;
}

void ASessionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ASessionGameState* GS = GetGameState<ASessionGameState>())
	{
		GS->ConnectedPlayerCount++;
		PRINTLOG_SH(TEXT("플레이어 접속 — 현재 인원: %d"), GS->ConnectedPlayerCount);

		// 첫 접속자를 호스트로 지정
		if (AMurphyPlayerState* MurphyPS = NewPlayer->GetPlayerState<AMurphyPlayerState>())
		{
			MurphyPS->bIsHost = (GS->ConnectedPlayerCount == 1);
		}
	}

	SpawnSessionCharacterFor(NewPlayer);
}

void ASessionGameMode::Logout(AController* Exiting)
{
	if (AMurphyPlayerState* MurphyPS = Exiting ? Exiting->GetPlayerState<AMurphyPlayerState>() : nullptr)
	{
		const int32 RemovedIndex = SpawnedSessionCharacters.IndexOfByPredicate([MurphyPS](const ASessionCharacter* Character)
		{
			return Character && Character->GetOwningPlayerState() == MurphyPS;
		});

		if (RemovedIndex != INDEX_NONE)
		{
			SpawnedSessionCharacters[RemovedIndex]->Destroy();
			SpawnedSessionCharacters.RemoveAt(RemovedIndex);
		}
	}

	Super::Logout(Exiting);

	if (ASessionGameState* GS = GetGameState<ASessionGameState>())
	{
		GS->ConnectedPlayerCount = FMath::Max(0, GS->ConnectedPlayerCount - 1);
		PRINTLOG_SH(TEXT("플레이어 퇴장 — 현재 인원: %d"), GS->ConnectedPlayerCount);
	}
}

void ASessionGameMode::SpawnSessionCharacterFor(APlayerController* NewPlayer)
{
	if (!SessionCharacterClass || !NewPlayer)
	{
		PRINTLOG_SH(TEXT("SpawnSessionCharacterFor 실패 — SessionCharacterClass 미설정"));
		return;
	}

	AMurphyPlayerState* MurphyPS = NewPlayer->GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPS)
	{
		return;
	}

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	if (PlayerStarts.Num() == 0)
	{
		PRINTLOG_SH(TEXT("SpawnSessionCharacterFor 실패 — 레벨에 PlayerStart가 없습니다."));
		return;
	}

	// 호스트는 PlayerStart_1P, 게스트는 PlayerStart_2P 좌석에 고정 스폰
	const FString SeatKeyword = MurphyPS->bIsHost ? TEXT("1P") : TEXT("2P");
	AActor** FoundSeat = PlayerStarts.FindByPredicate([&SeatKeyword](const AActor* Actor)
	{
		return Actor->GetActorLabel().Contains(SeatKeyword);
	});

	AActor* SeatActor = FoundSeat ? *FoundSeat : PlayerStarts[0];
	if (!FoundSeat)
	{
		PRINTLOG_SH(TEXT("SpawnSessionCharacterFor 경고 — %s 좌석을 찾지 못해 첫 PlayerStart로 대체"), *SeatKeyword);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;

	ASessionCharacter* NewCharacter = GetWorld()->SpawnActor<ASessionCharacter>(
		SessionCharacterClass, SeatActor->GetActorTransform(), SpawnParams);

	if (!NewCharacter)
	{
		PRINTLOG_SH(TEXT("SpawnSessionCharacterFor 실패 — 스폰 실패"));
		return;
	}

	NewCharacter->InitializeForPlayer(MurphyPS);
	SpawnedSessionCharacters.Add(NewCharacter);

	PRINTLOG_SH(TEXT("SessionCharacter 스폰 완료 — Seat: %s, Player: %s"), *SeatKeyword, *MurphyPS->GetPlayerName());
}

void ASessionGameMode::TryStartGame()
{
	ASessionGameState* GS = GetGameState<ASessionGameState>();
	if (!GS)
	{
		return;
	}

	// 목적지 선택 여부
	if (GS->SelectedDestination == ETravelDestination::None)
	{
		PRINTLOG_SH(TEXT("게임 시작 실패 — 목적지가 선택되지 않았습니다."));
		return;
	}

	// 호스트 제외 전원 준비 검증
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMurphyPlayerState* MurphyPS = Cast<AMurphyPlayerState>(PS);
		if (!MurphyPS)
		{
			continue;
		}

		if (!MurphyPS->bIsHost && !MurphyPS->bIsReady)
		{
			PRINTLOG_SH(TEXT("게임 시작 불가: 준비되지 않은 플레이어 존재"));
			return;
		}
	}

	PRINTLOG_SH(TEXT("게임 시작 — 인게임 트래블: %s"), *InGameLevelKey.ToString());

	if (ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>())
	{
		LevelSubsystem->TravelAllPlayers(InGameLevelKey);
	}
}
