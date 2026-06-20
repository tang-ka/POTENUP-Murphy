#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "GameFramework/Actor.h"
#include "QuestPlacementExampleActor.generated.h"

class AAgentNPCBase;
class AItemBaseActor;
class ATriggerBaseActor;
class USceneComponent;

/**
 * QuestData.csv의 QuestTargetID가 실제 맵 액터에 어떻게 연결되는지 보여주는 CPP 배치 예시입니다.
 * 레벨에 이 액터를 하나 두고 bSpawnOnBeginPlay를 켜면, 이 액터 위치를 기준으로 샘플 액터들이 스폰됩니다.
 */
UCLASS()
class MURPHY_API AQuestPlacementExampleActor : public AActor
{
	GENERATED_BODY()

public:
	AQuestPlacementExampleActor();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest Placement")
	void SpawnExampleForScenario(EScenarioType ScenarioType);

	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest Placement")
	void SpawnAirplaneExample();

	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest Placement")
	void SpawnImmigrationExample();

	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest Placement")
	void SpawnBaggageExample();

private:
	FTransform ToWorldTransform(const FTransform& LocalTransform) const;

	ATriggerBaseActor* SpawnReachLocationTrigger(FName QuestTargetID, const FTransform& LocalTransform, const FVector& BoxExtent);
	AItemBaseActor* SpawnQuestItem(TSubclassOf<AItemBaseActor> ItemClass, FName ItemID, FName QuestTargetID, const FTransform& LocalTransform);
	AAgentNPCBase* SpawnQuestNPC(TSubclassOf<AAgentNPCBase> NPCClass, FName QuestTargetID, const FTransform& LocalTransform);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement", meta = (AllowPrivateAccess = "true"))
	bool bSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement", meta = (AllowPrivateAccess = "true"))
	EScenarioType ScenarioToSpawn = EScenarioType::Prologue_Immigration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATriggerBaseActor> TriggerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AItemBaseActor> ArrivalCardItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AItemBaseActor> CarrierItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAgentNPCBase> AirplaneGuestClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAgentNPCBase> ImmigrationNPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAgentNPCBase> ServiceDeskNPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest Placement|Classes", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAgentNPCBase> CustomsOfficerNPCClass;
};
