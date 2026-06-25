// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MurphyPlayerState.h"

#include "Manager/DataManager.h"
#include "Net/UnrealNetwork.h"
#include "Quest/QuestRuntimeHelper.h"

void AMurphyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMurphyPlayerState, SelectedCharacter);
	DOREPLIFETIME(AMurphyPlayerState, bIsReady);
	DOREPLIFETIME(AMurphyPlayerState, bIsHost);
	
	DOREPLIFETIME(AMurphyPlayerState, SavedSurname);
	DOREPLIFETIME(AMurphyPlayerState, SavedGivenname);
	DOREPLIFETIME(AMurphyPlayerState, CurrentLocationID);
	DOREPLIFETIME(AMurphyPlayerState, CurrentItemID);

	DOREPLIFETIME(AMurphyPlayerState, PersonalScenario);
	DOREPLIFETIME(AMurphyPlayerState, PersonalActiveQuests);
	DOREPLIFETIME(AMurphyPlayerState, PersonalCurrentSubQuestIndex);
	DOREPLIFETIME(AMurphyPlayerState, CompletedPersonalScenarios);
}

void AMurphyPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	
	// SeamlessTravel 시 인게임 PS로 선택값 이관 (bIsReady는 룸 전용이라 제외)
	if (AMurphyPlayerState* NewPS = Cast<AMurphyPlayerState>(PlayerState))
	{
		NewPS->SelectedCharacter = SelectedCharacter;
		NewPS->bIsHost = bIsHost;
		NewPS->PersonalScenario = PersonalScenario;
		NewPS->PersonalActiveQuests = PersonalActiveQuests;
		NewPS->PersonalCurrentSubQuestIndex = PersonalCurrentSubQuestIndex;
		NewPS->CompletedPersonalScenarios = CompletedPersonalScenarios;
		NewPS->SavedSurname = SavedSurname;
		NewPS->SavedGivenname = SavedGivenname;
		NewPS->CurrentLocationID = CurrentLocationID;
		NewPS->CurrentItemID = CurrentItemID;
	}
}

void AMurphyPlayerState::OnRep_SessionRoomState()
{
	OnSessionRoomStateChanged.Broadcast();
}

void AMurphyPlayerState::OnRep_PersonalScenario()
{
	OnPersonalQuestStateChanged.Broadcast();
}

void AMurphyPlayerState::OnRep_PersonalActiveQuests(TArray<FQuestRuntimeData> OldPersonalActiveQuests)
{
	// 서버 delegate는 클라이언트로 복제되지 않으므로 상태 변화량으로 토스트/완료 이벤트를 재생성합니다.
	BroadcastQuestDeltaEvents(OldPersonalActiveQuests);
	OnPersonalQuestStateChanged.Broadcast();
}

void AMurphyPlayerState::OnRep_CompletedPersonalScenarios()
{
	OnPersonalQuestStateChanged.Broadcast();
}

bool AMurphyPlayerState::HasCompletedPersonalScenario(EScenarioType ScenarioType) const
{
	return ScenarioType != EScenarioType::None && CompletedPersonalScenarios.Contains(ScenarioType);
}

bool AMurphyPlayerState::IsCurrentPersonalScenarioCompleted() const
{
	return HasCompletedPersonalScenario(PersonalScenario);
}

void AMurphyPlayerState::StartPersonalScenario(EScenarioType NewScenario, const FScenarioTableRow* ScenarioData)
{
	if (!HasAuthority())
	{
		return;
	}

	PersonalScenario = NewScenario;
	PersonalActiveQuests.Empty();
	PersonalCurrentSubQuestIndex = INDEX_NONE;

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	// 개인 시나리오는 각 PlayerState가 자기만의 런타임 퀘스트 배열을 갖습니다.
	TArray<FQuestRuntimeEvent> Events;
	PersonalCurrentSubQuestIndex = FQuestRuntimeHelper::BuildScenarioRuntimeQuests(DataManager, ScenarioData, PersonalActiveQuests, Events);
	BroadcastQuestRuntimeEvents(Events);
	OnPersonalQuestStateChanged.Broadcast();
}

void AMurphyPlayerState::ClearPersonalScenario()
{
	if (!HasAuthority())
	{
		return;
	}

	PersonalScenario = EScenarioType::None;
	PersonalActiveQuests.Empty();
	PersonalCurrentSubQuestIndex = INDEX_NONE;
	OnPersonalQuestStateChanged.Broadcast();
}

void AMurphyPlayerState::NotifyPersonalQuestStartEvent(FName TargetID, EQuestCondition EventCondition)
{
	if (!HasAuthority() || PersonalScenario == EScenarioType::None)
	{
		return;
	}

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	TArray<FQuestRuntimeEvent> Events;
	FQuestRuntimeHelper::ProcessQuestStartEvent(DataManager, PersonalActiveQuests, PersonalCurrentSubQuestIndex, TargetID, EventCondition, Events);

	BroadcastQuestRuntimeEvents(Events);
	OnPersonalQuestStateChanged.Broadcast();
}

void AMurphyPlayerState::NotifyPersonalQuestConditionMet(FName TargetID, EQuestCondition Condition)
{
	if (!HasAuthority() || PersonalScenario == EScenarioType::None)
	{
		return;
	}

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;

	TArray<FQuestRuntimeEvent> Events;
	bool bScenarioCompleted = false;
	FQuestRuntimeHelper::ProcessQuestConditionMet(DataManager, PersonalActiveQuests, PersonalCurrentSubQuestIndex, TargetID, Condition, Events, bScenarioCompleted);

	if (bScenarioCompleted)
	{
		MarkCurrentPersonalScenarioCompleted();
	}

	BroadcastQuestRuntimeEvents(Events);
	OnPersonalQuestStateChanged.Broadcast();
}

void AMurphyPlayerState::BroadcastQuestRuntimeEvents(const TArray<FQuestRuntimeEvent>& Events)
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
			OnPersonalQuestCompleted.Broadcast(Event.QuestID);
		}
	}
}

void AMurphyPlayerState::BroadcastQuestDeltaEvents(const TArray<FQuestRuntimeData>& OldActiveQuests)
{
	for (const FQuestRuntimeData& NewRuntimeData : PersonalActiveQuests)
	{
		// 복제 전/후 상태를 비교해 이 클라이언트에 필요한 개인 퀘스트 알림만 발생시킵니다.
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
			OnPersonalQuestCompleted.Broadcast(NewRuntimeData.QuestID);
		}
	}
}

void AMurphyPlayerState::BroadcastQuestStarted(FName QuestID)
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

	// 메인 퀘스트는 HUD 상시 목표로 다루고, 서브/토스트 퀘스트만 팝업 알림으로 보냅니다.
	OnPersonalQuestStarted.Broadcast(QuestID, QuestData->QuestTitle, QuestData->QuestDescription);
}

void AMurphyPlayerState::MarkCurrentPersonalScenarioCompleted()
{
	if (PersonalScenario == EScenarioType::None || CompletedPersonalScenarios.Contains(PersonalScenario))
	{
		return;
	}

	CompletedPersonalScenarios.Add(PersonalScenario);
}

void AMurphyPlayerState::ServerSetArrivalData_Implementation(const FString& InSurname, const FString& InGivenname)
{
	// 서버에서 실행되는 실제 데이터 저장 로직
	SavedSurname = InSurname;
	SavedGivenname = InGivenname;
}

void AMurphyPlayerState::OnRep_ArrivalData()
{
	if (OnArrivalDataUpdated.IsBound())
	{
		OnArrivalDataUpdated.Broadcast();
	}
}
