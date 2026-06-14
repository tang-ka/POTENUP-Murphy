

#include "Actors/Triggers/TriggerBaseActor.h"

#include "Murphy.h"
#include "Actors/Characters/MurphyPlayer.h"

#include "Components/BoxComponent.h"
#include "Manager/ScenarioSubsystem.h"
#include "Kismet/GameplayStatics.h"

ATriggerBaseActor::ATriggerBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
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

void ATriggerBaseActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 플레이어가 진입했는지 확인
	if (!Cast<AMurphyPlayer>(OtherActor))
	{
		return;
	}

	// QuestTargetID가 설정되지 않은 경우 무시
	if (QuestTargetID.IsNone())
	{
		PRINTLOGE_JW(TEXT("퀘스트 ID 가 설정 되지 않음!"));
		return;
	}

	// ScenarioSubsystem에 ReachLocation 퀘스트 완료 통보
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UScenarioSubsystem* ScenarioSS = GI->GetSubsystem<UScenarioSubsystem>())
		{
			ScenarioSS->NotifyQuestConditionMet(QuestTargetID, EQuestClearCondition::ReachLocation);
		}
	}
}
