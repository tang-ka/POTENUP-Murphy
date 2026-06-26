// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Airplane/AirplaneGameMode.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"
#include "Framework/Airplane/AirplaneGameState.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Murphy.h"
#include "Data/CinematicTypes.h"
#include "MediaSource.h"
#include "Manager/CinematicManagerSubsystem.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

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

	// 시네마틱 재생 요청
	if (AMurphyPlayerController* MurphyPC = Cast<AMurphyPlayerController>(NewPlayer))
	{
		FCinematicPlayRequest Request;
		Request.CinematicId = TEXT("Airplane_Takeoff");
		// Request.MediaSource = TSoftObjectPtr<UMediaSource>(FSoftObjectPath(TEXT("/Game/Movies/Temp_Takeoff.Temp_Takeoff")));
		Request.MediaSource = TSoftObjectPtr<UMediaSource>(FSoftObjectPath(TEXT("/Game/Movies/05-1_Manhattan_street_first-person_view_1080p_202606251542.05-1_Manhattan_street_first-person_view_1080p_202606251542")));
		Request.bSkippable  = true;
		Request.Fade.FadeToBlackDuration = 0.0f;
		Request.Fade.MediaFadeInDuration = 2.f;
		Request.Fade.FadeFromBlackDuration = 2.f;

		MurphyPC->Client_PlayCinematic(Request, 1);
		
		UCinematicManagerSubsystem* CinematicManager = GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>();
		CinematicManager->OnCompleted.AddDynamic(this, &AAirplaneGameMode::HandleCinematicComplete);
	}
	else
	{
		PRINTLOG_SH(TEXT("HandleStartingNewPlayer: MurphyPlayerController 캐스팅 실패"));
	}
}

void AAirplaneGameMode::HandleCinematicComplete(int32 PlayId)
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




