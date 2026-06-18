// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Characters/STTTestPlayer.h"

#include "Murphy.h"
#include "EnhancedInputComponent.h"
#include "Framework/MurphyPlayerController.h"
#include "VoiceChat/VoiceRecorderComponent.h"
#include "VoiceChat/STTWebSocketComponent.h"
#include "Manager/AIBridgeSubsystem.h"

ASTTTestPlayer::ASTTTestPlayer()
{
	// VoiceRecorderComp는 부모(AMurphyPlayer)에서 이미 CreateDefaultSubobject로 생성됨
	// 중복 생성하지 않는다.

	STTWebSocketComp = CreateDefaultSubobject<USTTWebSocketComponent>(TEXT("STTWebSocketComp"));
}

void ASTTTestPlayer::BeginPlay()
{
	Super::BeginPlay();

	// STT WebSocket 델리게이트 바인딩
	if (ensure(STTWebSocketComp))
	{
		STTWebSocketComp->OnSubtitleUpdated.AddDynamic(this, &ASTTTestPlayer::OnSubtitleUpdated);
		STTWebSocketComp->OnFinalTranscriptReady.AddDynamic(this, &ASTTTestPlayer::OnFinalTranscriptReady);
		STTWebSocketComp->OnSTTError.AddDynamic(this, &ASTTTestPlayer::OnSTTError);
	}

	// VoiceRecorderComp Chunk 델리게이트 바인딩 (STTTestPlayer 단독 동작을 위해 복구)
	if (ensure(VoiceRecorderComp))
	{
		VoiceRecorderComp->OnAudioChunkReady.RemoveDynamic(this, &ASTTTestPlayer::OnAudioChunkReady);
		VoiceRecorderComp->OnAudioChunkReady.AddDynamic(this, &ASTTTestPlayer::OnAudioChunkReady);
	}
}

void ASTTTestPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 부모의 Move / Look / IA_Record(WAV 방식) / Bag / Phone / Interact 바인딩을 그대로 유지
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 새 STT 전용 InputAction 바인딩
	if (auto* PlayerInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA_STTStart: 누르는 순간 → STT 세션 시작
		if (IA_STTStart)
		{
			PlayerInput->BindAction(IA_STTStart, ETriggerEvent::Started, this, &ASTTTestPlayer::OnSTTStartPressed);
		}
		else
		{
			PRINTLOGW_JW(TEXT("[STTTestPlayer] IA_STTStart가 할당되지 않았습니다. 에디터 BP에서 할당하세요."));
		}

		// IA_STTStop: 누르는 순간 → STT 세션 종료 (누르고 떼는 방식이 아닌 토글형)
		if (IA_STTStop)
		{
			PlayerInput->BindAction(IA_STTStop, ETriggerEvent::Started, this, &ASTTTestPlayer::OnSTTStopPressed);
		}
		else
		{
			PRINTLOGW_JW(TEXT("[STTTestPlayer] IA_STTStop이 할당되지 않았습니다. 에디터 BP에서 할당하세요."));
		}
	}
}

// ----------------------------------------------------------------
// STT WebSocket 핸들러
// ----------------------------------------------------------------

void ASTTTestPlayer::OnSTTStartPressed(const FInputActionValue& Value)
{
	if (!STTWebSocketComp || !VoiceRecorderComp)
	{
		PRINTLOGE_JW(TEXT("[STTTestPlayer] 컴포넌트 null. STT 시작 불가."));
		return;
	}

	if (STTWebSocketComp->IsConnected())
	{
		PRINTLOGW_JW(TEXT("[STTTestPlayer] 이미 STT 세션 진행 중입니다."));
		return;
	}

	if (CurChatState != EPlayerChatState::Idle)
	{
		PRINTLOGW_JW(TEXT("[STTTestPlayer] Idle 상태가 아닙니다. 현재: %d"), (int32)CurChatState);
		return;
	}

	PRINTLOGW_JW(TEXT("[STTTestPlayer] STT 세션 시작"));

	// STT 세션 활성 플래그 세팅 - WAV 경로의 ForShortAnswer 방어
	SetSTTSessionActive(true);

	// 1) 테스트용 TurnData 준비 (실제 게임에서는 ScenarioSubsystem에서 받아올 것)
	PrepareTestTurnData();

	// 2) session_start 페이로드 구성
	FSTT_SessionStart SessionPayload;
	SessionPayload.request_id      = CachedTurnData.request_id;
	SessionPayload.session_id      = CachedTurnData.session.session_id;
	SessionPayload.turn_index      = CachedTurnData.session.turn_index;
	SessionPayload.chapter_id      = CachedTurnData.session.chapter_id;
	SessionPayload.scene_id        = CachedTurnData.session.scene_id;
	SessionPayload.current_node_id = CachedTurnData.session.current_node_id;

	// 3) VoiceRecorderComp Chunking Mode ON (100ms 단위 청크)
	VoiceRecorderComp->SetChunkingMode(true, 100);

	// 4) WebSocket 연결 (연결 성공 후 session_start 자동 전송)
	STTWebSocketComp->Connect(SessionPayload);

	// 5) 마이크 녹음 시작
	VoiceRecorderComp->StartRecording();

	SetChatState(EPlayerChatState::Recording);
}

void ASTTTestPlayer::OnSTTStopPressed(const FInputActionValue& Value)
{
	if (!VoiceRecorderComp || !VoiceRecorderComp->IsRecording())
	{
		PRINTLOGW_JW(TEXT("[STTTestPlayer] 녹음 중이 아닙니다."));
		return;
	}

	PRINTLOGW_JW(TEXT("[STTTestPlayer] STT 녹음 정지 (마지막 청크 commit=true 예약)"));

	// StopRecording 내부에서 ProcessAndBroadcastChunk(bIsLast=true)가 호출되어
	// 나머지 버퍼를 commit=true로 flush한다.
	// WAV 저장 안 함 (bSaveToWav=false)
	VoiceRecorderComp->StopRecording(TEXT(""), false);

	// WS는 열린 채로 유지 → 서버가 final_transcript 보낼 때까지 대기
	// final_transcript 수신 후 OnFinalTranscriptReady에서 /respond 호출
	SetChatState(EPlayerChatState::WaitingForAI);
}

// ----------------------------------------------------------------
// 델리게이트 콜백
// ----------------------------------------------------------------

void ASTTTestPlayer::OnAudioChunkReady(const TArray<uint8>& PCM16Chunk, bool bIsLastChunk)
{
	if (!STTWebSocketComp)
	{
		return;
	}
	STTWebSocketComp->SendAudioChunk(PCM16Chunk, bIsLastChunk);
}

void ASTTTestPlayer::OnSubtitleUpdated(const FString& Text, bool bIsFinal)
{
	// 자막 UI 연결 전: 로그로 확인
	// 추후 MurphyPlayer의 SetMicUIState 또는 별도 자막 위젯에 연결
	PRINTLOGW_JW(TEXT("[STT 자막%s] \"%s\""), bIsFinal ? TEXT(" (FINAL)") : TEXT(""), *Text);
}

void ASTTTestPlayer::OnFinalTranscriptReady(const FString& FinalText)
{
	PRINTLOGW_JW(TEXT("[STTTestPlayer] final_transcript 확정: \"%s\" → /respond 호출"), *FinalText);

	UAIBridgeSubsystem* NetSub = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>()
		: nullptr;

	if (!NetSub)
	{
		PRINTLOGE_JW(TEXT("[STTTestPlayer] NetSubsystem을 찾을 수 없습니다."));
		SetChatState(EPlayerChatState::Idle);
		return;
	}
	
	AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(GetController());
	if (!PC)
	{
		PRINTLOGE_JW(TEXT("[STTTestPlayer] MurphyPlayerController를 찾을 수 없습니다."));
		SetChatState(EPlayerChatState::Idle);
		return;
	}

	// final transcript를 CachedTurnData와 합쳐서 /respond 호출
	// DYNAMIC_DELEGATE는 CreateLambda 미지원 → BindDynamic + UFUNCTION 사용
	FOnAIResponseDataReceived ResponseDelegate;
	// ResponseDelegate.BindDynamic(this, &ASTTTestPlayer::OnAIRespondReceived);
	ResponseDelegate.BindDynamic(PC, &AMurphyPlayerController::OnAIResponseReceived);
	NetSub->SendToAIWithTranscript(CachedTurnData, FinalText, ResponseDelegate);
	
	// final_transcript 수신 완료 → WebSocket 및 STT 세션 즉시 정리
	// (AI 응답 대기 중에는 WebSocket 불필요)
	if (STTWebSocketComp)
	{
		STTWebSocketComp->Disconnect();
	}
	SetSTTSessionActive(false);
	SetChatState(EPlayerChatState::WaitingForAI);
}

void ASTTTestPlayer::OnSTTError(const FString& ErrorType, const FString& Message)
{
	PRINTLOGE_JW(TEXT("[STTTestPlayer] STT 에러 [%s]: %s"), *ErrorType, *Message);
	PRINTLOGW_JW(TEXT("[STTTestPlayer] 필요 시 기존 WAV 방식(IA_Record)으로 fallback 가능합니다."));

	// STT 세션 종료
	SetSTTSessionActive(false);

	// 상태 복구
	SetChatState(EPlayerChatState::Idle);

	// WebSocket 정리
	if (STTWebSocketComp)
	{
		STTWebSocketComp->Disconnect();
	}

	// VoiceRecorder가 녹음 중이었다면 정리
	if (VoiceRecorderComp && VoiceRecorderComp->IsRecording())
	{
		VoiceRecorderComp->StopRecording(TEXT(""), false);
	}
}

void ASTTTestPlayer::OnAIRespondReceived(const FAIResponseData& ResponseData)
{
	PRINTLOGW_JW(TEXT("[STTTestPlayer] /respond 응답 수신 - next_action: %s, next_node: %s"),
		*ResponseData.next_action, *ResponseData.next_node_id);

	// STT 세션 종료
	SetSTTSessionActive(false);

	// 상태 복구 + WebSocket 세션 종료
	SetChatState(EPlayerChatState::Idle);
	if (STTWebSocketComp)
	{
		STTWebSocketComp->Disconnect();
	}
}

// ----------------------------------------------------------------
// 내부 유틸
// ----------------------------------------------------------------

void ASTTTestPlayer::PrepareTestTurnData()
{
	AMurphyPlayerController* PC = Cast<AMurphyPlayerController>(GetController());
	if (PC)
	{
		// MurphyPlayerController에서 최신 턴 데이터 생성
		CachedTurnData = PC->GenerateAIRequestData();

		// WebSocket STT를 위한 오디오 설정 오버라이드
		CachedTurnData.audio.mime_type                       = TEXT("audio/wav");
		CachedTurnData.audio.sample_rate_hz                  = 16000;
		CachedTurnData.audio.channels                        = 1;
		CachedTurnData.audio.duration_ms                     = 3200; // Chunking Mode에서는 의미 없지만 더미로 세팅
		
		// 서버의 대화 트리(Graph)가 자유롭게 다음 노드를 선택할 수 있도록 제한 해제
		CachedTurnData.client_allowed_next_nodes.Empty();

		PRINTLOGW_JW(TEXT("[STTTestPlayer] TurnData 준비 완료: req_id=%s, current_node=%s"), *CachedTurnData.request_id, *CachedTurnData.session.current_node_id);
	}
	else
	{
		PRINTLOGE_JW(TEXT("[STTTestPlayer] AMurphyPlayerController를 찾을 수 없어 턴 데이터를 생성할 수 없습니다."));
	}
}
