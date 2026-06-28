// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager/CinematicManagerSubsystem.h"

#include "Murphy.h"
#include "Data/UILayerTypes.h"
#include "Settings/CinematicSettings.h"
#include "UI/CinematicOverlayWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "MediaTexture.h"


void UCinematicManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCinematicManagerSubsystem::Deinitialize()
{
	// 진행 중이면 방송 없이 정리.
	if (CurrentState != ECinematicState::Idle)
	{
		FinishAndCleanup(/*bBroadcastCompleted*/ false);
	}

	// 캐싱한 미디어 객체는 서브시스템 종료 시점에만 실제 해제.
	if (MediaPlayer)
	{
		MediaPlayer->Close();
		MediaPlayer->OnEndReached.RemoveDynamic(this, &UCinematicManagerSubsystem::HandleMediaEndReached);
		MediaPlayer->OnMediaOpened.RemoveDynamic(this, &UCinematicManagerSubsystem::HandleMediaOpened);
		MediaPlayer = nullptr;
	}
	if (MediaSoundComp)
	{
		MediaSoundComp->UnregisterComponent();
		MediaSoundComp->DestroyComponent();
		MediaSoundComp = nullptr;
	}
	MediaTexture = nullptr;
	CachedMediaSources.Empty();

	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// FTickableGameObject
// ---------------------------------------------------------------------------

TStatId UCinematicManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCinematicManagerSubsystem, STATGROUP_Tickables);
}

bool UCinematicManagerSubsystem::IsTickable() const
{
	// Idle일 땐 틱 불필요.
	return CurrentState != ECinematicState::Idle;
}

void UCinematicManagerSubsystem::Tick(float DeltaTime)
{
	if (!IsTickable())
	{
		return;
	}

	TickFade(DeltaTime);
	CheckSkipInput();
}

// ---------------------------------------------------------------------------
// 외부 API
// ---------------------------------------------------------------------------

void UCinematicManagerSubsystem::PlayMedia(const FCinematicPlayRequest& Request, int32 PlayId, bool bInAutoReleaseHold)
{
	// 이미 재생 중이면 즉시 정리하고 덮어쓴다 (단일 활성 가정).
	if (CurrentState != ECinematicState::Idle)
	{
		FinishAndCleanup(false);
	}

	ActiveRequest = Request;
	ActivePlayId = PlayId;
	bAutoReleaseHold = bInAutoReleaseHold;
	bMediaEnded = false;
	bMediaOpened = false;
	bPendingMediaPlay = false;
	ElapsedInState = 0.f;

	// 미디어 객체 준비 + 프리롤 (검정 뒤에서 미리 Open).
	StartMedia();

	// 커버 위젯 (로컬 뷰포트 있을 때만; 데디 서버는 상태머신만 진행).
	CreateOverlay();
	if (OverlayWidget)
	{
		OverlayWidget->SetFadeColor(ActiveRequest.Fade.FadeColor);
		OverlayWidget->SetMediaTexture(MediaTexture);
	}
	ApplyOpacity(/*Black*/ 0.f, /*Media*/ 0.f); // 시작: 게임 보임.

	BlockInput();

	EnterState(ECinematicState::FadingToBlack);
}

void UCinematicManagerSubsystem::PlayNextInHold(const FCinematicPlayRequest& Request, int32 PlayId)
{
	// 검정 Hold가 아니면 일반 재생으로 폴백 (최초 1회 등).
	if (CurrentState != ECinematicState::HoldingBlack)
	{
		PlayMedia(Request, PlayId, /*bInAutoReleaseHold*/ false);
		return;
	}

	ActiveRequest = Request;
	ActivePlayId = PlayId;
	bAutoReleaseHold = false;
	bMediaEnded = false;
	bMediaOpened = false;
	bPendingMediaPlay = false;
	ElapsedInState = 0.f;

	// 새 미디어 프리롤. (이전 미디어는 HoldingBlack 진입 시 Close됨)
	StartMedia();

	// 트래블로 오버레이가 파괴됐을 수 있으니 재생성 + 검정 유지.
	CreateOverlay();
	if (OverlayWidget)
	{
		OverlayWidget->SetFadeColor(ActiveRequest.Fade.FadeColor);
		OverlayWidget->SetMediaTexture(MediaTexture);
	}
	ApplyOpacity(/*Black*/ 1.f, /*Media*/ 0.f); // 게임 노출 없이 검정 유지.

	BlockInput();

	// 게임 -> 검정 단계를 건너뛰고 바로 검정 -> 미디어.
	EnterState(ECinematicState::MediaFadingIn);
}

void UCinematicManagerSubsystem::ReleaseHold(int32 PlayId)
{
	if (PlayId != ActivePlayId)
	{
		return;
	}

	if (CurrentState == ECinematicState::HoldingBlack)
	{
		EnterState(ECinematicState::FadingFromBlack);
	}
}

void UCinematicManagerSubsystem::RequestSkip(int32 PlayId)
{
	if (PlayId != ActivePlayId || !ActiveRequest.bSkippable)
	{
		return;
	}

	// 페이드인/재생 중에만 스킵 의미가 있다 -> 페이드아웃으로 점프.
	if (CurrentState == ECinematicState::MediaFadingIn || CurrentState == ECinematicState::Playing)
	{
		EnterState(ECinematicState::MediaFadingOut);
	}
}

void UCinematicManagerSubsystem::AbortImmediate(int32 PlayId)
{
	if (PlayId != ActivePlayId || CurrentState == ECinematicState::Idle)
	{
		return;
	}

	FinishAndCleanup(false);
}

// ---------------------------------------------------------------------------
// 상태머신
// ---------------------------------------------------------------------------

float UCinematicManagerSubsystem::GetSegmentDuration(ECinematicState State) const
{
	const FCinematicFadeParams& F = ActiveRequest.Fade;
	switch (State)
	{
	case ECinematicState::FadingToBlack:   return F.FadeToBlackDuration;
	case ECinematicState::MediaFadingIn:   return F.MediaFadeInDuration;
	case ECinematicState::MediaFadingOut:  return F.MediaFadeOutDuration;
	case ECinematicState::FadingFromBlack: return F.FadeFromBlackDuration;
	default:                               return 0.f;
	}
}

void UCinematicManagerSubsystem::EnterState(ECinematicState NewState)
{
	CurrentState = NewState;
	ElapsedInState = 0.f;

	switch (NewState)
	{
	case ECinematicState::FadingToBlack:
		// 게임 -> 검정. (미디어는 아직 0)
		break;

	case ECinematicState::MediaFadingIn:
		// 검정 위로 미디어 페이드인. 여기서 재생 시작.
		ApplyOpacity(1.f, 0.f);
		if (MediaPlayer)
		{
			if (bMediaOpened)
			{
				if (MediaSoundComp)
				{
					MediaSoundComp->Start();
				}
				MediaPlayer->Play();
			}
			else
			{
				// OpenSource 비동기 완료 전 (FadeToBlackDuration=0 등) -> OnMediaOpened에서 Play.
				bPendingMediaPlay = true;
			}
		}
		break;

	case ECinematicState::Playing:
		ApplyOpacity(1.f, 1.f);
		// 자연 종료(OnEndReached)가 이미 도착해 있으면 바로 페이드아웃.
		if (bMediaEnded)
		{
			EnterState(ECinematicState::MediaFadingOut);
			return;
		}
		break;

	case ECinematicState::MediaFadingOut:
		// 미디어 -> 검정.
		break;

	case ECinematicState::HoldingBlack:
		ApplyOpacity(1.f, 0.f);
		if (MediaPlayer)
		{
			MediaPlayer->Close();
		}
		// Close는 디코더만 멈춘다. 사운드 컴포넌트가 살아있으면 잔여 버퍼가 루프됨 -> 명시 정지.
		if (MediaSoundComp)
		{
			MediaSoundComp->Stop();
		}
		// 미디어 종료 = 검정 도달. relay가 이 시점에 서버 보고.
		OnReachedHold.Broadcast(ActivePlayId);

		// 단독 검증 모드: 즉시 탈출.
		if (bAutoReleaseHold)
		{
			EnterState(ECinematicState::FadingFromBlack);
			return;
		}
		break;

	case ECinematicState::FadingFromBlack:
		// 검정 -> 게임.
		break;

	default:
		break;
	}

	// 0초 페이드 구간은 즉시 통과 (NaN 방지 + 1프레임 깜빡 방지).
	if ((NewState == ECinematicState::FadingToBlack
		|| NewState == ECinematicState::MediaFadingOut
		|| NewState == ECinematicState::FadingFromBlack)
		&& GetSegmentDuration(NewState) <= KINDA_SMALL_NUMBER)
	{
		// 끝 상태 알파를 적용한 뒤 다음 단계로.
		// (MediaFadingIn은 제외: 첫 프레임 게이트를 거치도록 항상 TickFade에서 처리)
		switch (NewState)
		{
		case ECinematicState::FadingToBlack:   ApplyOpacity(1.f, 0.f); EnterState(ECinematicState::MediaFadingIn);	break;
		case ECinematicState::MediaFadingOut:  ApplyOpacity(1.f, 0.f); EnterState(ECinematicState::HoldingBlack);	break;
		case ECinematicState::FadingFromBlack: ApplyOpacity(0.f, 0.f); FinishAndCleanup(true);	break;
		default: break;
		}
	}
}

void UCinematicManagerSubsystem::TickFade(float DeltaTime)
{
	ElapsedInState += DeltaTime;

	switch (CurrentState)
	{
	case ECinematicState::FadingToBlack:
	{
		const float Dur = GetSegmentDuration(CurrentState);
		const float A = FMath::Clamp(ElapsedInState / Dur, 0.f, 1.f);
		ApplyOpacity(A, 0.f);
		if (ElapsedInState >= Dur)
		{
			EnterState(ECinematicState::MediaFadingIn);
		}
		break;
	}

	case ECinematicState::MediaFadingIn:
	{
		// 새 클립이 첫 프레임을 내보내기 전엔 노출 금지.
		// (비동기 Open 중에는 MediaTexture에 이전 클립의 마지막 프레임이 남아있어 그게 보인다.)
		const bool bFramesFlowing = MediaPlayer
			&& MediaPlayer->IsPlaying()
			&& MediaPlayer->GetTime() > FTimespan::Zero();
		if (!bFramesFlowing)
		{
			ApplyOpacity(1.f, 0.f); // 검정 유지, 미디어 숨김
			ElapsedInState = 0.f;   // 프레임 들어올 때까지 페이드 타이머 멈춤
			break;
		}

		const float Dur = GetSegmentDuration(CurrentState);
		const float A = FMath::Clamp(ElapsedInState / Dur, 0.f, 1.f);
		ApplyOpacity(1.f, A);
		if (ElapsedInState >= Dur)
		{
			EnterState(ECinematicState::Playing);
		}
		break;
	}

	case ECinematicState::Playing:
	{
		// 검정은 미디어 아래 깔린 상태 유지(게임 누출 방지).
		ApplyOpacity(1.f, 1.f);

		const bool bDurationCapped = (ActiveRequest.Duration > KINDA_SMALL_NUMBER)
			&& (ElapsedInState >= ActiveRequest.Duration);

		if (bMediaEnded || bDurationCapped)
		{
			EnterState(ECinematicState::MediaFadingOut);
		}
		break;
	}

	case ECinematicState::MediaFadingOut:
	{
		const float Dur = GetSegmentDuration(CurrentState);
		const float A = FMath::Clamp(ElapsedInState / Dur, 0.f, 1.f);
		ApplyOpacity(1.f, 1.f - A);
		if (ElapsedInState >= Dur)
		{
			EnterState(ECinematicState::HoldingBlack);
		}
		break;
	}

	case ECinematicState::HoldingBlack:
		// 외부 ReleaseHold 대기. 시간 기반 전이 없음.
		break;

	case ECinematicState::FadingFromBlack:
	{
		const float Dur = GetSegmentDuration(CurrentState);
		const float A = FMath::Clamp(ElapsedInState / Dur, 0.f, 1.f);
		ApplyOpacity(1.f - A, 0.f);
		if (ElapsedInState >= Dur)
		{
			FinishAndCleanup(true);
		}
		break;
	}

	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// 미디어
// ---------------------------------------------------------------------------

void UCinematicManagerSubsystem::StartMedia()
{
	UMediaSource* Source = ResolveMediaSource(ActiveRequest.MediaSource);
	if (!Source)
	{
		return;
	}

	EnsureMediaObjects();

	// 프리롤: 검정 페이드 동안 미리 연다. 실제 Play는 MediaFadingIn 진입 시.
	MediaPlayer->OpenSource(Source);
}

void UCinematicManagerSubsystem::EnsureMediaObjects()
{
	if (!MediaPlayer)
	{
		MediaPlayer = NewObject<UMediaPlayer>(this);
		// 검정 페이드 동안 미리 Open만 하고, 실제 Play는 MediaFadingIn 진입 시 명시 호출.
		// (기본 PlayOnOpen=true면 FadeToBlack 구간에서 영상이 먼저 흘러버린다.)
		MediaPlayer->PlayOnOpen = false;
		MediaPlayer->OnEndReached.AddDynamic(this, &UCinematicManagerSubsystem::HandleMediaEndReached);
		MediaPlayer->OnMediaOpened.AddDynamic(this, &UCinematicManagerSubsystem::HandleMediaOpened);
	}

	if (!MediaTexture)
	{
		MediaTexture = NewObject<UMediaTexture>(this);
		MediaTexture->AutoClear = true;
		MediaTexture->SetMediaPlayer(MediaPlayer);
		MediaTexture->UpdateResource();
	}

	UWorld* World = GetWorld();
	if (!MediaSoundComp)
	{
		MediaSoundComp = NewObject<UMediaSoundComponent>(this);
		MediaSoundComp->SetMediaPlayer(MediaPlayer);
		MediaSoundComp->SetVolumeMultiplier(1.0f);
		if (World)
		{
			MediaSoundComp->RegisterComponentWithWorld(World);
		}
	}
	else if (World && MediaSoundComp->GetWorld() != World)
	{
		// 맵 전환으로 월드가 바뀌면 사운드 컴포넌트를 새 월드에 재등록.
		MediaSoundComp->UnregisterComponent();
		MediaSoundComp->RegisterComponentWithWorld(World);
	}
}

UMediaSource* UCinematicManagerSubsystem::ResolveMediaSource(const TSoftObjectPtr<UMediaSource>& SoftSource)
{
	const FSoftObjectPath Path = SoftSource.ToSoftObjectPath();
	if (Path.IsNull())
	{
		return nullptr;
	}

	if (TObjectPtr<UMediaSource>* Found = CachedMediaSources.Find(Path))
	{
		if (*Found)
		{
			return *Found;
		}
	}

	// 1차: 동기 로드 (추후 RequestAsyncLoad 프리로드로 교체 예정).
	UMediaSource* Loaded = SoftSource.LoadSynchronous();
	if (Loaded)
	{
		CachedMediaSources.Add(Path, Loaded);
	}
	return Loaded;
}

void UCinematicManagerSubsystem::HandleMediaEndReached()
{
	bMediaEnded = true;

	// 재생 중 자연 종료가 오면 즉시 페이드아웃 (Duration 캡보다 우선).
	if (CurrentState == ECinematicState::Playing)
	{
		EnterState(ECinematicState::MediaFadingOut);
	}
}

void UCinematicManagerSubsystem::HandleMediaOpened(FString OpenedUrl)
{
	bMediaOpened = true;

	if (bPendingMediaPlay && MediaPlayer)
	{
		if (MediaSoundComp)
		{
			MediaSoundComp->Start();
		}
		MediaPlayer->Play();
		bPendingMediaPlay = false;
	}
}

// ---------------------------------------------------------------------------
// 커버 위젯 / 입력 / 정리
// ---------------------------------------------------------------------------

bool UCinematicManagerSubsystem::HasLocalViewport() const
{
	return GEngine != nullptr && GEngine->GameViewport != nullptr;
}

void UCinematicManagerSubsystem::CreateOverlay()
{
	if (OverlayWidget)
	{
		return;
	}

	const UCinematicSettings* Settings = GetDefault<UCinematicSettings>();
	UClass* WidgetClass = Settings ? Settings->OverlayWidgetClass.LoadSynchronous() : nullptr;
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cinematic] CinematicSettings.OverlayWidgetClass 미지정 - 커버 위젯 없이 진행."));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	OverlayWidget = CreateWidget<UCinematicOverlayWidget>(GI, WidgetClass);
	if (OverlayWidget)
	{
		OverlayWidget->AddToViewport(GetUILayerZOrder(EUILayer::Cinematic));
	}
}

void UCinematicManagerSubsystem::DestroyOverlay()
{
	if (OverlayWidget)
	{
		OverlayWidget->RemoveFromParent();
		OverlayWidget = nullptr;
	}
}

void UCinematicManagerSubsystem::ApplyOpacity(float BlackAlpha, float MediaAlpha)
{
	if (OverlayWidget)
	{
		OverlayWidget->SetBlackOpacity(BlackAlpha);
		OverlayWidget->SetMediaOpacity(MediaAlpha);
	}
}

void UCinematicManagerSubsystem::BlockInput()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (APlayerController* PC = GI->GetFirstLocalPlayerController())
	{
		PC->DisableInput(PC);
		// TODO(사운드): 게임 오디오 SoundMix 덕킹/일시정지. 1차 범위 밖.
	}
}

void UCinematicManagerSubsystem::CheckSkipInput()
{
	if (!ActiveRequest.bSkippable)
	{
		return;
	}

	if (CurrentState != ECinematicState::MediaFadingIn && CurrentState != ECinematicState::Playing)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	APlayerController* PC = GI->GetFirstLocalPlayerController();
	if (!PC || !PC->PlayerInput)
	{
		return;
	}

	if (PC->PlayerInput->WasJustPressed(EKeys::SpaceBar))
	{
		PRINTLOG_SH(TEXT("Cinematic skip input detected (PlayId=%d)"), ActivePlayId);
		RequestSkip(ActivePlayId);
	}
}

void UCinematicManagerSubsystem::RestoreInput()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (APlayerController* PC = GI->GetFirstLocalPlayerController())
	{
		PC->EnableInput(PC);
	}
}

void UCinematicManagerSubsystem::FinishAndCleanup(bool bBroadcastCompleted)
{
	const int32 FinishedPlayId = ActivePlayId;

	// 캐싱: 미디어 객체는 파괴하지 않고 다음 재생에서 재사용. 스트림만 닫는다.
	if (MediaPlayer)
	{
		MediaPlayer->Close();
	}
	// 스트림만 닫으면 사운드 컴포넌트 잔여 버퍼가 인게임에서 루프됨 -> 정지.
	if (MediaSoundComp)
	{
		MediaSoundComp->Stop();
	}

	DestroyOverlay();
	RestoreInput();

	CurrentState = ECinematicState::Idle;
	ActivePlayId = INDEX_NONE;
	ElapsedInState = 0.f;
	bMediaEnded = false;
	bMediaOpened = false;
	bPendingMediaPlay = false;
	bAutoReleaseHold = false;
	ActiveRequest = FCinematicPlayRequest();

	if (bBroadcastCompleted)
	{
		OnCompleted.Broadcast(FinishedPlayId);
	}
}
