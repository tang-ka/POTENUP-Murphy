#include "Framework/STTPlayerController.h"
#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"
#include "VoiceChat/STTWebSocketComponent.h"
#include "VoiceChat/VoiceRecorderComponent.h"
#include "Manager/AIBridgeSubsystem.h"
#include "EnhancedInputComponent.h"
#include "Murphy.h"

ASTTPlayerController::ASTTPlayerController()
{
	STTWebSocketComp = CreateDefaultSubobject<USTTWebSocketComponent>(TEXT("STTWebSocketComp"));
}

void ASTTPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// WebSocket 컴포넌트 델리게이트 바인딩
	if (ensure(STTWebSocketComp))
	{
		STTWebSocketComp->OnSubtitleUpdated.AddDynamic(this, &ASTTPlayerController::OnSubtitleUpdated);
		STTWebSocketComp->OnFinalTranscriptReady.AddDynamic(this, &ASTTPlayerController::OnFinalTranscriptReady);
		STTWebSocketComp->OnSTTError.AddDynamic(this, &ASTTPlayerController::OnSTTError);
	}

	// Player Pawn의 VoiceRecorderComponent 이벤트 바인딩
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (UVoiceRecorderComponent* VoiceComp = MurphyPlayer->GetVoiceRecorderComp())
		{
			VoiceComp->OnAudioChunkReady.AddDynamic(this, &ASTTPlayerController::OnAudioChunkReady);
		}
	}
}

void ASTTPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// STTTestPlayer가 입력을 단독으로 받을 수 있도록, 컨트롤러에서의 입력 가로채기(BindAction)를 제거했습니다.
}

void ASTTPlayerController::OnSTTStartPressed(const FInputActionValue& Value)
{
	AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn());
	if (!MurphyPlayer || !STTWebSocketComp) return;

	UVoiceRecorderComponent* VoiceComp = MurphyPlayer->GetVoiceRecorderComp();
	if (!VoiceComp) return;

	// 중복 실행 방지: Idle 상태일 때만 시작 가능
	if (MurphyPlayer->GetChatState() != EPlayerChatState::Idle)
	{
		PRINTLOGW_JW(TEXT("[STTPlayerController] 현재 대화/녹음 상태이므로 STT 세션을 시작할 수 없습니다."));
		return;
	}

	if (STTWebSocketComp->IsConnected())
	{
		PRINTLOGW_JW(TEXT("[STTPlayerController] 이미 STT 세션 진행 중입니다."));
		return;
	}

	// TargetNPC가 없으면 말을 걸 수 없도록 방어
	if (!GetTargetNPC())
	{
		PRINTLOGW_JW(TEXT("[STTPlayerController] 대화할 NPC가 없습니다."));
		return;
	}

	PRINTLOGW_JW(TEXT("[STTPlayerController] STT 세션 시작"));

	// STT 세션 활성 플래그 세팅 - WAV 경로의 ForShortAnswer 방어
	MurphyPlayer->SetSTTSessionActive(true);

	// BeginPlay 시점에는 GetPawn()이 null일 수 있어 바인딩이 누락되므로 여기서 확실히 바인딩
	VoiceComp->OnAudioChunkReady.RemoveDynamic(this, &ASTTPlayerController::OnAudioChunkReady);
	VoiceComp->OnAudioChunkReady.AddDynamic(this, &ASTTPlayerController::OnAudioChunkReady);

	PrepareTestTurnData();

	FSTT_SessionStart SessionPayload;
	SessionPayload.request_id      = CachedTurnData.request_id;
	SessionPayload.session_id      = CachedTurnData.session.session_id;
	SessionPayload.turn_index      = CachedTurnData.session.turn_index;
	SessionPayload.chapter_id      = CachedTurnData.session.chapter_id;
	SessionPayload.scene_id        = CachedTurnData.session.scene_id;
	SessionPayload.current_node_id = CachedTurnData.session.current_node_id;

	VoiceComp->SetChunkingMode(true, 100);
	STTWebSocketComp->Connect(SessionPayload);
	VoiceComp->StartRecording();

	MurphyPlayer->SetChatState(EPlayerChatState::Recording);
}

void ASTTPlayerController::OnSTTStopPressed(const FInputActionValue& Value)
{
	AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn());
	if (!MurphyPlayer) return;

	UVoiceRecorderComponent* VoiceComp = MurphyPlayer->GetVoiceRecorderComp();
	if (!VoiceComp || !VoiceComp->IsRecording())
	{
		return;
	}

	PRINTLOGW_JW(TEXT("[STTPlayerController] STT 녹음 정지 (마지막 청크 commit 예약)"));

	VoiceComp->StopRecording(TEXT(""), false);
	MurphyPlayer->SetChatState(EPlayerChatState::WaitingForAI);
}

void ASTTPlayerController::OnAudioChunkReady(const TArray<uint8>& PCM16Chunk, bool bIsLastChunk)
{
	if (STTWebSocketComp && STTWebSocketComp->IsConnected())
	{
		STTWebSocketComp->SendAudioChunk(PCM16Chunk, bIsLastChunk);
	}
}

void ASTTPlayerController::OnSubtitleUpdated(const FString& Text, bool bIsFinal)
{
	PRINTLOGW_JW(TEXT("[STT 자막%s] \"%s\""), bIsFinal ? TEXT(" (FINAL)") : TEXT(""), *Text);
}

void ASTTPlayerController::OnFinalTranscriptReady(const FString& FinalText)
{
	PRINTLOGW_JW(TEXT("[STTPlayerController] final_transcript 확정: \"%s\" → /respond 호출"), *FinalText);

	UAIBridgeSubsystem* NetSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>() : nullptr;
	if (!NetSub) return;

	// NPC에게 플레이어가 말을 끝마쳤음을 알림 (타이핑 대기 UI 연출 등)
	if (AAgentNPCBase* NPC = GetTargetNPC())
	{
		NPC->NotifyPlayerSpoke();
	}
    
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		MurphyPlayer->SetMicUIState(false);
	}

	FOnAIResponseDataReceived ResponseDelegate;
	ResponseDelegate.BindDynamic(this, &ASTTPlayerController::OnAIRespondReceived);
	NetSub->SendToAIWithTranscript(CachedTurnData, FinalText, ResponseDelegate);
}

void ASTTPlayerController::OnAIRespondReceived(const FAIResponseData& ResponseData)
{
	PRINTLOGW_JW(TEXT("[STTPlayerController] /respond 응답 수신"));

	// NPC에게 응답 데이터를 전달해 애니메이션/TTS 재생 처리
	if (AAgentNPCBase* NPC = GetTargetNPC())
	{
		NPC->ProcessDialogueResponse(ResponseData);
	}

	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		// STT 세션 종료
		MurphyPlayer->SetSTTSessionActive(false);

		MurphyPlayer->SetChatState(EPlayerChatState::Idle);
		MurphyPlayer->EndChatWithNPC();
		MurphyPlayer->SetMicUIState(true);
	}

	if (STTWebSocketComp)
	{
		STTWebSocketComp->Disconnect();
	}
}

void ASTTPlayerController::OnSTTError(const FString& ErrorType, const FString& Message)
{
	PRINTLOGE_JW(TEXT("[STTPlayerController] STT 에러 [%s]: %s"), *ErrorType, *Message);

	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		// STT 세션 종료
		MurphyPlayer->SetSTTSessionActive(false);

		MurphyPlayer->SetChatState(EPlayerChatState::Idle);
		if (UVoiceRecorderComponent* VoiceComp = MurphyPlayer->GetVoiceRecorderComp())
		{
			if (VoiceComp->IsRecording())
			{
				VoiceComp->StopRecording(TEXT(""), false);
			}
		}
	}

	if (STTWebSocketComp)
	{
		STTWebSocketComp->Disconnect();
	}
}

void ASTTPlayerController::PrepareTestTurnData()
{
	// 부모 클래스(MurphyPlayerController)의 최신 턴 데이터 생성 함수 호출
	CachedTurnData = GenerateAIRequestData();

	// WebSocket STT를 위한 오디오 설정만 오버라이드
	CachedTurnData.audio.mime_type                       = TEXT("audio/wav");
	CachedTurnData.audio.sample_rate_hz                  = 16000;
	CachedTurnData.audio.channels                        = 1;
	CachedTurnData.audio.duration_ms                     = 3200; // Chunking Mode에서는 의미 없지만 더미로 세팅
	
	// 서버의 대화 트리(Graph)가 자유롭게 다음 노드를 선택할 수 있도록 제한 해제
	CachedTurnData.client_allowed_next_nodes.Empty();
}
