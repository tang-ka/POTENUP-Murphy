// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Airplane/AirplaneGameMode.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"
#include "Framework/Airplane/AirplaneGameState.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Murphy.h"
#include "Manager/CinematicSequenceSubsystem.h"
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

	// 시퀀스 전체 완료 시 NPC 상호작용 박스 반전 후처리를 연결.
	if (UCinematicSequenceSubsystem* Seq = GetGameInstance()->GetSubsystem<UCinematicSequenceSubsystem>())
	{
		Seq->OnSequenceCompleted.AddUniqueDynamic(this, &AAirplaneGameMode::HandleCinematicComplete);
	}
}

void AAirplaneGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (AAirplaneGameState* AirplaneGameState = GetGameState<AAirplaneGameState>())
	{
		AirplaneGameState->StartScenario(EScenarioType::Tutorial_Airplane);
	}

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
}

void AAirplaneGameMode::HandleCinematicComplete()
{
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




