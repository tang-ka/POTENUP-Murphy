
#include "Framework/MurphyPlayerController.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"

#include "VoiceChat/VoiceRecorderComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "Components/QuestEventNotifyComponent.h"
#include "Components/InputComponent.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "InputCoreTypes.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"

#include "Murphy.h"
#include "Manager/CinematicManagerSubsystem.h"
#include "Manager/DataManager.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Manager/AIBridgeSubsystem.h"
#include "Manager/ScenarioSubsystem.h"
#include "Manager/UIManagerSubsystem.h"
#include "Framework/MurphyGameStateBase.h"
#include "Framework/MurphyPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AMurphyPlayerController::AMurphyPlayerController()
{
	QuestEventNotifier = CreateDefaultSubobject<UQuestEventNotifyComponent>(TEXT("QuestEventNotifier"));
}

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
		BindLocalQuestStateSources();
	}
}

void AMurphyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	// 테스트 전용 단축키입니다. EnhancedInput 에셋 수정 없이 PlayerController에서 직접 N 키만 받습니다.
	FInputKeyBinding& AdvanceSubQuestBinding = InputComponent->BindKey(
		EKeys::N,
		IE_Pressed,
		this,
		&AMurphyPlayerController::HandleAdvanceSubQuestTestKey);
	AdvanceSubQuestBinding.bConsumeInput = false;
}

void AMurphyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocalController())
	{
		BindLocalQuestStateSources();
	}
}

void AMurphyPlayerController::HandleAdvanceSubQuestTestKey()
{
	if (!IsLocalController())
	{
		return;
	}

	// 퀘스트 상태는 서버 권한으로 관리되므로 로컬 입력을 서버 실행으로 넘깁니다.
	if (HasAuthority())
	{
		ServerAdvanceCurrentSubQuestForTest_Implementation();
	}
	else
	{
		ServerAdvanceCurrentSubQuestForTest();
	}
}

void AMurphyPlayerController::ServerAdvanceCurrentSubQuestForTest_Implementation()
{
	if (!AdvanceCurrentSubQuestForTest())
	{
		PRINTLOGW_JW(TEXT("[Test] N 키 서브퀘스트 넘기기 실패: 진행 중인 서브퀘스트를 찾지 못했습니다."));
	}
}

bool AMurphyPlayerController::AdvanceCurrentSubQuestForTest()
{
	AMurphyGameStateBase* MurphyGameState = GetWorld() ? GetWorld()->GetGameState<AMurphyGameStateBase>() : nullptr;
	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();

	// 신규 퀘스트 경로: ScenarioData 정책에 따라 공유(GameState) 또는 개인(PlayerState) 저장소를 선택합니다.
	if (MurphyGameState && MurphyPlayerState && MurphyGameState->IsInScenario())
	{
		const FScenarioTableRow* ScenarioData = MurphyGameState->GetCurrentScenarioData();
		if (!ScenarioData)
		{
			return false;
		}

		const TArray<FQuestRuntimeData>& ActiveQuests = ScenarioData->QuestProgressScope == EQuestProgressScope::Shared
			? MurphyGameState->GetSharedActiveQuests()
			: MurphyPlayerState->GetPersonalActiveQuests();

		FName QuestID;
		FName TargetID;
		EQuestCondition ClearCondition = EQuestCondition::None;
		if (!ResolveCurrentSubQuestForTest(ScenarioData->RequiredQuestIDs, ActiveQuests, QuestID, TargetID, ClearCondition))
		{
			return false;
		}

		if (TargetID.IsNone() || ClearCondition == EQuestCondition::None)
		{
			PRINTLOGW_JW(TEXT("[Test] N 키 서브퀘스트 넘기기 실패: 완료 조건/대상 ID가 비어 있습니다. QuestID: %s"), *QuestID.ToString());
			return false;
		}

		PRINTLOGW_JW(TEXT("[Test] N 키로 현재 서브퀘스트 완료 처리: %s"), *QuestID.ToString());
		// 실제 완료 이벤트와 같은 경로를 타야 다음 퀘스트 시작, 토스트, 시나리오 종료 정책까지 함께 검증됩니다.
		MurphyGameState->NotifyQuestConditionMet(MurphyPlayerState, TargetID, ClearCondition);
		return true;
	}

	return false;
}

bool AMurphyPlayerController::ResolveCurrentSubQuestForTest(const TArray<FName>& QuestOrder, const TArray<FQuestRuntimeData>& ActiveQuests, FName& OutQuestID, FName& OutTargetID, EQuestCondition& OutClearCondition) const
{
	const UDataManager* DataManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDataManager>() : nullptr;
	if (!DataManager)
	{
		return false;
	}

	// RequiredQuestIDs 순서를 기준으로 "현재 단계"를 판단합니다. 동시에 여러 SubQuest가 켜져도 앞쪽 퀘스트를 우선합니다.
	for (FName QuestID : QuestOrder)
	{
		const FQuestRuntimeData* RuntimeData = ActiveQuests.FindByPredicate([QuestID](const FQuestRuntimeData& Candidate)
		{
			return Candidate.QuestID == QuestID;
		});

		if (!RuntimeData || RuntimeData->QuestState != EScenarioState::InProgress)
		{
			continue;
		}

		const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID);
		if (!QuestData || QuestData->QuestType != EQuestType::SubQuest)
		{
			continue;
		}

		OutQuestID = QuestID;
		OutTargetID = QuestData->QuestTargetID;
		OutClearCondition = QuestData->ClearCondition;
		return true;
	}

	return false;
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

bool AMurphyPlayerController::NotifyQuestConditionFromLocal(FName TargetID, EQuestCondition Condition)
{
	if (TargetID.IsNone() || Condition == EQuestCondition::None)
	{
		return false;
	}

	if (QuestEventNotifier)
	{
		// UI는 ActorComponent를 직접 붙일 수 없으므로, PC가 소유한 통보 컴포넌트를 재사용합니다.
		QuestEventNotifier->SetQuestTargetID(TargetID);
		return QuestEventNotifier->NotifyQuestComplete(this, Condition);
	}

	if (!IsLocalController())
	{
		return false;
	}

	// 컴포넌트가 없는 예외 상황에서도 기존 퀘스트 흐름이 완전히 끊기지 않도록 최소 폴백을 둡니다.
	ServerNotifyQuestConditionMet(TargetID, Condition);
	return true;
}

void AMurphyPlayerController::ServerNotifyQuestStartEvent_Implementation(FName TargetID, EQuestCondition StartCondition)
{
	if (TargetID.IsNone() || StartCondition == EQuestCondition::None)
	{
		return;
	}

	AMurphyGameStateBase* MurphyGameState = GetWorld() ? GetWorld()->GetGameState<AMurphyGameStateBase>() : nullptr;
	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();
	if (MurphyGameState && MurphyPlayerState)
	{
		// 최종 저장소 선택은 GameState가 ScenarioData 정책을 보고 결정합니다.
		MurphyGameState->NotifyQuestStartEvent(MurphyPlayerState, TargetID, StartCondition);
		return;
	}

	PRINTLOGW_JW(TEXT("[Quest] MurphyGameStateBase가 없어 퀘스트 시작 이벤트를 처리하지 못했습니다. TargetID: %s"), *TargetID.ToString());
}

void AMurphyPlayerController::ServerNotifyQuestConditionMet_Implementation(FName TargetID, EQuestCondition Condition)
{
	if (TargetID.IsNone() || Condition == EQuestCondition::None)
	{
		return;
	}

	AMurphyGameStateBase* MurphyGameState = GetWorld() ? GetWorld()->GetGameState<AMurphyGameStateBase>() : nullptr;
	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();
	if (MurphyGameState && MurphyPlayerState)
	{
		// 클라이언트 액터/UI에서 발생한 퀘스트 완료 이벤트는 반드시 서버 GameState를 거쳐 처리합니다.
		MurphyGameState->NotifyQuestConditionMet(MurphyPlayerState, TargetID, Condition);
		return;
	}

	PRINTLOGW_JW(TEXT("[Quest] MurphyGameStateBase가 없어 퀘스트 완료 이벤트를 처리하지 못했습니다. TargetID: %s"), *TargetID.ToString());
}

void AMurphyPlayerController::BindLocalQuestStateSources()
{
	if (!IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UUIManagerSubsystem>())
	{
		// PlayerState는 OnRep로 늦게 들어올 수 있으므로 BeginPlay와 OnRep_PlayerState 양쪽에서 재시도합니다.
		UIManager->BindQuestStateSources();
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
		
		FAIRequestData RequestData = GenerateAIRequestData();
		
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
		TargetNPC->UpdateSessionStateFromResponse(ResponseData);
		
		// 필수 디버그 로그 추가 (응답 후)
		PRINTLOGW_JW(TEXT("[Voice Test] --- AI Response After ---"));
		PRINTLOGW_JW(TEXT("response.current_node_id: %s"), *ResponseData.current_node_id);
		PRINTLOGW_JW(TEXT("response.next_action: %s"), *ResponseData.next_action);
		PRINTLOGW_JW(TEXT("response.next_node_id: %s"), *ResponseData.next_node_id);
		PRINTLOGW_JW(TEXT("response.npc.text: %s"), *ResponseData.npc.text);
		PRINTLOGW_JW(TEXT("갱신된 Local CurrentNodeId: %s"), *TargetNPC->GetCurrentNodeId());

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
		FAIRequestData RequestData = GenerateAIRequestData();
		
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

	ULevelStreaming* BaggageLevel = LevelSubsystem->GetStreamingSubLevel(TEXT("SubLevel_BaggageClaim"));
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

void AMurphyPlayerController::ServerStartScenarioForTest_Implementation(EScenarioType NewScenario)
{
	if (AMurphyGameStateBase* MurphyGameState = GetWorld() ? GetWorld()->GetGameState<AMurphyGameStateBase>() : nullptr)
	{
		// 테스트 명령도 서버 GameState를 우선 사용해 실제 멀티 흐름과 같은 경로를 탑니다.
		MurphyGameState->StartScenario(NewScenario);
		return;
	}

	if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UScenarioSubsystem>() : nullptr)
	{
		ScenarioSubsystem->StartScenario(NewScenario);
	}
}

void AMurphyPlayerController::ServerEndScenarioForTest_Implementation(bool bSuccess)
{
	if (AMurphyGameStateBase* MurphyGameState = GetWorld() ? GetWorld()->GetGameState<AMurphyGameStateBase>() : nullptr)
	{
		MurphyGameState->EndScenario(bSuccess);
		return;
	}

	if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UScenarioSubsystem>() : nullptr)
	{
		ScenarioSubsystem->EndScenario(bSuccess);
	}
}

// ==============================================================================
// === 테스트 커맨드 ===
// ==============================================================================

void AMurphyPlayerController::Test_StartScenario(int32 ScenarioIndex)
{
	const EScenarioType ScenarioType = static_cast<EScenarioType>(ScenarioIndex);

	if (HasAuthority())
	{
		ServerStartScenarioForTest_Implementation(ScenarioType);
	}
	else
	{
		ServerStartScenarioForTest(ScenarioType);
	}

	PRINTLOGW_JW(TEXT("[Test] 시나리오 강제 시작 요청: 인덱스 %d"), ScenarioIndex);
}

void AMurphyPlayerController::Test_EndScenarioAndTravel(FName NextLevelKey)
{
	if (HasAuthority())
	{
		ServerEndScenarioForTest_Implementation(true);
	}
	else
	{
		ServerEndScenarioForTest(true);
	}

	PRINTLOGW_JW(TEXT("[Test] 시나리오 성공 처리 요청 완료"));
	
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

FAIRequestData AMurphyPlayerController::GenerateAIRequestData()
{
	FAIRequestData RequestData;
	RequestData.contract_version = TEXT("dev_c_unreal_turn.v1");
	RequestData.request_id = FGuid::NewGuid().ToString();
	
	RequestData.session.player_id = TEXT("player_001");
	
	if (IsValid(TargetNPC))
	{
		RequestData.session.session_id = TargetNPC->GetCurrentSessionId();
		RequestData.session.chapter_id = TargetNPC->GetChapterId();
		RequestData.session.scene_id = TargetNPC->GetSceneId();
		RequestData.session.current_node_id = TargetNPC->GetCurrentNodeId();
		RequestData.session.turn_index = TargetNPC->GetTurnIndex();
		
		RequestData.npc.npc_id = TargetNPC->GetNPCName();
		RequestData.npc.npc_role = TargetNPC->GetNPCRole();
		RequestData.npc.last_npc_message = TargetNPC->GetLastNpcMessage();
		
		RequestData.scenario_state = TargetNPC->GetScenarioState();
	}
	else
	{
		RequestData.session.session_id = TEXT("session_fallback");
		RequestData.session.chapter_id = TEXT("CH0_03_IMMIGRATION_CHECK");
		RequestData.session.scene_id = TEXT("JFK_IMMIGRATION_HALL");
		RequestData.session.current_node_id = TEXT("IMM_002_PURPOSE");
		RequestData.session.turn_index = 1;
		
		RequestData.npc.npc_id = TEXT("OFFICER_MILLER"); // fallback
		RequestData.npc.npc_role = TEXT("immigration_officer");
		RequestData.npc.last_npc_message = TEXT("What is the purpose of your visit?");
	}
	
	RequestData.audio.mime_type = TEXT("audio/wav");
	RequestData.audio.sample_rate_hz = 48000;
	RequestData.audio.channels = 2;
	RequestData.audio.duration_ms = 2800;
	RequestData.audio.language_hint = TEXT("en-US");

	RequestData.interaction.initiator = TEXT("npc");
	RequestData.interaction.interaction_type = TEXT("quest");
	RequestData.interaction.time_limit_s = 30;
	RequestData.interaction.first_contact = false;
	
	RequestData.player_profile.nickname = TEXT("Sean");
	RequestData.player_profile.english_confidence = TEXT("beginner");
	RequestData.player_profile.tier = TEXT("Bronze");
	RequestData.player_profile.travel_speaking_level = TEXT("TSL_1_SURVIVAL");
	
	RequestData.game_state.inventory = { TEXT("passport"), TEXT("boarding_pass"), TEXT("return_ticket") };
	RequestData.game_state.flags = { TEXT("arrived_at_jfk"), TEXT("passport_submitted") };
	RequestData.game_state.completed_intents = { TEXT("submit_passport") };
	RequestData.game_state.current_objective = TEXT("State the visit purpose");
	
	return RequestData;
}

bool AMurphyPlayerController::BuildRealtimeSTTTurnData(FAIRequestData& OutRequestData, FSTT_SessionStart& OutSessionPayload)
{
	if (!IsValid(TargetNPC))
	{
		PRINTLOGW_JW(TEXT("[MurphyController|STT] 대화할 NPC가 없어 STT 요청 데이터를 만들 수 없습니다."));
		return false;
	}

	OutRequestData = GenerateAIRequestData();

	// WebSocket STT는 16kHz mono PCM16 청크를 전송한다.
	OutRequestData.audio.mime_type = TEXT("audio/wav");
	OutRequestData.audio.sample_rate_hz = 16000;
	OutRequestData.audio.channels = 1;
	OutRequestData.audio.duration_ms = 3200;

	// 서버가 현재 노드 컨텍스트 기준으로 자연스럽게 다음 노드를 고르게 한다.
	OutRequestData.client_allowed_next_nodes.Empty();

	OutSessionPayload.request_id = OutRequestData.request_id;
	OutSessionPayload.session_id = OutRequestData.session.session_id;
	OutSessionPayload.turn_index = OutRequestData.session.turn_index;
	OutSessionPayload.chapter_id = OutRequestData.session.chapter_id;
	OutSessionPayload.scene_id = OutRequestData.session.scene_id;
	OutSessionPayload.current_node_id = OutRequestData.session.current_node_id;

	PRINTLOGW_JW(TEXT("[MurphyController|STT] TurnData 준비 완료: req_id=%s, current_node=%s"),
		*OutRequestData.request_id, *OutRequestData.session.current_node_id);

	return true;
}

bool AMurphyPlayerController::SendRealtimeSTTTranscriptToAI(const FAIRequestData& RequestData, const FString& FinalText)
{
	UAIBridgeSubsystem* NetSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>()
		: nullptr;
	if (!NetSubsystem)
	{
		PRINTLOGE_JW(TEXT("[MurphyController|STT] AIBridgeSubsystem을 찾을 수 없습니다."));
		return false;
	}

	const FString TrimmedFinalText = FinalText.TrimStartAndEnd();
	if (TrimmedFinalText.IsEmpty())
	{
		PRINTLOGW_JW(TEXT("[MurphyController|STT] final transcript가 비어 있어 /respond 호출을 건너뜁니다."));
		return false;
	}

	if (IsValid(TargetNPC))
	{
		TargetNPC->NotifyPlayerSpoke();
	}

	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		MurphyPlayer->SetMicUIState(false);
	}

	FOnAIResponseDataReceived Callback;
	Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResponseReceived);
	NetSubsystem->SendToAIWithTranscript(RequestData, TrimmedFinalText, Callback);

	PRINTLOGW_JW(TEXT("[MurphyController|STT] final transcript /respond 전송: \"%s\""), *TrimmedFinalText);
	return true;
}
