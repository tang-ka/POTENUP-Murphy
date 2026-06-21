
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggerBaseActor.generated.h"

class UBoxComponent;
class UQuestEventNotifyComponent;

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

	/** 구역 감지 결과를 퀘스트 서버 RPC로 전달하는 공통 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components")
	TObjectPtr<UQuestEventNotifyComponent> QuestEventNotifier;

public:
	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest")
	void SetQuestTargetID(FName InQuestTargetID);

private:
	/** 플레이어가 구역에 진입하면 QuestEventNotifier로 ReachLocation 퀘스트 완료 통보 */
	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

