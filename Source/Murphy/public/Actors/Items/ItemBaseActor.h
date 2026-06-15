
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/GameDataTypes.h"
#include "Framework/InteractableInterface.h"
#include "ItemBaseActor.generated.h"

class UBoxComponent;
class UWidgetComponent;
class AMurphyPlayer;

/**
 * 게임 월드에 배치되는 획득 가능한 아이템 액터 Base 클래스
 * - 플레이어가 InteractionBox에 Overlap 시 InteractPromptWidget(힌트 UI) 표시
 * - F키(InteractPressed) → 시야각 체크 → Interact() → 가방 추가 + 퀘스트 통보
 */
UCLASS()
class MURPHY_API AItemBaseActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AItemBaseActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/**
	 * 플레이어가 F키를 눌렀을 때 InteractPressed()에서 호출
	 * 가방 추가 + 퀘스트 완료 통보
	 * @param Player - 상호작용을 시도하는 플레이어
	 */
	virtual void Interact(AMurphyPlayer* Player) override;

protected:
	// === Components ===
	/** 플레이어 접근 인식을 위한 콜리전 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components")
	TObjectPtr<UBoxComponent> InteractionBox;

	/** 아이템 Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components")
	TObjectPtr<UStaticMeshComponent> InteractMesh;

	/** 아이템 위에 떠 있는 상호작용 힌트 UI (기본 숨김, 로컬 플레이어 Overlap 시 표시) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Components")
	TObjectPtr<UWidgetComponent> InteractUIWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Components")
	TSubclassOf<UUserWidget> InteractUIClass;
	
	// === 에디터 설정 ===
	/** 아이템 고유 ID (DataManager 로드용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	FName ItemID;

	/** 퀘스트 CSV의 QuestTargetID 컬럼값과 일치시킬 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Murphy|Item")
	FName QuestTargetID;

	/** 상호작용 가능한 시야각 임계값 (도 단위, 기본 60도) */
	UPROPERTY(EditAnywhere, Category = "Murphy|Item", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float InteractAngleDeg = 60.0f;

public:
	/**
	 * 플레이어 시야 내에 이 아이템이 있는지 Dot Product로 체크
	 * @param Player - 체크할 플레이어
	 * @return true이면 시야 내에 있음
	 */
	bool IsInPlayerSight(const AMurphyPlayer* Player) const;

private:
	/** Overlap 시작 → 로컬 플레이어이면 InteractPromptWidget 표시 */
	UFUNCTION()
	void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Overlap 종료 → 로컬 플레이어이면 InteractPromptWidget 숨김 */
	UFUNCTION()
	void OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

