// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Characters/STTTestPlayer.h"

#include "Murphy.h"
#include "EnhancedInputComponent.h"
#include "VoiceChat/VoiceRecorderComponent.h"
#include "VoiceChat/STTWebSocketComponent.h"
#include "Manager/NetSubsystem.h"

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

	UNetSubsystem* NetSub = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UNetSubsystem>()
		: nullptr;

	if (!NetSub)
	{
		PRINTLOGE_JW(TEXT("[STTTestPlayer] NetSubsystem을 찾을 수 없습니다."));
		SetChatState(EPlayerChatState::Idle);
		return;
	}

	// final transcript를 CachedTurnData와 합쳐서 /respond 호출
	// DYNAMIC_DELEGATE는 CreateLambda 미지원 → BindDynamic + UFUNCTION 사용
	FOnAIResponseDataReceived ResponseDelegate;
	ResponseDelegate.BindDynamic(this, &ASTTTestPlayer::OnAIRespondReceived);
	NetSub->SendToAIWithTranscript(CachedTurnData, FinalText, ResponseDelegate);
}

void ASTTTestPlayer::OnSTTError(const FString& ErrorType, const FString& Message)
{
	PRINTLOGE_JW(TEXT("[STTTestPlayer] STT 에러 [%s]: %s"), *ErrorType, *Message);
	PRINTLOGW_JW(TEXT("[STTTestPlayer] 필요 시 기존 WAV 방식(IA_Record)으로 fallback 가능합니다."));

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
	// 실제 게임에서는 ScenarioSubsystem / DataManager에서 현재 노드 정보를 받아 채울 것.
	// 현재는 테스트용 하드코딩 값 사용.
	CachedTurnData = FAIRequestData();
	CachedTurnData.contract_version                      = TEXT("dev_c_unreal_turn.v1");
	CachedTurnData.request_id                            = TEXT("req_turn_stt_test_001");
	CachedTurnData.session.session_id                    = TEXT("session_test_001");
	CachedTurnData.session.player_id                     = TEXT("player_test");
	CachedTurnData.session.chapter_id                    = TEXT("CH0_03_IMMIGRATION_CHECK");
	CachedTurnData.session.scene_id                      = TEXT("JFK_IMMIGRATION_HALL");
	CachedTurnData.session.current_node_id               = TEXT("IMM_003_DURATION");
	CachedTurnData.session.turn_index                    = 3;
	CachedTurnData.npc.npc_id                            = TEXT("hale");
	CachedTurnData.npc.npc_role                          = TEXT("immigration_officer");
	CachedTurnData.npc.last_npc_message                  = TEXT("How long will you stay?");
	CachedTurnData.audio.mime_type                       = TEXT("audio/wav");
	CachedTurnData.audio.sample_rate_hz                  = 16000;
	CachedTurnData.audio.channels                        = 1;
	CachedTurnData.audio.duration_ms                     = 3200;
	CachedTurnData.audio.language_hint                   = TEXT("en");
	// CachedTurnData.interaction.contract_version          = TEXT("dev_c_interaction_context.v1");
	CachedTurnData.interaction.initiator                 = TEXT("npc");
	CachedTurnData.interaction.interaction_type          = TEXT("quest");
	// CachedTurnData.interaction.quest_id                  = TEXT("immigration_check");
	// CachedTurnData.interaction.interaction_id            = TEXT("imm_duration_turn");
	CachedTurnData.interaction.time_limit_s              = 30;
	CachedTurnData.interaction.first_contact             = false;
	CachedTurnData.player_profile.nickname               = TEXT("Player");
	CachedTurnData.player_profile.english_confidence     = TEXT("beginner");
	CachedTurnData.player_profile.tier                   = TEXT("Bronze");
	CachedTurnData.player_profile.travel_speaking_level  = TEXT("TSL_1_SURVIVAL");
	CachedTurnData.scenario_state.patience               = 100;
	CachedTurnData.scenario_state.suspicion              = 0;
	CachedTurnData.scenario_state.retry_count            = 0;
	CachedTurnData.scenario_state.hint_count             = 0;
	CachedTurnData.game_state.current_objective          = TEXT("Answer the officer's question.");
	// 서버의 대화 트리(Graph)가 자유롭게 다음 노드를 선택할 수 있도록 제한 해제
	CachedTurnData.client_allowed_next_nodes.Empty();

	PRINTLOGW_JW(TEXT("[STTTestPlayer] TurnData 준비 완료: req_id=%s"), *CachedTurnData.request_id);
}
