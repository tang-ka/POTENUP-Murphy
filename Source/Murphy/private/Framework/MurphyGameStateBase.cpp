#include "Framework/MurphyGameStateBase.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Framework/MurphyPlayerController.h"
#include "Framework/MurphyPlayerState.h"
#include "Manager/DataManager.h"
#include "Manager/CinematicSequenceSubsystem.h"
#include "Murphy.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Quest/QuestRuntimeHelper.h"

void AMurphyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMurphyGameStateBase, CurrentScenario);
	DOREPLIFETIME(AMurphyGameStateBase, SharedActiveQuests);
	DOREPLIFETIME(AMurphyGameStateBase, SharedCurrentSubQuestIndex);
	DOREPLIFETIME(AMurphyGameStateBase, ScenarioCompletedPlayers);
	DOREPLIFETIME(AMurphyGameStateBase, CurrentResultState);
	DOREPLIFETIME(AMurphyGameStateBase, ChatViewMode);
}

void AMurphyGameStateBase::SetChatViewMode(EChatViewMode NewMode)
{
	if (!HasAuthority())
	{
		return;
	}

	ChatViewMode = NewMode;
	PRINTLOG_SH(TEXT("[GameState] ChatViewMode(%d) 설정"), static_cast<int32>(ChatViewMode));
}

void AMurphyGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// 서버·클라 모두 자기 GameState의 기본값(LevelCinematic)을 읽어 로컬 재생
	TryPlayLevelIntro();
}

void AMurphyGameStateBase::TryPlayLevelIntro()
{
	if (bLevelIntroPlayed || !LevelCinematic)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		// 데디 서버는 화면이 없으므로 재생하지 않음
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	UCinematicSequenceSubsystem* SequenceSubsystem = GameInstance
		? GameInstance->GetSubsystem<UCinematicSequenceSubsystem>()
		: nullptr;
	if (!SequenceSubsystem)
	{
		return;
	}

	bLevelIntroPlayed = true;

	// 인트로는 게임플레이 입력 모드에서 재생돼야 스킵(스페이스바)이 먹고,
	// 종료 후 입력이 이전 UI가 남긴 UIOnly로 굳지 않는다.
	if (APlayerController* LocalPC = World->GetFirstPlayerController())
	{
		LocalPC->SetInputMode(FInputModeGameOnly());
		LocalPC->bShowMouseCursor = false;
	}

	SequenceSubsystem->OnSequenceCompleted.AddUniqueDynamic(this, &AMurphyGameStateBase::HandleLevelIntroFinished);
	SequenceSubsystem->StartLevelSequenceLocal(LevelCinematic);
	PRINTLOG_SH(TEXT("[GameState] 레벨 인트로 시네마틱 로컬 재생 시작"));
}

void AMurphyGameStateBase::RestoreInputModeAfterIntro(APlayerController* LocalPC)
{
	if (!LocalPC)
	{
		return;
	}

	// 기본: 이동 씬 (GameOnly + 커서 off)
	LocalPC->bShowMouseCursor = false;
	LocalPC->SetInputMode(FInputModeGameOnly());
}

void AMurphyGameStateBase::HandleLevelIntroFinished()
{
	// 재생·완료 감지는 각 머신 로컬. 서버 권위 StartScenario만 통보로 트리거한다.
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;

	// 시네마틱이 껐던 커서/입력모드를 레벨별 기본값으로 복원 (시작이 아닌 '종료' 시점).
	RestoreInputModeAfterIntro(PC);

	AMurphyPlayerController* MurphyPC = Cast<AMurphyPlayerController>(PC);
	if (!MurphyPC)
	{
		return;
	}

	MurphyPC->Server_NotifyIntroCinematicFinished();
	PRINTLOG_SH(TEXT("[GameState] 로컬 인트로 완료 — 서버 통보"));
}

void AMurphyGameStateBase::OnRep_ChatViewMode()
{
	// 클라이언트: 복제된 시점 모드를 로컬 컨트롤 폰에 반영
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	if (AMurphyPlayer* LocalPlayer = Cast<AMurphyPlayer>(PC->GetPawn()))
	{
		LocalPlayer->ApplyChatViewMode();
	}
}

void AMurphyGameStateBase::StartScenario(EScenarioType NewScenario)
{
	if (!HasAuthority() || CurrentScenario == NewScenario)
	{
		return;
	}

	CurrentScenario = NewScenario;
	SharedActiveQuests.Empty();
	ScenarioCompletedPlayers.Empty();

	const FScenarioTableRow* ScenarioData = GetCurrentScenarioData();
	if (ScenarioData)
	{
		// DataTable 정책 하나로 개인 진행과 공유 진행을 분기합니다.
		if (ScenarioData->QuestProgressScope == EQuestProgressScope::Shared)
		{
			StartSharedScenario(ScenarioData);
			ClearPersonalScenarioForAllPlayers();
		}
		else
		{
			StartPersonalScenarioForAllPlayers(ScenarioData);
		}
	}

	OnScenarioStateChanged.Broadcast(CurrentScenario);
	OnSharedQuestStateChanged.Broadcast();
}

void AMurphyGameStateBase::EndScenario(bool bSuccess)
{
	if (!HasAuthority() || CurrentScenario == EScenarioType::None)
	{
		return;
	}

	const EScenarioType EndedScenario = CurrentScenario;

	// 서버 상태를 먼저 비우고 delegate를 쏴야 클라이언트 UI가 종료 상태를 일관되게 받습니다.
	CurrentScenario = EScenarioType::None;
	SharedActiveQuests.Empty();
	SharedCurrentSubQuestIndex = INDEX_NONE;
	ScenarioCompletedPlayers.Empty();
	ClearPersonalScenarioForAllPlayers();

	OnScenarioEnded.Broadcast(EndedScenario, bSuccess);
	OnScenarioStateChanged.Broadcast(CurrentScenario);
	OnSharedQuestStateChanged.Broadcast();
}

const FScenarioTableRow* AMurphyGameStateBase::GetCurrentScenarioData() const
{
	if (CurrentScenario == EScenarioType::None)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;
	if (!DataManager)
	{
		return nullptr;
	}

	const FString EnumName = StaticEnum<EScenarioType>()->GetNameStringByValue(static_cast<int64>(CurrentScenario));
	return DataManager->GetScenarioData(FName(*EnumName));
}

void AMurphyGameStateBase::NotifyQuestStartEvent(AMurphyPlayerState* SourcePlayerState, FName TargetID, EQuestCondition EventCondition)
{
	if (!HasAuthority() || !IsValid(SourcePlayerState) || CurrentScenario == EScenarioType::None)
	{
		return;
	}

	const FScenarioTableRow* ScenarioData = GetCurrentScenarioData();
	if (!ScenarioData)
	{
		return;
	}

	if (ScenarioData->QuestProgressScope == EQuestProgressScope::Shared)
	{
		NotifySharedQuestStartEvent(TargetID, EventCondition);
	}
	else
	{
		SourcePlayerState->NotifyPersonalQuestStartEvent(TargetID, EventCondition);
	}
}

void AMurphyGameStateBase::NotifyQuestConditionMet(AMurphyPlayerState* SourcePlayerState, FName TargetID, EQuestCondition Condition)
{
	if (!HasAuthority() || !IsValid(SourcePlayerState) || CurrentScenario == EScenarioType::None)
	{
		return;
	}

	const FScenarioTableRow* ScenarioData = GetCurrentScenarioData();
	if (!ScenarioData)
	{
		return;
	}

	if (ScenarioData->QuestProgressScope == EQuestProgressScope::Shared)
	{
		NotifySharedQuestConditionMet(TargetID, Condition);
	}
	else
	{
		SourcePlayerState->NotifyPersonalQuestConditionMet(TargetID, Condition);
		CheckPersonalScenarioCompletion(SourcePlayerState);
	}

	TryEndScenarioByPolicy();
}

void AMurphyGameStateBase::CheckPersonalScenarioCompletion(AMurphyPlayerState* SourcePlayerState)
{
	if (!HasAuthority() || !IsValid(SourcePlayerState) || CurrentScenario == EScenarioType::None)
	{
		return;
	}

	if (SourcePlayerState->HasCompletedPersonalScenario(CurrentScenario))
	{
		AddScenarioCompletedPlayer(SourcePlayerState);
	}
}

void AMurphyGameStateBase::TryEndScenarioByPolicy()
{
	if (!HasAuthority() || CurrentScenario == EScenarioType::None)
	{
		return;
	}

	const FScenarioTableRow* ScenarioData = GetCurrentScenarioData();
	if (!ScenarioData)
	{
		return;
	}

	switch (ScenarioData->ScenarioEndPolicy)
	{
	case EScenarioEndPolicy::AllPlayersCompleted:
		// 개인 시나리오 기본 정책: 참여 플레이어 전원이 자기 메인 퀘스트를 끝내야 합니다.
		if (AreAllPlayersCompleted())
		{
			EndScenario(true);
		}
		break;
	case EScenarioEndPolicy::SharedQuestCompleted:
		// 공유 시나리오 기본 정책: GameState의 공유 메인 퀘스트 완료만 봅니다.
		if (IsSharedScenarioCompleted())
		{
			EndScenario(true);
		}
		break;
	case EScenarioEndPolicy::AnyPlayerCompleted:
		// 추후 한 명만 통과해도 되는 튜토리얼/이벤트 시나리오용 정책입니다.
		if (!ScenarioCompletedPlayers.IsEmpty() || IsSharedScenarioCompleted())
		{
			EndScenario(true);
		}
		break;
	default:
		break;
	}
}

void AMurphyGameStateBase::OnRep_CurrentScenario()
{
	OnScenarioStateChanged.Broadcast(CurrentScenario);
}

void AMurphyGameStateBase::OnRep_SharedActiveQuests(TArray<FQuestRuntimeData> OldSharedActiveQuests)
{
	// RepNotify에는 서버에서 직접 쏜 delegate가 오지 않으므로 상태 차이를 보고 클라 UI 이벤트를 재구성합니다.
	BroadcastQuestDeltaEvents(OldSharedActiveQuests);
	OnSharedQuestStateChanged.Broadcast();
}

void AMurphyGameStateBase::OnRep_ScenarioCompletedPlayers()
{
	OnSharedQuestStateChanged.Broadcast();
}

void AMurphyGameStateBase::StartPersonalScenarioForAllPlayers(const FScenarioTableRow* ScenarioData)
{
	if (!HasAuthority())
	{
		return;
	}

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (AMurphyPlayerState* MurphyPlayerState = Cast<AMurphyPlayerState>(PlayerState))
		{
			MurphyPlayerState->StartPersonalScenario(CurrentScenario, ScenarioData);
		}
	}
}

void AMurphyGameStateBase::StartSharedScenario(const FScenarioTableRow* ScenarioData)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	// 공유 퀘스트는 플레이어별 복사본을 만들지 않고 GameState의 단일 배열만 사용합니다.
	SharedCurrentSubQuestIndex = INDEX_NONE;
	TArray<FQuestRuntimeEvent> Events;
	SharedCurrentSubQuestIndex = FQuestRuntimeHelper::BuildScenarioRuntimeQuests(DataManager, ScenarioData, SharedActiveQuests, Events);
	BroadcastQuestRuntimeEvents(Events);
}

void AMurphyGameStateBase::ClearPersonalScenarioForAllPlayers()
{
	if (!HasAuthority())
	{
		return;
	}

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (AMurphyPlayerState* MurphyPlayerState = Cast<AMurphyPlayerState>(PlayerState))
		{
			MurphyPlayerState->ClearPersonalScenario();
		}
	}
}

void AMurphyGameStateBase::NotifySharedQuestStartEvent(FName TargetID, EQuestCondition EventCondition)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	TArray<FQuestRuntimeEvent> Events;
	FQuestRuntimeHelper::ProcessQuestStartEvent(DataManager, SharedActiveQuests, SharedCurrentSubQuestIndex, TargetID, EventCondition, Events);

	BroadcastQuestRuntimeEvents(Events);
	OnSharedQuestStateChanged.Broadcast();
}

void AMurphyGameStateBase::NotifySharedQuestConditionMet(FName TargetID, EQuestCondition Condition)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	TArray<FQuestRuntimeEvent> Events;
	bool bScenarioCompleted = false;
	FQuestRuntimeHelper::ProcessQuestConditionMet(DataManager, SharedActiveQuests, SharedCurrentSubQuestIndex, TargetID, Condition, Events, bScenarioCompleted);

	BroadcastQuestRuntimeEvents(Events);
	OnSharedQuestStateChanged.Broadcast();
}

bool AMurphyGameStateBase::AreAllPlayersCompleted() const
{
	int32 PlayerCount = 0;

	// PlayerArray 기준으로 현재 참여 중인 MurphyPlayerState만 완료 여부를 검사합니다.
	for (APlayerState* PlayerState : PlayerArray)
	{
		AMurphyPlayerState* MurphyPlayerState = Cast<AMurphyPlayerState>(PlayerState);
		if (!MurphyPlayerState)
		{
			continue;
		}

		++PlayerCount;
		if (!ScenarioCompletedPlayers.Contains(MurphyPlayerState))
		{
			return false;
		}
	}

	return PlayerCount > 0;
}

bool AMurphyGameStateBase::IsSharedScenarioCompleted() const
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	return FQuestRuntimeHelper::AreMainQuestsCompleted(DataManager, SharedActiveQuests);
}

void AMurphyGameStateBase::AddScenarioCompletedPlayer(AMurphyPlayerState* SourcePlayerState)
{
	if (!IsValid(SourcePlayerState) || ScenarioCompletedPlayers.Contains(SourcePlayerState))
	{
		return;
	}

	ScenarioCompletedPlayers.Add(SourcePlayerState);
	OnSharedQuestStateChanged.Broadcast();
}

void AMurphyGameStateBase::BroadcastQuestRuntimeEvents(const TArray<FQuestRuntimeEvent>& Events)
{
	for (const FQuestRuntimeEvent& Event : Events)
	{
		if (Event.QuestID.IsNone())
		{
			continue;
		}

		if (Event.EventType == EQuestRuntimeEventType::Started)
		{
			BroadcastQuestStarted(Event.QuestID);
		}
		else
		{
			OnSharedQuestCompleted.Broadcast(Event.QuestID);
		}
	}
}

void AMurphyGameStateBase::BroadcastQuestDeltaEvents(const TArray<FQuestRuntimeData>& OldActiveQuests)
{
	for (const FQuestRuntimeData& NewRuntimeData : SharedActiveQuests)
	{
		// 이전 복제 상태와 현재 상태를 비교해 시작/완료 delegate를 클라이언트에서 다시 발생시킵니다.
		const FQuestRuntimeData* OldRuntimeData = OldActiveQuests.FindByPredicate([NewRuntimeData](const FQuestRuntimeData& RuntimeData)
		{
			return RuntimeData.QuestID == NewRuntimeData.QuestID;
		});

		const EScenarioState OldState = OldRuntimeData ? OldRuntimeData->QuestState : EScenarioState::NotStarted;

		if (NewRuntimeData.QuestState == EScenarioState::InProgress && OldState == EScenarioState::NotStarted)
		{
			BroadcastQuestStarted(NewRuntimeData.QuestID);
		}
		else if (NewRuntimeData.QuestState == EScenarioState::Completed && OldState != EScenarioState::Completed)
		{
			OnSharedQuestCompleted.Broadcast(NewRuntimeData.QuestID);
		}
	}
}

void AMurphyGameStateBase::BroadcastQuestStarted(FName QuestID)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;
	if (!DataManager)
	{
		return;
	}

	const FQuestTableRow* QuestData = DataManager->GetQuestData(QuestID);
	if (!QuestData || !QuestData->bShowToastOnStart)
	{
		return;
	}

	if (QuestData->QuestType != EQuestType::SubQuest && QuestData->QuestType != EQuestType::ToastQuest)
	{
		return;
	}

	OnSharedQuestStarted.Broadcast(QuestID, QuestData->QuestTitle, QuestData->QuestDescription);
}

void AMurphyGameStateBase::SetGameResultState(EGameResultState NewState)
{
	if (HasAuthority() && CurrentResultState != NewState)
	{
		CurrentResultState = NewState;
		OnRep_GameResultState(); // 서버 측에서도 이벤트 발생
	}
}

void AMurphyGameStateBase::OnRep_GameResultState()
{
	OnGameResultStateChanged.Broadcast(CurrentResultState);
}
