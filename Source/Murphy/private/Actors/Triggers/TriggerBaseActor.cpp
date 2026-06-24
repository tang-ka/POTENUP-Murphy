

#include "Actors/Triggers/TriggerBaseActor.h"

#include "Murphy.h"
#include "Actors/Characters/MurphyPlayer.h"
#include "Components/QuestEventNotifyComponent.h"
#include "Manager/DataManager.h"

#include "Components/BoxComponent.h"

ATriggerBaseActor::ATriggerBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	QuestEventNotifier = CreateDefaultSubobject<UQuestEventNotifyComponent>(TEXT("QuestEventNotifier"));
}

void ATriggerBaseActor::BeginPlay()
{
	Super::BeginPlay();

	// Overlap 이벤트 바인딩
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATriggerBaseActor::OnTriggerBoxBeginOverlap);
	}
}

void ATriggerBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATriggerBaseActor::SetQuestTargetID(FName InQuestTargetID)
{
	if (QuestEventNotifier)
	{
		QuestEventNotifier->SetQuestTargetID(InQuestTargetID);
	}
}

void ATriggerBaseActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 플레이어가 진입했는지 확인
	AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(OtherActor);
	if (!IsValid(MurphyPlayer))
	{
		return;
	}

	if (!QuestEventNotifier)
	{
		return;
	}

	if (QuestEventNotifier->GetQuestTargetID().IsNone())
	{
		PRINTLOGE_JW(TEXT("[TriggerBaseActor] QuestTargetID가 설정되지 않았습니다. Owner: %s"), *GetNameSafe(this));
		return;
	}

	WarnIfQuestTargetLooksLikeQuestID(QuestEventNotifier->GetQuestTargetID());

	// 트리거는 장소 감지만 담당하고, 서버 퀘스트 통보는 공통 컴포넌트가 처리합니다.
	QuestEventNotifier->NotifyQuestComplete(MurphyPlayer, EQuestCondition::ReachLocation);
}

void ATriggerBaseActor::WarnIfQuestTargetLooksLikeQuestID(FName QuestTargetID) const
{
	if (QuestTargetID.IsNone())
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UDataManager* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManager>() : nullptr;
	if (!DataManager)
	{
		return;
	}

	if (DataManager->GetAllQuestRowNames().Contains(QuestTargetID))
	{
		PRINTLOGE_JW(
			TEXT("[TriggerBaseActor] QuestTargetID가 퀘스트 RowName처럼 보입니다. Trigger에는 QuestID가 아니라 DT_Quest의 QuestTargetID 값을 넣어야 합니다. Owner: %s, 입력값: %s"),
			*GetNameSafe(this),
			*QuestTargetID.ToString());
	}
}
