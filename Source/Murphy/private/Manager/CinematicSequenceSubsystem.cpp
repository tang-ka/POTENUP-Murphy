// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager/CinematicSequenceSubsystem.h"

#include "Murphy.h"
#include "Data/CinematicTypes.h"
#include "Data/CinematicSequenceData.h"
#include "Framework/MurphyPlayerController.h"
#include "Manager/CinematicManagerSubsystem.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UCinematicSequenceSubsystem::StartLevelSequence(UCinematicSequenceData* Sequence)
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		PRINTLOG_SH(TEXT("StartLevelSequence: World is null"));
		return;
	}

	// 클라는 RPC로 받은 재생만 수행. 시퀀스 구동은 서버 권위.
	if (World->GetNetMode() == NM_Client)
	{
		return;
	}

	if (!Sequence || Sequence->Entries.Num() == 0)
	{
		PRINTLOG_SH(TEXT("StartLevelSequence: Sequence가 비어있음"));
		return;
	}

	ActiveSequence = Sequence;
	EntryIndex = 0;
	bRunning = true;

	// 호스트 매니저의 '검정 도달' 신호로 다음 엔트리/트래블을 구동.
	if (UCinematicManagerSubsystem* Manager = GetLocalManager())
	{
		if (!bSubscribed)
		{
			Manager->OnReachedHold.AddUniqueDynamic(this, &UCinematicSequenceSubsystem::HandleReachedHold);
			bSubscribed = true;
		}
	}

	PlayEntryAtIndex(0);
}

void UCinematicSequenceSubsystem::PlayEntryAtIndex(int32 Index)
{
	if (!ActiveSequence || !ActiveSequence->Entries.IsValidIndex(Index))
	{
		return;
	}

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const FCinematicEntry& Entry = ActiveSequence->Entries[Index];

	// 엔트리 -> 재생 요청 변환.
	FCinematicPlayRequest Request;
	Request.CinematicId = Entry.CinematicId;
	Request.MediaSource = Entry.Media;
	Request.Duration = Entry.Duration;
	Request.bSkippable = Entry.bSkippable;
	Request.Fade = Entry.Fade;

	CurrentPlayId = NextPlayId++;

	// 매니저가 검정 Hold면 게임 노출 없이 이어재생, 아니면 일반 재생(최초 1회).
	const UCinematicManagerSubsystem* Manager = GetLocalManager();
	const bool bChainInHold = (Manager && Manager->GetCurrentState() == ECinematicState::HoldingBlack);

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(It->Get());
		if (!PC)
		{
			continue;
		}

		if (bChainInHold)
		{
			PC->Client_PlayNextCinematic(Request, CurrentPlayId);
		}
		else
		{
			PC->Client_PlayCinematic(Request, CurrentPlayId);
		}
	}

	PRINTLOG_SH(TEXT("[CinSeq] Entry %d 재생 (PlayId=%d, Chain=%d, Id=%s)"),
		Index, CurrentPlayId, bChainInHold ? 1 : 0, *Entry.CinematicId.ToString());
}

void UCinematicSequenceSubsystem::HandleReachedHold(int32 PlayId)
{
	if (!bRunning || PlayId != CurrentPlayId)
	{
		return;
	}

	const int32 NextIndex = EntryIndex + 1;
	if (ActiveSequence && ActiveSequence->Entries.IsValidIndex(NextIndex))
	{
		EntryIndex = NextIndex;
		PlayEntryAtIndex(NextIndex);
	}
	else
	{
		OnSequenceFinished();
	}
}

void UCinematicSequenceSubsystem::OnSequenceFinished()
{
	bRunning = false;

	// 레벨별 후처리(NPC 반전 등)를 먼저. 아직 검정 Hold라 화면엔 안 보임.
	OnSequenceCompleted.Broadcast();

	const FName NextLevelKey = ActiveSequence ? ActiveSequence->NextLevelKey : NAME_None;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;

	if (!NextLevelKey.IsNone())
	{
		// 검정 Hold 유지한 채 트래블 -> 새 월드 로드가 검정 뒤에서 진행.
		if (ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>())
		{
			PRINTLOG_SH(TEXT("[CinSeq] 시퀀스 종료 -> 트래블: %s"), *NextLevelKey.ToString());
			LevelSubsystem->TravelAllPlayers(NextLevelKey);
		}
		// 도착 후 새 레벨 GameMode가 StartLevelSequence 재호출 (매니저는 검정 Hold 유지).
	}
	else
	{
		// 다음 레벨 없음 -> 검정에서 게임으로 복귀.
		if (World)
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(It->Get());
				if (PC)
				{
					PC->Client_ReleaseCinematic(CurrentPlayId);
				}
			}
		}
		PRINTLOG_SH(TEXT("[CinSeq] 시퀀스 종료 -> 게임 복귀(Release)"));
	}
}

UCinematicManagerSubsystem* UCinematicSequenceSubsystem::GetLocalManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>() : nullptr;
}
