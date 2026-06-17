
#include "Framework/MurphyPlayerController.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"

#include "VoiceChat/VoiceRecorderComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "Json.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"

#include "Murphy.h"
#include "Manager/CinematicManagerSubsystem.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Manager/AIBridgeSubsystem.h"
#include "Manager/ScenarioSubsystem.h"
#include "Manager/UIManagerSubsystem.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AMurphyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AMurphyPlayer* MainPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (IsValid(MainPlayer->GetVoiceRecorderComp()))
		{
			MainPlayer->GetVoiceRecorderComp()->OnRecordingFinished.AddDynamic(this, &AMurphyPlayerController::OnAudioRecordingFinished);
			PRINTLOG_JW(TEXT("OnRecordingFinished 바인딩 완료"));
		}
	}
	
	if (IsLocalController())
	{
		SubscribeLevelEnterEvents();
	}
}

void AMurphyPlayerController::SetActiveNPC(AAgentNPCBase* NewNPC)
{
	TargetNPC = NewNPC;
	
	// 오버랩에 따른 마이크 UI 상태(활성화/비활성화) 업데이트
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		MurphyPlayer->SetMicUIState(TargetNPC != nullptr);
	}
}

void AMurphyPlayerController::OnAudioRecordingFinished(const FString& SavedFilePath)
{
	if (!IsValid(TargetNPC)) return;

	AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn());

	if (MurphyPlayer)
	{
		// realtime STT 세션 활성 중이면 WAV 길이 기반 판정 전체 스킵
		if (MurphyPlayer->IsSTTSessionActive())
		{
			PRINTLOG_SH(TEXT("[MurphyController] STT 세션 활성 중 - WAV 길이 판정 및 ForShortAnswer 스킵"));
			return;
		}

		if (MurphyPlayer->GetRecordTime() < 0.5f)
		{
			PRINTLOGW_JW(TEXT("[Voice Test] 녹음 시간이 너무 짧습니다. AI 서버로 전송하지 않고 기본 응답을 처리합니다."));

			TargetNPC->ForShortAnswer();
			MurphyPlayer->EndChatWithNPC();
			
			// 최소 v0.1.0 응답 포맷으로 더미 JSON 생성/
			// FString SimulatedJSONResponse = TEXT("{\"npc\":{\"speaker\":\"System\",\"text\":\"잘 못 들었어. 조금만 더 길게 말해줄래?\",\"tone\":\"neutral\",\"animation\":\"\",\"audio_url\":\"\"}}");
			// Test_SimulateAIResponse(SimulatedJSONResponse);
			return;
		}
	}
	
	TargetNPC->NotifyPlayerSpoke();
	
	if (UAIBridgeSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>())
	{
		FOnAIResponseDataReceived Callback;
		Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResponseReceived);
		
		FAIRequestData RequestData;
		RequestData.contract_version = TEXT("dev_c_unreal_turn.v1");
		RequestData.request_id = FGuid::NewGuid().ToString();
		
		RequestData.session.session_id = CurrentSessionId;
		RequestData.session.player_id = TEXT("player_001");
		// RequestData.session.chapter_id = TEXT("CH0_IMMIGRATION");
		RequestData.session.chapter_id = TEXT("CH0_03_IMMIGRATION_CHECK");
		RequestData.session.scene_id = TEXT("JFK_IMMIGRATION_HALL");
		RequestData.session.current_node_id = CurrentNodeId;
		RequestData.session.turn_index = TurnIndex;
		
		RequestData.npc.npc_id = TargetNPC->GetNPCName(); // RequestData.npc.npc_id = TEXT("OFFICER_MILLER");
		RequestData.npc.npc_role = TEXT("immigration_officer");
		RequestData.npc.last_npc_message = LastNpcMessage;
		
		RequestData.audio.mime_type = TEXT("audio/wav");
		RequestData.audio.sample_rate_hz = 48000;
		RequestData.audio.channels = 2;
		RequestData.audio.duration_ms = 2800;
		RequestData.audio.language_hint = TEXT("en-US");
		
		RequestData.player_profile.nickname = TEXT("Sean");
		RequestData.player_profile.english_confidence = TEXT("beginner");
		RequestData.player_profile.tier = TEXT("Bronze");
		RequestData.player_profile.travel_speaking_level = TEXT("TSL_1_SURVIVAL");
		
		RequestData.scenario_state = CurrentScenarioState;
		
		RequestData.game_state.inventory = { TEXT("passport"), TEXT("boarding_pass"), TEXT("return_ticket") };
		RequestData.game_state.flags = { TEXT("arrived_at_jfk"), TEXT("passport_submitted") };
		RequestData.game_state.completed_intents = { TEXT("submit_passport") };
		RequestData.game_state.current_objective = TEXT("State the visit purpose");
		
		PRINTLOGW_JW(TEXT("[Voice Test] --- AI Request Before ---"));
		PRINTLOGW_JW(TEXT("request_id: %s"), *RequestData.request_id);
		PRINTLOGW_JW(TEXT("turn_index: %d"), RequestData.session.turn_index);
		PRINTLOGW_JW(TEXT("session.current_node_id: %s"), *RequestData.session.current_node_id);
		PRINTLOGW_JW(TEXT("npc.last_npc_message: %s"), *RequestData.npc.last_npc_message);
		PRINTLOGW_JW(TEXT("scenario_state - patience: %d, suspicion: %d, retry_count: %d, hint_count: %d"),
			RequestData.scenario_state.patience, RequestData.scenario_state.suspicion, RequestData.scenario_state.retry_count, RequestData.scenario_state.hint_count);

		PRINTLOGW_JW(TEXT("[Voice Test] NetSubsystem을 통해 서버로 오디오 전송 시작"));
		NetSubsystem->SendToAI(RequestData, SavedFilePath, Callback);
		
		if (MurphyPlayer)
		{
			// 마이크 UI 비활성화
			MurphyPlayer->SetMicUIState(false);
		}
	}
}

void AMurphyPlayerController::OnAIResponseReceived(const FAIResponseData& ResponseData)
{
	if (IsValid(TargetNPC))
	{
		TargetNPC->ProcessDialogueResponse(ResponseData);
		
		// 응답 수신 후 상태 갱신
		LastNpcMessage = ResponseData.npc.text;
		
		CurrentScenarioState.patience += ResponseData.state_delta.patience_delta;
		CurrentScenarioState.suspicion += ResponseData.state_delta.suspicion_delta;
		CurrentScenarioState.retry_count += ResponseData.state_delta.retry_count_delta;
		CurrentScenarioState.hint_count += ResponseData.state_delta.hint_count_delta;
		
		TurnIndex += 1;
		
		// if (ResponseData.next_action == TEXT("ADVANCE") && !ResponseData.next_node_id.IsEmpty())
		// {
		// 	CurrentNodeId = ResponseData.next_node_id;
		// }
		
		if (!ResponseData.current_node_id.IsEmpty())
		{
			CurrentNodeId = ResponseData.current_node_id;
		}

		if (ResponseData.next_action == TEXT("ADVANCE") && !ResponseData.next_node_id.IsEmpty())
		{
			CurrentNodeId = ResponseData.next_node_id;
		}
		
		// 필수 디버그 로그 추가 (응답 후)
		PRINTLOGW_JW(TEXT("[Voice Test] --- AI Response After ---"));
		PRINTLOGW_JW(TEXT("response.current_node_id: %s"), *ResponseData.current_node_id);
		PRINTLOGW_JW(TEXT("response.next_action: %s"), *ResponseData.next_action);
		PRINTLOGW_JW(TEXT("response.next_node_id: %s"), *ResponseData.next_node_id);
		PRINTLOGW_JW(TEXT("response.npc.text: %s"), *ResponseData.npc.text);
		PRINTLOGW_JW(TEXT("갱신된 Local CurrentNodeId: %s"), *CurrentNodeId);

		// AI 응답이 도착해 대화가 끝나면 NPC점유 해제 및 상태 초기화
		if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
		{
			MurphyPlayer->EndChatWithNPC();
			
			// 마이크 UI 활성화
			MurphyPlayer->SetMicUIState(true);
		}
	}
}

void AMurphyPlayerController::SendTimeoutAudioToAI()
{
	if (UAIBridgeSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>())
	{
		FOnAIResponseDataReceived Callback;
		Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResponseReceived);
		
		// OnAudioRecordingFinished와 동일하게 RequestData 세팅
		FAIRequestData RequestData;
		RequestData.contract_version = TEXT("dev_c_unreal_turn.v1");
		RequestData.request_id = FGuid::NewGuid().ToString();
		
		RequestData.session.session_id = CurrentSessionId;
		RequestData.session.player_id = TEXT("player_001");
		// RequestData.session.chapter_id = TEXT("CH0_IMMIGRATION");
		RequestData.session.chapter_id = TEXT("CH0_03_IMMIGRATION_CHECK");
		RequestData.session.scene_id = TEXT("JFK_IMMIGRATION_HALL");
		RequestData.session.current_node_id = CurrentNodeId;
		RequestData.session.turn_index = TurnIndex;
		
		RequestData.npc.npc_id = TEXT("OFFICER_MILLER");
		RequestData.npc.npc_role = TEXT("immigration_officer");
		RequestData.npc.last_npc_message = LastNpcMessage;
		
		RequestData.audio.mime_type = TEXT("audio/wav");
		RequestData.audio.sample_rate_hz = 48000;
		RequestData.audio.channels = 2;
		RequestData.audio.duration_ms = 2800;
		RequestData.audio.language_hint = TEXT("en-US");
		
		RequestData.player_profile.nickname = TEXT("Sean");
		RequestData.player_profile.english_confidence = TEXT("beginner");
		RequestData.player_profile.tier = TEXT("Bronze");
		RequestData.player_profile.travel_speaking_level = TEXT("TSL_1_SURVIVAL");
		
		RequestData.scenario_state = CurrentScenarioState;
		
		RequestData.game_state.inventory = { TEXT("passport"), TEXT("boarding_pass"), TEXT("return_ticket") };
		RequestData.game_state.flags = { TEXT("arrived_at_jfk"), TEXT("passport_submitted") };
		RequestData.game_state.completed_intents = { TEXT("submit_passport") };
		RequestData.game_state.current_objective = TEXT("State the visit purpose");
		
		PRINTLOGW_JW(TEXT("[Voice Test] --- AI Request Before ---"));
		PRINTLOGW_JW(TEXT("request_id: %s"), *RequestData.request_id);
		PRINTLOGW_JW(TEXT("turn_index: %d"), RequestData.session.turn_index);
		PRINTLOGW_JW(TEXT("session.current_node_id: %s"), *RequestData.session.current_node_id);
		PRINTLOGW_JW(TEXT("npc.last_npc_message: %s"), *RequestData.npc.last_npc_message);
		PRINTLOGW_JW(TEXT("scenario_state - patience: %d, suspicion: %d, retry_count: %d, hint_count: %d"),
			RequestData.scenario_state.patience, RequestData.scenario_state.suspicion, RequestData.scenario_state.retry_count, RequestData.scenario_state.hint_count);

		PRINTLOGW_JW(TEXT("[Voice Test] 타임아웃으로 빈 오디오 데이터를 서버로 전송합니다."));
		
		// 파일 경로를 빈 문자열 TEXT("")로 전달 (NetSubsystem 내부에서 빈 문자열이면 더미 데이터로 처리되도록 구현되어 있다고 가정)
		NetSubsystem->SendToAI(RequestData, TEXT(""), Callback);
	}
}

void AMurphyPlayerController::SubscribeLevelEnterEvents()
{
	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>();
	if (!LevelSubsystem)
	{
		PRINTLOG_SH(TEXT("SubscribeLevelEnterEvents: LevelStreamingSubsystem is null"));
		return;
	}

	// BeginPlay 시점에는 SubLevel_Immigration의 OnLevelShown만 구독
	// SubLevel_BaggageClaim 구독은 TransitionToBaggageClaim에서 처리
	ULevelStreaming* ImmigrationLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_Immigration"));
	if (ImmigrationLevel)
	{
		ImmigrationLevel->OnLevelShown.AddDynamic(this, &AMurphyPlayerController::OnImmigrationLevelShown);
	}
}

void AMurphyPlayerController::OnImmigrationLevelShown()
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>();
	if (!UIManager)
	{
		return;
	}

	UIManager->ShowLevelEnterToast(FText::FromString(TEXT("입국심사")));
}

void AMurphyPlayerController::TransitionToBaggageClaim()
{
	if (!IsLocalController())
	{
		return;
	}
	
	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>();
	if (!LevelSubsystem)
	{
		PRINTLOG_SH(TEXT("TransitionToBaggageClaim: LevelStreamingSubsystem is null"));
		return;
	}

	// BaggageClaim 레벨이 표시되면 플레이어 이동 & 토스트 출력
	ULevelStreaming* BaggageLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_BaggageClaim"));
	if (BaggageLevel)
	{
		BaggageLevel->OnLevelShown.AddDynamic(this, &AMurphyPlayerController::OnBaggageClaimLevelShown);
	}

	// Immigration 언로드 → 완료 콜백으로 BaggageClaim 로드
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName("OnImmigrationLevelHidden");
	LatentInfo.UUID = 2;
	LatentInfo.Linkage = 0;

	LevelSubsystem->UnloadSubLevel(TEXT("SubLevel_Immigration"), LatentInfo, false);
}

void AMurphyPlayerController::OnImmigrationLevelHidden()
{
	if (!IsLocalController())
	{
		return;
	}
	
	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>();
	if (!LevelSubsystem)
	{
		PRINTLOG_SH(TEXT("OnImmigrationLevelHidden: LevelStreamingSubsystem is null"));
		return;
	}

	LevelSubsystem->LoadSubLevel(TEXT("SubLevel_BaggageClaim"), true, false);
}

void AMurphyPlayerController::OnBaggageClaimLevelShown()
{
	if (!IsLocalController())
	{
		return;
	}

	// 플레이어를 PlayerStart[0] 위치로 이동

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UUIManagerSubsystem* UIManager = LP->GetSubsystem<UUIManagerSubsystem>();
	if (UIManager)
	{
		UIManager->ShowLevelEnterToast(FText::FromString(TEXT("수하물 수취장")));
	}

	Server_RequestReposition(TEXT("SubLevel_BaggageClaim"));
}

void AMurphyPlayerController::Client_PlayCinematic_Implementation(const FCinematicPlayRequest& Request, int32 PlayId)
{
	UCinematicManagerSubsystem* CinematicManager = GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>();
	if (!CinematicManager)
	{
		PRINTLOG_SH(TEXT("Client_PlayCinematic: CinematicManagerSubsystem is null"));
		return;
	}

	CinematicManager->PlayMedia(Request, PlayId, /*bInAutoReleaseHold*/ true);
}

void AMurphyPlayerController::Server_RequestReposition_Implementation(const FName& SubLevelName)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return;
	}

	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>();
	if (!LevelSubsystem)
	{
		return;
	}

	ULevelStreaming* BaggageLevel =	LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_BaggageClaim"));
	if (!BaggageLevel)
	{
		return;
	} 

	// BaggageClaim 서브레벨 소속 PlayerStart만 필터링
	ULevel* BaggageLoadedLevel = BaggageLevel->GetLoadedLevel();
	if (!BaggageLoadedLevel)
	{
		return;
	}

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	AActor** FoundStart = PlayerStarts.FindByPredicate([&](AActor* Actor)
	{
		return Actor->GetLevel() == BaggageLoadedLevel;
	});

	if (!FoundStart) return;

	MyPawn->SetActorLocationAndRotation(
		(*FoundStart)->GetActorLocation(),
		(*FoundStart)->GetActorRotation());
}

// ==============================================================================
// === 테스트 커맨드 ===
// ==============================================================================

void AMurphyPlayerController::Test_StartScenario(int32 ScenarioIndex)
{
	if (UScenarioSubsystem* ScenarioSubsys = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
	{
		ScenarioSubsys->StartScenario(static_cast<EScenarioType>(ScenarioIndex));
		PRINTLOGW_JW(TEXT("[Test] 시나리오 강제 시작: 인덱스 %d"), ScenarioIndex);
	}
}

void AMurphyPlayerController::Test_EndScenarioAndTravel(FName NextLevelKey)
{
	if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
	{
		ScenarioSubsystem->EndScenario(true);
		PRINTLOGW_JW(TEXT("[Test] 시나리오 성공 처리 완료"));
	}
	
	if (ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>())
	{
		PRINTLOGW_JW(TEXT("[Test] 다음 맵으로 서버 트래블 시도: %s"), *NextLevelKey.ToString());
		LevelSubsystem->TravelAllPlayers(NextLevelKey);
	}
}

void AMurphyPlayerController::Test_SimulateAIResponse(const FString& SimulatedJSONResponse)
{
	// 실제 파이썬 서버가 켜져있지 않을 때 NPC의 대화 처리 로직을 강제로 테스트하기 위함
	FAIResponseData ResponseData;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(SimulatedJSONResponse, &ResponseData, 0, 0))
	{
		if (IsValid(TargetNPC))
		{
			TargetNPC->ProcessDialogueResponse(ResponseData);
			
			if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
			{
				MurphyPlayer->EndChatWithNPC();
			}
			PRINTLOGW_JW(TEXT("[Test] 가짜 응답 시뮬레이션 및 NPC 점유 해제 완료!"));
		}
	}
	else
	{
		PRINTLOGE_JW(TEXT("[Test] 시뮬레이션 실패! JSON 문법이 틀렸거나 파싱에 실패했습니다."));
	}
}
