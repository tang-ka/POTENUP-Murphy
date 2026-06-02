// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/LevelStreamingSubsystem.h"
#include "Settings/LevelStreamingSettings.h"
#include "GameFramework/GameMode.h"
#include "Murphy.h"

void ULevelStreamingSubsystem::TravelAllPlayers(FName LevelKey)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		PRINTLOG_SH(TEXT("TravelAllPlayers: World is null"));
		return;
	}

	// 서버 권한 확인 (ServerTravel은 서버에서만 유효)
	if (World->GetNetMode() == NM_Client)
	{
		PRINTLOG_SH(TEXT("TravelAllPlayers: Must be called on server"));
		return;
	}

	// LevelKey로 레벨 경로 조회
	const TSoftObjectPtr<UWorld>* FoundLevel = GetLevelMap().Find(LevelKey);
	if (!FoundLevel || FoundLevel->IsNull())
	{
		PRINTLOG_SH(TEXT("TravelAllPlayers: LevelKey '%s' not found in LevelMap"), *LevelKey.ToString());
		return;
	}

	// PlayerState를 유지하려면 SeamlessTravel이 필요 → GameMode에 활성화
	if (AGameMode* GameMode = World->GetAuthGameMode<AGameMode>())
	{
		GameMode->bUseSeamlessTravel = true;
	}

	// SoftObjectPath에서 패키지 경로 추출 (e.g. /Game/Maps/Lv_Lobby)
	const FString LevelPath = FoundLevel->ToSoftObjectPath().GetLongPackageName();

	// ServerTravel: 서버가 이동하면 모든 클라이언트도 자동으로 함께 이동
	World->ServerTravel(LevelPath);
}

const TMap<FName, TSoftObjectPtr<UWorld>>& ULevelStreamingSubsystem::GetLevelMap() const
{
	return GetDefault<ULevelStreamingSettings>()->LevelMap;
}
