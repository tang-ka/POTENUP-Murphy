// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Data/CinematicTypes.h"
#include "CinematicManagerSubsystem.generated.h"

class UCinematicOverlayWidget;
class UMediaPlayer;
class UMediaTexture;
class UMediaSoundComponent;

// 미디어 종료 -> 검정 도달. (= relay가 서버에 "나 끝남" 보고할 지점)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicReachedHold, int32, PlayId);
// FadingFromBlack 종료. (= 입력 복귀, 후속 게임플레이 재개)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicCompleted, int32, PlayId);

/**
 * 로컬 풀스크린 미디어 시네마틱 재생기.
 *
 * 책임:
 *  - 상태 머신 + 4구간 페이드 + 검정 Hold
 *  - 풀스크린 커버 위젯 생성/제어
 *  - 미디어 재생/종료 감지
 *  - 로컬 입력 차단/복구
 *
 * 비책임(다음 단계):
 *  - 네트워크 동기화 (relay 컴포넌트가 PlayId 발급 후 본 API 호출)
 *  - MoviePlayer 트래블 커버, 비동기 맵 로드, 시퀀스/플레이리스트
 *
 * 종료 권위는 서브시스템에 없다. 재생이 끝나면 HoldingBlack에서 멈추고,
 * 외부(서버 합의)가 ReleaseHold()를 호출해야 검정에서 빠져나온다.
 * 단독 검증용으로 bAutoReleaseHold=true 를 주면 Hold 진입 즉시 자동 탈출한다.
 */
UCLASS()
class MURPHY_API UCinematicManagerSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~ USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override
	{
		// CDO/아키타입은 틱 대상에서 제외.
		return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Conditional;
	}

	// ---- 외부 API ----

	/**
	 * 재생 시작. Hold 진입까지 자동 진행.
	 * @param Request             재생 명세서
	 * @param PlayId              인스턴스 식별자 (외부/relay가 발급. 서브시스템은 보관만)
	 * @param bInAutoReleaseHold  true면 Hold 진입 즉시 자동 탈출 (단독 검증용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PlayMedia(const FCinematicPlayRequest& Request, int32 PlayId, bool bInAutoReleaseHold = false);

	/**
	 * 검정 Hold 상태에서 게임 노출 없이 다음 미디어로 이어 재생.
	 * (시퀀스 연속재생 / 트래블 직후 첫 클립용. Hold가 아니면 일반 PlayMedia로 폴백)
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PlayNextInHold(const FCinematicPlayRequest& Request, int32 PlayId);

	/** 검정 Hold 탈출 -> FadingFromBlack 시작. (서버 합의 후 호출) */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void ReleaseHold(int32 PlayId);

	/** 스킵 (bSkippable && 재생/페이드인 중일 때만). MediaFadingOut으로 점프. */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void RequestSkip(int32 PlayId);

	/** 즉시 강제 종료/정리 (연결 끊김, 강제 전환 등). */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void AbortImmediate(int32 PlayId);

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	bool IsPlaying() const { return CurrentState != ECinematicState::Idle; }

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	ECinematicState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	int32 GetActivePlayId() const { return ActivePlayId; }

	// ---- 이벤트 ----

	UPROPERTY(BlueprintAssignable, Category = "Cinematic")
	FOnCinematicReachedHold OnReachedHold;

	UPROPERTY(BlueprintAssignable, Category = "Cinematic")
	FOnCinematicCompleted OnCompleted;

private:
	// ---- 상태머신 내부 ----
	void EnterState(ECinematicState NewState);
	void TickFade(float DeltaTime);
	float GetSegmentDuration(ECinematicState State) const;

	// 미디어
	void StartMedia();
	UFUNCTION()
	void HandleMediaEndReached();
	UFUNCTION()
	void HandleMediaOpened(FString OpenedUrl);

	// 미디어 객체/소스 캐싱: 매 재생 생성·파괴 대신 재사용한다.
	void EnsureMediaObjects();
	UMediaSource* ResolveMediaSource(const TSoftObjectPtr<UMediaSource>& SoftSource);

	// 커버 위젯
	void CreateOverlay();
	void DestroyOverlay();
	bool HasLocalViewport() const;

	// 입력
	void BlockInput();
	void RestoreInput();
	void CheckSkipInput();

	// 종료 정리 (FadingFromBlack 완료 / Abort 공통)
	void FinishAndCleanup(bool bBroadcastCompleted);

	// 알파 적용 헬퍼
	void ApplyOpacity(float BlackAlpha, float MediaAlpha);

	// ---- 활성 시네마틱 상태 (단일 활성. 추후 시퀀스 확장 시 이 묶음을 큐로) ----
	ECinematicState CurrentState = ECinematicState::Idle;
	FCinematicPlayRequest ActiveRequest;
	int32 ActivePlayId = INDEX_NONE;
	float ElapsedInState = 0.f;
	bool bMediaEnded = false;
	bool bMediaOpened = false;
	bool bPendingMediaPlay = false;
	bool bAutoReleaseHold = false;

	UPROPERTY(Transient)
	TObjectPtr<UCinematicOverlayWidget> OverlayWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;
	
	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> MediaSoundComp;

	// 경로별 로드된 MediaSource 캐시 (반복 재생 시 재로드 방지 + GC 방지).
	UPROPERTY(Transient)
	TMap<FSoftObjectPath, TObjectPtr<UMediaSource>> CachedMediaSources;
};
