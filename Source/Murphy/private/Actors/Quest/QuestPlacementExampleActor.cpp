#include "Actors/Quest/QuestPlacementExampleActor.h"

#include "Actors/Characters/AgentNPCBase.h"
#include "Actors/Items/ItemBaseActor.h"
#include "Actors/Triggers/TriggerBaseActor.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

AQuestPlacementExampleActor::AQuestPlacementExampleActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerClass = ATriggerBaseActor::StaticClass();
	ArrivalCardItemClass = AItemBaseActor::StaticClass();
	CarrierItemClass = AItemBaseActor::StaticClass();
}

void AQuestPlacementExampleActor::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnExampleForScenario(ScenarioToSpawn);
	}
}

void AQuestPlacementExampleActor::SpawnExampleForScenario(EScenarioType ScenarioType)
{
	switch (ScenarioType)
	{
	case EScenarioType::Tutorial_Airplane:
		SpawnAirplaneExample();
		break;
	case EScenarioType::Prologue_Immigration:
		SpawnImmigrationExample();
		break;
	case EScenarioType::Prologue_Baggage:
		SpawnBaggageExample();
		break;
	case EScenarioType::None:
	default:
		break;
	}
}

void AQuestPlacementExampleActor::SpawnAirplaneExample()
{
	// Q_Airplane_ArrivalCard: ClearCondition=GetItem, QuestTargetID=Item_ArrivalCard
	SpawnQuestItem(
		ArrivalCardItemClass,
		TEXT("Item_ArrivalCard"),
		TEXT("Item_ArrivalCard"),
		FTransform(FRotator::ZeroRotator, FVector(200.0f, -120.0f, 80.0f), FVector::OneVector));

	// Q_Airplane_SmallTalk: ClearCondition=TalkToNPC, QuestTargetID=NPC_AirplaneGuest
	SpawnQuestNPC(
		AirplaneGuestClass,
		TEXT("NPC_AirplaneGuest"),
		FTransform(FRotator(0.0f, 180.0f, 0.0f), FVector(520.0f, 0.0f, 0.0f), FVector::OneVector));
}

void AQuestPlacementExampleActor::SpawnImmigrationExample()
{
	// Q_Immigration_CheckArrivalCard / CheckCustomsForm은 월드 배치가 아니라 가방 UI의 CheckItem 이벤트로 완료됩니다.

	// Q_Immigration_GoToDesk: ClearCondition=ReachLocation, QuestTargetID=Zone_ImmigrationDesk
	SpawnReachLocationTrigger(
		TEXT("Zone_ImmigrationDesk"),
		FTransform(FRotator::ZeroRotator, FVector(700.0f, 0.0f, 100.0f), FVector::OneVector),
		FVector(260.0f, 220.0f, 120.0f));

	// Q_Immigration_SubmitArrivalCard: ClearCondition=TalkToNPC, QuestTargetID=NPC_Immigration
	SpawnQuestNPC(
		ImmigrationNPCClass,
		TEXT("NPC_Immigration"),
		FTransform(FRotator(0.0f, 180.0f, 0.0f), FVector(980.0f, 0.0f, 0.0f), FVector::OneVector));
}

void AQuestPlacementExampleActor::SpawnBaggageExample()
{
	// Q_Baggage_GoToBaggage: ClearCondition=ReachLocation, QuestTargetID=Zone_BaggageSpawn
	SpawnReachLocationTrigger(
		TEXT("Zone_BaggageSpawn"),
		FTransform(FRotator::ZeroRotator, FVector(300.0f, 500.0f, 100.0f), FVector::OneVector),
		FVector(320.0f, 260.0f, 120.0f));

	// Q_Baggage_AskServiceDesk: ClearCondition=TalkToNPC, QuestTargetID=NPC_ServiceDesk
	SpawnQuestNPC(
		ServiceDeskNPCClass,
		TEXT("NPC_ServiceDesk"),
		FTransform(FRotator(0.0f, -90.0f, 0.0f), FVector(720.0f, 500.0f, 0.0f), FVector::OneVector));

	// Q_Baggage_ReturnToBaggage는 CSV의 QuestTargetID를 Zone_BaggageClaim 같은 위치 ID로 바꾸는 것을 권장합니다.
	SpawnReachLocationTrigger(
		TEXT("Zone_BaggageClaim"),
		FTransform(FRotator::ZeroRotator, FVector(300.0f, 900.0f, 100.0f), FVector::OneVector),
		FVector(360.0f, 300.0f, 120.0f));

	// Q_Baggage_TalkToCustoms / PassCustoms: ClearCondition=TalkToNPC, QuestTargetID=NPC_CustomsOfficer
	SpawnQuestNPC(
		CustomsOfficerNPCClass,
		TEXT("NPC_CustomsOfficer"),
		FTransform(FRotator(0.0f, -90.0f, 0.0f), FVector(860.0f, 900.0f, 0.0f), FVector::OneVector));

	// Q_Baggage_GetCarrier: ClearCondition=GetItem, QuestTargetID=Item_Carrier
	SpawnQuestItem(
		CarrierItemClass,
		TEXT("Item_Carrier"),
		TEXT("Item_Carrier"),
		FTransform(FRotator::ZeroRotator, FVector(120.0f, 900.0f, 80.0f), FVector::OneVector));

	// Q_Baggage_OpenCarrier는 월드 배치가 아니라 가방 UI의 CheckItem 이벤트로 완료됩니다.
}

FTransform AQuestPlacementExampleActor::ToWorldTransform(const FTransform& LocalTransform) const
{
	return LocalTransform * GetActorTransform();
}

ATriggerBaseActor* AQuestPlacementExampleActor::SpawnReachLocationTrigger(FName QuestTargetID, const FTransform& LocalTransform, const FVector& BoxExtent)
{
	if (!GetWorld() || !TriggerClass)
	{
		return nullptr;
	}

	const FTransform WorldTransform = ToWorldTransform(LocalTransform);
	ATriggerBaseActor* Trigger = GetWorld()->SpawnActorDeferred<ATriggerBaseActor>(
		TriggerClass,
		WorldTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Trigger)
	{
		return nullptr;
	}

	Trigger->SetQuestTargetID(QuestTargetID);
	if (UBoxComponent* BoxComponent = Cast<UBoxComponent>(Trigger->GetRootComponent()))
	{
		BoxComponent->SetBoxExtent(BoxExtent);
	}

	Trigger->FinishSpawning(WorldTransform);
	return Trigger;
}

AItemBaseActor* AQuestPlacementExampleActor::SpawnQuestItem(TSubclassOf<AItemBaseActor> ItemClass, FName ItemID, FName QuestTargetID, const FTransform& LocalTransform)
{
	if (!GetWorld() || !ItemClass)
	{
		return nullptr;
	}

	const FTransform WorldTransform = ToWorldTransform(LocalTransform);
	AItemBaseActor* Item = GetWorld()->SpawnActorDeferred<AItemBaseActor>(
		ItemClass,
		WorldTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Item)
	{
		return nullptr;
	}

	Item->ConfigureQuestItem(ItemID, QuestTargetID);
	Item->FinishSpawning(WorldTransform);
	return Item;
}

AAgentNPCBase* AQuestPlacementExampleActor::SpawnQuestNPC(TSubclassOf<AAgentNPCBase> NPCClass, FName QuestTargetID, const FTransform& LocalTransform)
{
	if (!GetWorld() || !NPCClass)
	{
		return nullptr;
	}

	const FTransform WorldTransform = ToWorldTransform(LocalTransform);
	AAgentNPCBase* NPC = GetWorld()->SpawnActorDeferred<AAgentNPCBase>(
		NPCClass,
		WorldTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NPC)
	{
		return nullptr;
	}

	NPC->SetQuestTargetID(QuestTargetID);
	NPC->FinishSpawning(WorldTransform);
	return NPC;
}
