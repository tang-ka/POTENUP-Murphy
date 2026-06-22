

#include "Actors/Triggers/TriggerBaseActor.h"

#include "Murphy.h"
#include "Actors/Characters/MurphyPlayer.h"
#include "Components/QuestEventNotifyComponent.h"

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

	// 트리거는 장소 감지만 담당하고, 서버 퀘스트 통보는 공통 컴포넌트가 처리합니다.
	QuestEventNotifier->NotifyQuestComplete(MurphyPlayer, EQuestClearCondition::ReachLocation);
}
