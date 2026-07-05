
#include "Framework/MurphyPlayerController.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"
#include "UI/HUD/MainHUD.h"

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
#include "Framework/MurphyPlayerController.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"
#include "UI/HUD/MainHUD.h"

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
#include "Components/BoxComponent.h"
#include "Manager/CinematicManagerSubsystem.h"
#include "Manager/DataManager.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Manager/AIBridgeSubsystem.h"
#include "Manager/ScenarioSubsystem.h"
#include "Manager/UIManagerSubsystem.h"
#include "Data/CinematicSequenceData.h"
#include "Framework/Airplane/AirplaneGameMode.h"
#include "Framework/MurphyGameStateBase.h"
#include "Framework/MurphyPlayerState.h"
#include "Framework/Prologue/PrologueGameMode.h"
#include "Framework/Prologue/PrologueGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/BagPopupWidget.h"
#include "UI/HUD/MainHUD.h"
#include "UI/PlayReport/PlayReportMainWidget.h"

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
		RefreshLocalBagFromOwnedItems();

		const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
		NotifyAIResultTriggerLevelEntered(FName(*CurrentLevelName));

		if (CurrentLevelName.Contains(TEXT("Lv_Airplane")))
		{
			ShowLevelEnterToastAfterCinematic(FText::FromString(TEXT("비행기(기내)")));
		}
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
		RefreshLocalBagFromOwnedItems();
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
	const bool bHasActiveNPC = IsValid(TargetNPC);
	
	// 오버랩에 따른 마이크 UI 상태(활성화/비활성화) 업데이트
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		// MurphyPlayer->SetMicUIState(bHasActiveNPC);

		// 대화 진입~이탈 동안 Translate 연결 표시등 ON/OFF
		if (UMainHUD* MainHUD = MurphyPlayer->GetMainHUD())
		{
			MainHUD->SetTranslateConnecting(bHasActiveNPC);
			MainHUD->SetCaptionInteractionActive(bHasActiveNPC);
		}
	}
}

void AMurphyPlayerController::FlipNearestAirplaneNPCBoxLocal()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		PRINTLOG_SH(TEXT("[Airplane] 로컬 박스 반전 실패: Pawn이 없습니다."));
		return;
	}

	const FVector PlayerLocation = MyPawn->GetActorLocation();

	TArray<AActor*> FoundNPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAgentNPCBase::StaticClass(), FoundNPCs);

	AAgentNPCBase* NearestNPC = nullptr;
	float NearestDistSq = TNumericLimits<float>::Max();

	for (AActor* Actor : FoundNPCs)
	{
		AAgentNPCBase* NPC = Cast<AAgentNPCBase>(Actor);
		if (!NPC)
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(PlayerLocation, NPC->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestNPC = NPC;
		}
	}

	if (!NearestNPC)
	{
		PRINTLOG_SH(TEXT("[Airplane] 로컬 박스 반전 실패: NPC를 찾지 못했습니다."));
		return;
	}

	UBoxComponent* InteractionBox = NearestNPC->FindComponentByClass<UBoxComponent>();
	if (!InteractionBox)
	{
		PRINTLOG_SH(TEXT("[Airplane] 로컬 박스 반전 실패: %s의 InteractionBox가 없습니다."), *NearestNPC->GetName());
		return;
	}

	FVector LocalLoc = InteractionBox->GetRelativeLocation();
	LocalLoc.X *= -1.0f;
	InteractionBox->SetRelativeLocation(LocalLoc);

	PRINTLOG_SH(TEXT("[Airplane] (Local) 가장 가까운 NPC %s InteractionBox X 반전 (%f)"), *NearestNPC->GetName(), LocalLoc.X);
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

void AMurphyPlayerController::ServerNotifyPrologueAINodeReached_Implementation(FName NodeId)
{
	if (NodeId.IsNone())
	{
		return;
	}

	APrologueGameMode* PrologueGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APrologueGameMode>() : nullptr;
	if (PrologueGameMode)
	{
		PrologueGameMode->HandleAINodeReached(NodeId);
	}
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

void AMurphyPlayerController::EnsurePrologueRequiredItemsInBag()
{
	if (!IsLocalController())
	{
		return;
	}

	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();
	if (!IsValid(MurphyPlayerState))
	{
		return;
	}

	static const FName ArrivalCardItemID(TEXT("Item_ArrivalCard"));
	static const FName PassportItemID(TEXT("Item_Passport"));
	const TArray<FName> RequiredItemIDs = { ArrivalCardItemID, PassportItemID };

	// Prologue 기본 소지품은 PlayerState 보유 목록에 먼저 보장하고, Bag UI는 그 목록을 표시합니다.
	if (MurphyPlayerState->HasAuthority())
	{
		MurphyPlayerState->EnsureOwnedItems(RequiredItemIDs);
	}
	else
	{
		MurphyPlayerState->ServerEnsureOwnedItems(RequiredItemIDs);
	}

	RefreshLocalBagFromOwnedItems();
}

void AMurphyPlayerController::RefreshLocalBagFromOwnedItems()
{
	if (!IsLocalController())
	{
		return;
	}

	AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn());
	if (!IsValid(MurphyPlayer))
	{
		return;
	}

	UMainHUD* MainHUD = MurphyPlayer->GetMainHUD();
	if (!IsValid(MainHUD))
	{
		return;
	}

	UBagPopupWidget* BagWidget = MainHUD->GetBagPopupWidget();
	if (IsValid(BagWidget))
	{
		BagWidget->RefreshFromOwnedItems();
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
		
		// if (MurphyPlayer)
		// {
		 	// 마이크 UI 비활성화
		// 	MurphyPlayer->SetMicUIState(false);
		// }
	}
}

void AMurphyPlayerController::OnAIResponseReceived(const FAIResponseData& ResponseData)
{
	// 충돌체 비활성화 시 TargetNPC가 즉시 nullptr로 초기화되는 것을 막기 위해 로컬 변수에 저장
	AAgentNPCBase* CurrentNPC = TargetNPC;
	if (IsValid(CurrentNPC))
	{
		const FString ResultSessionId = CurrentNPC->GetCurrentSessionId();

		CurrentNPC->ProcessDialogueResponse(ResponseData);
		CurrentNPC->UpdateSessionStateFromResponse(ResponseData);
		
		// 필수 디버그 로그 추가 (응답 후)
		PRINTLOGW_JW(TEXT("[Voice Test] --- AI Response After ---"));
		PRINTLOGW_JW(TEXT("response.current_node_id: %s"), *ResponseData.current_node_id);
		PRINTLOGW_JW(TEXT("response.next_action: %s"), *ResponseData.next_action);
		PRINTLOGW_JW(TEXT("response.next_node_id: %s"), *ResponseData.next_node_id);
		PRINTLOGW_JW(TEXT("response.npc.text: %s"), *ResponseData.npc.text);
		PRINTLOGW_JW(TEXT("갱신된 Local CurrentNodeId: %s"), *CurrentNPC->GetCurrentNodeId());

		if (ResponseData.next_action == TEXT("ADVANCE") && !ResponseData.next_node_id.IsEmpty())
		{
			ServerNotifyPrologueAINodeReached(FName(*ResponseData.next_node_id));
		}

		// AI 응답이 도착해 대화가 끝나면 NPC점유 해제 및 상태 초기화
		if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
		{
			// Agent 대사 블록 추가 (TTS 텍스트)
			if (UMainHUD* MainHUD = MurphyPlayer->GetMainHUD())
			{
				MainHUD->AddAgentDialog(ResponseData.npc.speaker, ResponseData.npc.text);
			}

			MurphyPlayer->EndChatWithNPC();

			// 마이크 UI 활성화
			// MurphyPlayer->SetMicUIState(true);
		}

		if (IsAIResultTriggerResponse(ResponseData))
		{
			RequestAIResultForSession(ResultSessionId, true);
		}
	}
}

void AMurphyPlayerController::OnAIResultReceived_Silent(const FAIResultResponse& ResultData)
{
	if (ResultData.session_id.IsEmpty() && ResultData.contract_version.IsEmpty())
	{
		PRINTLOGE_JW(TEXT("[AIResult] 비어 있는 최종 결과 응답을 받아 저장하지 않습니다."));
		return;
	}

	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPlayerState)
	{
		PRINTLOGE_JW(TEXT("[AIResult] MurphyPlayerState가 없어 최종 결과를 저장하지 못했습니다."));
		return;
	}

	MurphyPlayerState->SaveAIResult(ResultData);
	PRINTLOGW_JW(TEXT("[AIResult] 최종 결과(Silent) 저장 완료: session_id=%s, tier=%s, score=%d"),
		*ResultData.session_id,
		*ResultData.final_result.tier,
		ResultData.final_result.final_score_100);
}

void AMurphyPlayerController::OnAIResultReceived_ShowUI(const FAIResultResponse& ResultData)
{
	// 1. 데이터 저장 (Silent와 동일)
	OnAIResultReceived_Silent(ResultData);
	
	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPlayerState) return;

	// 2. UI 표시 및 데이터 바인딩
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (UMainHUD* MainHUD = MurphyPlayer->GetMainHUD())
		{
			MainHUD->SetReportVisible(true);
			
			if (UPlayReportMainWidget* ReportWidget = MainHUD->GetPlayReportMain())
			{
				ReportWidget->DisplayReport(MurphyPlayerState->GetLastPlayReportData());
			}
		}
	}

	// 3. 성적표 조작을 위한 인풋 모드 설정
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}

void AMurphyPlayerController::OnFinalScoreboardSignalResponse(const FAIResponseData& ResponseData)
{
	const FString ResultSessionId = ResponseData.session_id.IsEmpty()
		? (IsValid(TargetNPC) ? TargetNPC->GetCurrentSessionId() : GetOrCreateAIPlaySessionId())
		: ResponseData.session_id;

	PRINTLOGW_JW(TEXT("[AIResult] 최종 점수판 신호 응답 수신: session_id=%s, next_node=%s, action=%s"),
		*ResultSessionId,
		*ResponseData.next_node_id,
		*ResponseData.next_action);

	RequestAIResultForSession(ResultSessionId, true);
}

FString AMurphyPlayerController::GetOrCreateAIPlaySessionId()
{
	AMurphyPlayerState* MurphyPlayerState = GetPlayerState<AMurphyPlayerState>();
	if (!MurphyPlayerState)
	{
		PRINTLOGE_JW(TEXT("[AIResult] MurphyPlayerState가 없어 AIPlaySessionId를 생성하지 못했습니다."));
		return TEXT("");
	}

	return MurphyPlayerState->GetOrCreateAIPlaySessionId();
}

FAIRequestData AMurphyPlayerController::GenerateFinalScoreboardSignalRequestData()
{
	FAIRequestData RequestData = GenerateAIRequestData();

	RequestData.request_id = FGuid::NewGuid().ToString();
	RequestData.session.session_id = IsValid(TargetNPC) ? TargetNPC->GetCurrentSessionId() : GetOrCreateAIPlaySessionId();
	RequestData.session.current_node_id = FinalScoreboardNodeId;
	RequestData.session.turn_index = FMath::Max(RequestData.session.turn_index, 1);

	RequestData.npc.npc_id = TEXT("SYSTEM");
	RequestData.npc.npc_role = TEXT("final_scoreboard");
	RequestData.npc.last_npc_message = TEXT("");

	RequestData.audio.duration_ms = 0;
	RequestData.interaction.initiator = TEXT("system");
	RequestData.interaction.interaction_type = TEXT("system");
	RequestData.interaction.time_limit_s = 1;
	RequestData.interaction.first_contact = false;
	RequestData.interaction.system_event = TEXT("enter_final_scoreboard");
	RequestData.client_allowed_next_nodes.Empty();

	return RequestData;
}

bool AMurphyPlayerController::IsAIResultTriggerResponse(const FAIResponseData& ResponseData) const
{
	return ResponseData.next_node_id == FinalScoreboardNodeId
		|| ResponseData.current_node_id == FinalScoreboardNodeId
		|| ResponseData.next_action == FinalScoreboardNodeId;
}

bool AMurphyPlayerController::ShouldTriggerFinalScoreboardForLevel(FName EnteredLevelName) const
{
	return !FinalScoreboardTriggerLevelName.IsNone()
		&& !EnteredLevelName.IsNone()
		&& EnteredLevelName == FinalScoreboardTriggerLevelName;
}

void AMurphyPlayerController::RequestAIResultForSession(const FString& SessionId, bool bShowUI)
{
	const FString TrimmedSessionId = SessionId.TrimStartAndEnd();
	if (TrimmedSessionId.IsEmpty())
	{
		PRINTLOGE_JW(TEXT("[AIResult] session_id가 비어 있어 최종 결과 조회를 요청하지 않습니다."));
		return;
	}

	UAIBridgeSubsystem* NetSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>()
		: nullptr;
	if (!NetSubsystem)
	{
		PRINTLOGE_JW(TEXT("[AIResult] AIBridgeSubsystem을 찾을 수 없어 최종 결과 조회를 요청하지 못했습니다."));
		return;
	}

	FOnAIResultReceived Callback;
	if (bShowUI)
	{
		Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResultReceived_ShowUI);
	}
	else
	{
		Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResultReceived_Silent);
	}
	NetSubsystem->RequestAIResult(TrimmedSessionId, Callback);
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
	EnsurePrologueRequiredItemsInBag();

	ShowLevelEnterToastAfterCinematic(FText::FromString(TEXT("입국심사")));
}

void AMurphyPlayerController::TransitionToBaggageClaim()
{
	if (!IsLocalController())
	{
		return;
	}

	// 각 플레이어가 각자 넘어가므로 서버 전역 시퀀스가 아닌 '로컬' 시네마틱으로 검정을 깐다.
	// 시네마틱 미지정/매니저 없음이면 예전처럼 즉시 스왑으로 폴백.
	if (!BaggageClaimCinematic || BaggageClaimCinematic->Entries.Num() == 0)
	{
		PRINTLOG_SH(TEXT("TransitionToBaggageClaim: BaggageClaimCinematic 미지정 - 즉시 스왑"));
		StartBaggageClaimSwap();
		return;
	}

	UCinematicManagerSubsystem* CinematicManager = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>()
		: nullptr;
	if (!CinematicManager)
	{
		PRINTLOG_SH(TEXT("TransitionToBaggageClaim: CinematicManagerSubsystem is null - 즉시 스왑"));
		StartBaggageClaimSwap();
		return;
	}

	// 첫 엔트리로 로컬 재생 요청 구성 (단일 클립).
	const FCinematicEntry& Entry = BaggageClaimCinematic->Entries[0];
	FCinematicPlayRequest Request;
	Request.CinematicId = Entry.CinematicId;
	Request.MediaSource = Entry.Media;
	Request.Duration = Entry.Duration;
	Request.bSkippable = Entry.bSkippable;
	Request.VolumeScale = Entry.VolumeScale;
	Request.Fade = Entry.Fade;

	// 검정(HoldingBlack) 도달 시 스왑을 태우기 위해 로컬 PlayId 발급 + Hold 신호 1회 구독.
	BaggageClaimCinematicPlayId = NextLocalCinematicPlayId++;
	CinematicManager->OnReachedHold.AddUniqueDynamic(this, &AMurphyPlayerController::HandleBaggageClaimCinematicReachedHold);

	// bAutoReleaseHold=false: 스왑/로드가 끝난 뒤 OnBaggageClaimLevelShown에서 직접 ReleaseHold로 리빌한다.
	CinematicManager->PlayMedia(Request, BaggageClaimCinematicPlayId, /*bInAutoReleaseHold*/ false);
	PRINTLOG_SH(TEXT("TransitionToBaggageClaim: 로컬 시네마틱 시작 (PlayId=%d)"), BaggageClaimCinematicPlayId);
}

void AMurphyPlayerController::StartBaggageClaimSwap()
{
	ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>()
		: nullptr;
	if (!LevelSubsystem)
	{
		PRINTLOG_SH(TEXT("StartBaggageClaimSwap: LevelStreamingSubsystem is null"));
		return;
	}

	// BaggageClaim 레벨이 표시되면 리포지션 & 리빌 & 토스트 처리
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

void AMurphyPlayerController::HandleBaggageClaimCinematicReachedHold(int32 PlayId)
{
	if (PlayId != BaggageClaimCinematicPlayId)
	{
		return;
	}

	// 이 신호는 1회만 필요하므로 즉시 구독 해제.
	if (UCinematicManagerSubsystem* CinematicManager = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>()
		: nullptr)
	{
		CinematicManager->OnReachedHold.RemoveDynamic(this, &AMurphyPlayerController::HandleBaggageClaimCinematicReachedHold);
	}

	PRINTLOG_SH(TEXT("HandleBaggageClaimCinematicReachedHold: 검정 도달 - 서브레벨 스왑 시작 (PlayId=%d)"), PlayId);
	StartBaggageClaimSwap();
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

	NotifyAIResultTriggerLevelEntered(FName(TEXT("SubLevel_BaggageClaim")));

	EnsurePrologueRequiredItemsInBag();

	if (APrologueGameState* PrologueGameState = GetWorld() ? GetWorld()->GetGameState<APrologueGameState>() : nullptr)
	{
		PrologueGameState->ApplyBaggageCustomsHoldActorState();
	}

	ShowLevelEnterToastAfterCinematic(FText::FromString(TEXT("수하물 수취장")));

	Server_RequestReposition(TEXT("SubLevel_BaggageClaim"));

	// 스왑/로드 완료 - 검정 아래에서 리포지션까지 마쳤으니 검정을 풀어 새 레벨을 드러낸다. (로컬 리빌)
	if (BaggageClaimCinematicPlayId != INDEX_NONE)
	{
		if (UCinematicManagerSubsystem* CinematicManager = GetGameInstance()
			? GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>()
			: nullptr)
		{
			CinematicManager->ReleaseHold(BaggageClaimCinematicPlayId);
		}
		BaggageClaimCinematicPlayId = INDEX_NONE;
	}
}

void AMurphyPlayerController::ShowLevelEnterToastAfterCinematic(const FText& LevelName)
{
	UCinematicManagerSubsystem* CinematicManager = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>()
		: nullptr;

	if (CinematicManager && CinematicManager->IsPlaying())
	{
		PendingLevelEnterToastName = LevelName;
		CinematicManager->OnCompleted.AddUniqueDynamic(this, &AMurphyPlayerController::HandleCinematicCompletedForLevelEnterToast);
		return;
	}

	ULocalPlayer* LP = GetLocalPlayer();
	UUIManagerSubsystem* UIManager = LP ? LP->GetSubsystem<UUIManagerSubsystem>() : nullptr;
	if (UIManager)
	{
		UIManager->ShowLevelEnterToast(LevelName);
	}
}

void AMurphyPlayerController::HandleCinematicCompletedForLevelEnterToast(int32 PlayId)
{
	if (!PendingLevelEnterToastName.IsSet())
	{
		return;
	}

	const FText LevelName = PendingLevelEnterToastName.GetValue();
	PendingLevelEnterToastName.Reset();

	ShowLevelEnterToastAfterCinematic(LevelName);
}

void AMurphyPlayerController::RequestAirplaneScenarioCompleteTravel()
{
	if (HasAuthority())
	{
		Server_RequestAirplaneScenarioCompleteTravel_Implementation();
		return;
	}

	Server_RequestAirplaneScenarioCompleteTravel();
}

void AMurphyPlayerController::Server_RequestAirplaneScenarioCompleteTravel_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		PRINTLOG_SH(TEXT("Server_RequestAirplaneScenarioCompleteTravel: World is null"));
		return;
	}

	AAirplaneGameMode* AirplaneGameMode = Cast<AAirplaneGameMode>(World->GetAuthGameMode());
	if (!AirplaneGameMode)
	{
		PRINTLOG_SH(TEXT("Server_RequestAirplaneScenarioCompleteTravel: AirplaneGameMode가 아닙니다."));
		return;
	}

	AirplaneGameMode->CompleteScenarioAndTravel();
}

void AMurphyPlayerController::Client_PlayCinematic_Implementation(const FCinematicPlayRequest& Request, int32 PlayId)
{
	UCinematicManagerSubsystem* CinematicManager = GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>();
	if (!CinematicManager)
	{
		PRINTLOG_SH(TEXT("Client_PlayCinematic: CinematicManagerSubsystem is null"));
		return;
	}

	// 시퀀서가 검정 Hold에서 다음 엔트리/트래블을 제어하므로 자동 해제는 끈다.
	CinematicManager->PlayMedia(Request, PlayId, /*bInAutoReleaseHold*/ false);
}

void AMurphyPlayerController::Client_PlayNextCinematic_Implementation(const FCinematicPlayRequest& Request, int32 PlayId)
{
	UCinematicManagerSubsystem* CinematicManager = GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>();
	if (!CinematicManager)
	{
		PRINTLOG_SH(TEXT("Client_PlayNextCinematic: CinematicManagerSubsystem is null"));
		return;
	}

	CinematicManager->PlayNextInHold(Request, PlayId);
}

void AMurphyPlayerController::Client_ReleaseCinematic_Implementation(int32 PlayId)
{
	UCinematicManagerSubsystem* CinematicManager = GetGameInstance()->GetSubsystem<UCinematicManagerSubsystem>();
	if (!CinematicManager)
	{
		PRINTLOG_SH(TEXT("Client_ReleaseCinematic: CinematicManagerSubsystem is null"));
		return;
	}

	CinematicManager->ReleaseHold(PlayId);
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
		const FString PlaySessionId = GetOrCreateAIPlaySessionId();
		RequestData.session.session_id = PlaySessionId.IsEmpty() ? TEXT("session_fallback") : PlaySessionId;
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
		// User 대사 블록 추가 (확정된 STT 텍스트)
		if (UMainHUD* MainHUD = MurphyPlayer->GetMainHUD())
		{
			MainHUD->AddUserDialog(TrimmedFinalText);
		}

		MurphyPlayer->SetMicUIState(false);
	}

	FOnAIResponseDataReceived Callback;
	Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResponseReceived);
	NetSubsystem->SendToAIWithTranscript(RequestData, TrimmedFinalText, Callback);

	PRINTLOGW_JW(TEXT("[MurphyController|STT] final transcript /respond 전송: \"%s\""), *TrimmedFinalText);
	return true;
}

void AMurphyPlayerController::NotifyAIResultTriggerLevelEntered(FName EnteredLevelName)
{
	if (!IsLocalController() || !ShouldTriggerFinalScoreboardForLevel(EnteredLevelName))
	{
		return;
	}

	TriggerFinalScoreboardSignal();
}

void AMurphyPlayerController::TriggerFinalScoreboardSignal()
{
	if (!IsLocalController())
	{
		return;
	}

	if (bFinalScoreboardSignalSent)
	{
		PRINTLOGW_JW(TEXT("[AIResult] 최종 점수판 신호가 이미 전송되어 중복 호출을 건너뜁니다."));
		return;
	}

	UAIBridgeSubsystem* NetSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UAIBridgeSubsystem>()
		: nullptr;
	if (!NetSubsystem)
	{
		PRINTLOGE_JW(TEXT("[AIResult] AIBridgeSubsystem을 찾을 수 없어 최종 점수판 신호를 보내지 못했습니다."));
		return;
	}

	const FString SessionId = IsValid(TargetNPC) ? TargetNPC->GetCurrentSessionId() : GetOrCreateAIPlaySessionId();
	if (SessionId.IsEmpty())
	{
		PRINTLOGE_JW(TEXT("[AIResult] session_id가 비어 있어 최종 점수판 신호를 보내지 못했습니다."));
		return;
	}

	bFinalScoreboardSignalSent = true;

	FOnAIResponseDataReceived Callback;
	Callback.BindDynamic(this, &AMurphyPlayerController::OnFinalScoreboardSignalResponse);
	NetSubsystem->SendToAIWithTranscript(GenerateFinalScoreboardSignalRequestData(), FinalScoreboardNodeId, Callback);

	PRINTLOGW_JW(TEXT("[AIResult] 최종 점수판 신호 전송: session_id=%s, node=%s"), *SessionId, *FinalScoreboardNodeId);
}
