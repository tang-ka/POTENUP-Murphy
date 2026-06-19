
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggerBaseActor.generated.h"

class UBoxComponent;

/**
 * 특정 구역 진입 시 ReachLocation 퀘스트를 완료 처리하는 트리거 액터 Base 클래스
 * QuestTargetID를 에디터에서 설정하고, 퀘스트 CSV의 QuestTargetID 컬럼값과 일치시켜야 합니다.
 */
UCLASS()
class MURPHY_API ATriggerBaseActor : public AActor
{
	GENERATED_BODY()

public:
	ATriggerBaseActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	// === Components ===
	/** 구역 진입 감지용 콜리전 박스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

public:
	// === 에디터 설정 ===
	/** 퀘스트 CSV의 QuestTargetID 컬럼값과 일치시킬 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest")
	FName QuestTargetID;

private:
	/** 플레이어가 구역에 진입하면 ScenarioSubsystem에 ReachLocation 퀘스트 완료 통보 */
	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

