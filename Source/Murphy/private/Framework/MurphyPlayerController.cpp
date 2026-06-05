
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

#include "Murphy.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Manager/NetSubsystem.h"
#include "Manager/ScenarioSubsystem.h"

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

void AMurphyPlayerController::SetActiveNPC(AAgentNPCBase* NewNPC)
{
	TargetNPC = NewNPC;
}

void AMurphyPlayerController::OnAudioRecordingFinished(const FString& SavedFilePath)
{
	if (!IsValid(TargetNPC)) return;
	
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (MurphyPlayer->GetRecordTime() < 0.5f)
		{			
			PRINTLOGW_JW(TEXT("[Voice Test] 녹음 시간이 너무 짧습니다. AI 서버로 전송하지 않고 기본 응답을 처리합니다."));
			
			// 최소 v0.1.0 응답 포맷으로 더미 JSON 생성
			FString SimulatedJSONResponse = TEXT("{\"npc\":{\"speaker\":\"System\",\"text\":\"잘 못 들었어. 조금만 더 길게 말해줄래?\",\"tone\":\"neutral\",\"animation\":\"\",\"audio_url\":\"\"}}");
			Test_SimulateAIResponse(SimulatedJSONResponse);
			return;
		}
	}
	
	if (UNetSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetSubsystem>())
	{
		FOnAIResponseDataReceived Callback;
		Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResponseReceived);
		
		FAIRequestData RequestData;
		RequestData.contract_version = TEXT("dev_c_unreal_turn.v1");
		RequestData.request_id = TEXT("req_imm_0001");
		
		RequestData.session.session_id = TEXT("session_001");
		RequestData.session.player_id = TEXT("player_001");
		RequestData.session.chapter_id = TEXT("CH0_IMMIGRATION");
		RequestData.session.scene_id = TEXT("JFK_IMMIGRATION_HALL");
		RequestData.session.current_node_id = TEXT("IMM_002_PURPOSE");
		RequestData.session.turn_index = 2;
		
		RequestData.npc.npc_id = TEXT("OFFICER_MILLER");
		RequestData.npc.npc_role = TEXT("immigration_officer");
		RequestData.npc.last_npc_message = TEXT("What is the purpose of your visit?");
		
		RequestData.audio.mime_type = TEXT("audio/wav");
		RequestData.audio.sample_rate_hz = 48000;
		RequestData.audio.channels = 2;
		RequestData.audio.duration_ms = 2800;
		RequestData.audio.language_hint = TEXT("en-US");
		
		RequestData.player_profile.nickname = TEXT("Sean");
		RequestData.player_profile.english_confidence = TEXT("beginner");
		RequestData.player_profile.tier = TEXT("Bronze");
		RequestData.player_profile.travel_speaking_level = TEXT("TSL_1_SURVIVAL");
		
		RequestData.scenario_state.patience = 100;
		RequestData.scenario_state.suspicion = 0;
		RequestData.scenario_state.retry_count = 0;
		RequestData.scenario_state.hint_count = 0;
		RequestData.scenario_state.previous_fail_count = 0;
		
		RequestData.game_state.inventory = { TEXT("passport"), TEXT("boarding_pass"), TEXT("return_ticket") };
		RequestData.game_state.flags = { TEXT("arrived_at_jfk"), TEXT("passport_submitted") };
		RequestData.game_state.completed_intents = { TEXT("submit_passport") };
		RequestData.game_state.current_objective = TEXT("State the visit purpose");
		
		PRINTLOGW_JW(TEXT("[Voice Test] NetSubsystem을 통해 서버로 오디오 전송 시작"));
		NetSubsystem->SendToAI(RequestData, SavedFilePath, Callback);
	}
}

void AMurphyPlayerController::OnAIResponseReceived(const FAIResponseData& ResponseData)
{
	if (IsValid(TargetNPC))
	{
		TargetNPC->ProcessDialogueResponse(ResponseData);
		
		// AI 응답이 도착해 대화가 끝나면 NPC점유 해제 및 상태 초기화
		if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
		{
			MurphyPlayer->EndChatWithNPC();
		}
	}
}
