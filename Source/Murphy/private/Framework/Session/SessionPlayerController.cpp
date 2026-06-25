// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Session/SessionPlayerController.h"

#include "Murphy.h"
#include "Blueprint/UserWidget.h"
#include "Framework/MurphyPlayerState.h"
#include "Camera/CameraActor.h"
#include "Framework/Session/SessionGameMode.h"
#include "Framework/Session/SessionGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/SessionMainHUDWidget.h"

void ASessionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	// ── 고정 카메라 설정 ──────────────────────────────────────
	if (FixedCameraClass)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), FixedCameraClass, FoundActors);

		if (FoundActors.Num() > 0)
		{
			SetViewTargetWithBlend(FoundActors[0], 0.f);
			PRINTLOG_SH(TEXT("고정 카메라 설정 완료 — %s"), *FoundActors[0]->GetName());
		}
		else
		{
			PRINTLOG_SH(TEXT("고정 카메라를 찾지 못했습니다. Class: %s"), *FixedCameraClass->GetName());
		}
	}
	// ──────────────────────────────────────────────────────────

	if (!SessionMainHUDClass)
	{
		PRINTLOG_SH(TEXT("SessionMainHUDClass가 설정되지 않았습니다."));
		return;
	}

	SessionMainHUDWidget = CreateWidget<USessionMainHUDWidget>(this, SessionMainHUDClass);
	if (!SessionMainHUDWidget)
	{
		return;
	}

	SessionMainHUDWidget->OnCharacterSelected.BindUObject(this, &ASessionPlayerController::Server_SelectCharacter);
	SessionMainHUDWidget->OnReadyRequested.BindLambda([this](bool bReady) { Server_SetReady(bReady); });
	SessionMainHUDWidget->OnStartRequested.BindUObject(this, &ASessionPlayerController::Server_RequestStartGame);
	SessionMainHUDWidget->AddToViewport();
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());

	// 리슨 호스트는 BeginPlay 시점에 PS가 이미 존재하므로 여기서 바인딩 시도
	TryBindPlayerStateToHUD();
}

void ASessionPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SessionMainHUDWidget)
	{
		SessionMainHUDWidget->RemoveFromParent();
		SessionMainHUDWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ASessionPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트는 PS가 복제된 이후 이 함수가 호출됨
	if (IsLocalController())
	{
		TryBindPlayerStateToHUD();
	}
}

void ASessionPlayerController::TryBindPlayerStateToHUD()
{
	if (!SessionMainHUDWidget || bPlayerStateBoundToHUD)
	{
		return;
	}

	AMurphyPlayerState* MurphyPS = GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPS)
	{
		return;
	}

	SessionMainHUDWidget->SetPlayerRole(MurphyPS->bIsHost);
	bPlayerStateBoundToHUD = true;

	PRINTLOG_SH(TEXT("HUD PlayerState 바인딩 완료 — bIsHost: %s"), MurphyPS->bIsHost ? TEXT("true") : TEXT("false"));
}

void ASessionPlayerController::Server_SelectCharacter_Implementation(EPlayerCharacterType NewCharacter)
{
	if (AMurphyPlayerState* MurphyPS = GetPlayerState<AMurphyPlayerState>())
	{
		MurphyPS->SelectedCharacter = NewCharacter;
		MurphyPS->OnSessionRoomStateChanged.Broadcast();
		PRINTLOG_SH(TEXT("캐릭터 선택: %s"), *UEnum::GetValueAsString(NewCharacter));
	}
}

void ASessionPlayerController::Server_SetReady_Implementation(bool bReady)
{
	if (AMurphyPlayerState* MurphyPS = GetPlayerState<AMurphyPlayerState>())
	{
		MurphyPS->bIsReady = bReady;
		MurphyPS->OnSessionRoomStateChanged.Broadcast();
		PRINTLOG_SH(TEXT("준비 상태 변경: %d"), bReady);
	}
}

void ASessionPlayerController::Server_SelectDestination_Implementation(ETravelDestination NewDestination)
{
	AMurphyPlayerState* MurphyPS = GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPS || !MurphyPS->bIsHost)
	{
		PRINTLOG_SH(TEXT("여행지 선택 거부: 호스트 권한 없음"));
		return;
	}

	if (ASessionGameState* GS = GetWorld()->GetGameState<ASessionGameState>())
	{
		GS->SelectedDestination = NewDestination;
		GS->OnSelectedDestinationChanged.Broadcast();
		PRINTLOG_SH(TEXT("여행지 선택: %s"), *UEnum::GetValueAsString(NewDestination));
	}
}

void ASessionPlayerController::Server_RequestStartGame_Implementation()
{
	AMurphyPlayerState* MurphyPS = GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPS || !MurphyPS->bIsHost)
	{
		PRINTLOG_SH(TEXT("게임 시작 거부: 호스트 권한 없음"));
		return;
	}

	if (ASessionGameMode* GM = GetWorld()->GetAuthGameMode<ASessionGameMode>())
	{
		GM->TryStartGame();
	}
}
