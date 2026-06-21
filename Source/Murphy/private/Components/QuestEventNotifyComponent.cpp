#include "Components/QuestEventNotifyComponent.h"

#include "Murphy.h"
#include "Framework/MurphyPlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UQuestEventNotifyComponent::UQuestEventNotifyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestEventNotifyComponent::SetQuestTargetID(FName InQuestTargetID)
{
	QuestTargetID = InQuestTargetID;
}

bool UQuestEventNotifyComponent::NotifyQuestStart(AActor* InstigatorActor, EQuestStartCondition ConditionOverride) const
{
	if (!HasValidQuestTarget())
	{
		return false;
	}

	const EQuestStartCondition StartCondition = ConditionOverride == EQuestStartCondition::None
		? DefaultStartCondition
		: ConditionOverride;
	if (StartCondition == EQuestStartCondition::None)
	{
		PRINTLOGW_JW(TEXT("[QuestEventNotifyComponent] 시작 조건이 설정되지 않았습니다. Owner: %s, TargetID: %s"),
			*GetNameSafe(GetOwner()), *QuestTargetID.ToString());
		return false;
	}

	AMurphyPlayerController* MurphyPC = ResolveMurphyPlayerController(InstigatorActor);
	if (!IsValid(MurphyPC) || !MurphyPC->IsLocalController())
	{
		return false;
	}

	MurphyPC->ServerNotifyQuestStartEvent(QuestTargetID, StartCondition);
	return true;
}

bool UQuestEventNotifyComponent::NotifyQuestComplete(AActor* InstigatorActor, EQuestClearCondition ConditionOverride) const
{
	if (!HasValidQuestTarget())
	{
		return false;
	}

	const EQuestClearCondition ClearCondition = ConditionOverride == EQuestClearCondition::None
		? DefaultClearCondition
		: ConditionOverride;
	if (ClearCondition == EQuestClearCondition::None)
	{
		PRINTLOGW_JW(TEXT("[QuestEventNotifyComponent] 완료 조건이 설정되지 않았습니다. Owner: %s, TargetID: %s"),
			*GetNameSafe(GetOwner()), *QuestTargetID.ToString());
		return false;
	}

	AMurphyPlayerController* MurphyPC = ResolveMurphyPlayerController(InstigatorActor);
	if (!IsValid(MurphyPC) || !MurphyPC->IsLocalController())
	{
		return false;
	}

	MurphyPC->ServerNotifyQuestConditionMet(QuestTargetID, ClearCondition);
	return true;
}

AMurphyPlayerController* UQuestEventNotifyComponent::ResolveMurphyPlayerController(AActor* InstigatorActor) const
{
	if (!IsValid(InstigatorActor))
	{
		return nullptr;
	}

	if (AMurphyPlayerController* MurphyPC = Cast<AMurphyPlayerController>(InstigatorActor))
	{
		return MurphyPC;
	}

	if (APawn* Pawn = Cast<APawn>(InstigatorActor))
	{
		return Cast<AMurphyPlayerController>(Pawn->GetController());
	}

	if (AController* Controller = Cast<AController>(InstigatorActor))
	{
		return Cast<AMurphyPlayerController>(Controller);
	}

	return nullptr;
}

bool UQuestEventNotifyComponent::HasValidQuestTarget() const
{
	if (!QuestTargetID.IsNone())
	{
		return true;
	}

	PRINTLOGE_JW(TEXT("[QuestEventNotifyComponent] QuestTargetID가 설정되지 않았습니다. Owner: %s"), *GetNameSafe(GetOwner()));
	return false;
}
