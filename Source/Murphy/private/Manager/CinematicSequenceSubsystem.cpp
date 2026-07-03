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

void UCinematicSequenceSubsystem::StartLevelSequenceLocal(UCinematicSequenceData* Sequence)
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		PRINTLOG_SH(TEXT("StartLevelSequenceLocal: World is null"));
		return;
	}

	// 각 머신이 자기 로컬 매니저로 독립 재생한다. (서버 조율/RPC 없음)

	if (!Sequence || Sequence->Entries.Num() == 0)
	{
		PRINTLOG_SH(TEXT("StartLevelSequenceLocal: Sequence가 비어있음"));
		return;
	}

	ActiveSequence = Sequence;
	EntryIndex = 0;
	bRunning = true;

	// 로컬 매니저의 '검정 도달' 신호로 다음 엔트리/종료를 구동.
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
	Request.VolumeScale = Entry.VolumeScale;
	Request.Fade = Entry.Fade;

	CurrentPlayId = NextPlayId++;

	// 로컬 매니저로 직접 재생. 검정 Hold면 게임 노출 없이 이어재생, 아니면 일반 재생(최초 1회).
	UCinematicManagerSubsystem* Manager = GetLocalManager();
	if (!Manager)
	{
		return;
	}

	const bool bChainInHold = (Manager->GetCurrentState() == ECinematicState::HoldingBlack);
	if (bChainInHold)
	{
		Manager->PlayNextInHold(Request, CurrentPlayId);
	}
	else
	{
		Manager->PlayMedia(Request, CurrentPlayId, /*bInAutoReleaseHold*/ false);
	}

	PRINTLOG_SH(TEXT("[CinSeq] Entry %d 로컬 재생 (PlayId=%d, Chain=%d, Id=%s)"),
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
		// (안전장치) 트래블은 서버 권위. 인트로 시네마틱은 NextLevelKey가 없어 이 분기를 타지 않는다.
		if (World && World->GetNetMode() != NM_Client)
		{
			if (ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>())
			{
				PRINTLOG_SH(TEXT("[CinSeq] 시퀀스 종료 -> 트래블: %s"), *NextLevelKey.ToString());
				LevelSubsystem->TravelAllPlayers(NextLevelKey);
			}
		}
	}
	else
	{
		// 다음 레벨 없음 -> 로컬 매니저만 검정에서 게임으로 복귀.
		if (UCinematicManagerSubsystem* Manager = GetLocalManager())
		{
			Manager->ReleaseHold(CurrentPlayId);
		}
		PRINTLOG_SH(TEXT("[CinSeq] 시퀀스 종료 -> 게임 복귀(로컬 Release)"));
	}
}

UCinematicManagerSubsystem* UCinematicSequenceSubsystem::GetLocalManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>() : nullptr;
}
