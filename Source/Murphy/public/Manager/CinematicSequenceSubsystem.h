// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CinematicSequenceSubsystem.generated.h"

class UCinematicSequenceData;
class UCinematicManagerSubsystem;

// 모든 엔트리 재생 완료 시 broadcast. (트래블/게임복귀 직전 - 레벨별 후처리 훅)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCinematicSequenceCompleted);

/**
 * 레벨 시네마틱 시퀀스 구동기 (두뇌).
 *
 * 책임:
 *  - 시퀀스 데이터의 엔트리를 순서대로 재생 (연속, 검정 유지)
 *  - 마지막 엔트리 후 트래블(검정 커버) 또는 게임 복귀
 *
 * 비책임:
 *  - 실제 미디어 재생 (CinematicManagerSubsystem이 담당)
 *
 * 서버 권위. 클라이언트는 PlayerController RPC로 받은 재생만 수행한다.
 */
UCLASS()
class MURPHY_API UCinematicSequenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 서버 전용. 레벨 시퀀스 재생 시작. */
	void StartLevelSequence(UCinematicSequenceData* Sequence);

	bool IsRunning() const { return bRunning; }

	UPROPERTY(BlueprintAssignable, Category = "Cinematic")
	FOnCinematicSequenceCompleted OnSequenceCompleted;

private:
	void PlayEntryAtIndex(int32 Index);

	UFUNCTION()
	void HandleReachedHold(int32 PlayId);

	void OnSequenceFinished();
	UCinematicManagerSubsystem* GetLocalManager() const;

	UPROPERTY(Transient)
	TObjectPtr<UCinematicSequenceData> ActiveSequence;

	int32 EntryIndex = INDEX_NONE;
	int32 CurrentPlayId = INDEX_NONE;
	int32 NextPlayId = 1;
	bool bRunning = false;
	bool bSubscribed = false;
};
