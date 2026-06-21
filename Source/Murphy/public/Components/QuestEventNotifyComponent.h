#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/GameDataTypes.h"
#include "QuestEventNotifyComponent.generated.h"

class AMurphyPlayerController;

/**
 * Overlap, 대화, 아이템 획득 같은 입력 시점과 퀘스트 서버 통보 로직을 분리하기 위한 컴포넌트입니다.
 * 이 컴포넌트는 Collision을 직접 감지하지 않고, 호출자가 넘긴 InstigatorActor 기준으로 PlayerController RPC만 호출합니다.
 */
UCLASS(ClassGroup = (Murphy), meta = (BlueprintSpawnableComponent))
class MURPHY_API UQuestEventNotifyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestEventNotifyComponent();

	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest")
	void SetQuestTargetID(FName InQuestTargetID);

	UFUNCTION(BlueprintPure, Category = "Murphy|Quest")
	FName GetQuestTargetID() const { return QuestTargetID; }

	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest")
	bool NotifyQuestStart(AActor* InstigatorActor, EQuestStartCondition ConditionOverride = EQuestStartCondition::None) const;

	UFUNCTION(BlueprintCallable, Category = "Murphy|Quest")
	bool NotifyQuestComplete(AActor* InstigatorActor, EQuestClearCondition ConditionOverride = EQuestClearCondition::None) const;

protected:
	/** 퀘스트 CSV의 QuestTargetID 컬럼값과 일치시킬 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest")
	FName QuestTargetID;

	/** ConditionOverride가 None일 때 사용할 기본 시작 조건 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest")
	EQuestStartCondition DefaultStartCondition = EQuestStartCondition::None;

	/** ConditionOverride가 None일 때 사용할 기본 완료 조건 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Quest")
	EQuestClearCondition DefaultClearCondition = EQuestClearCondition::None;

private:
	AMurphyPlayerController* ResolveMurphyPlayerController(AActor* InstigatorActor) const;
	bool HasValidQuestTarget() const;
};
