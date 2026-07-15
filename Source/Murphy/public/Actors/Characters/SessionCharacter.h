// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/GameDataTypes.h"
#include "SessionCharacter.generated.h"

class AMurphyPlayerState;
class UCapsuleComponent;
class UChildActorComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class UWidgetComponent;
class USessionPlayerStateWidget;

UCLASS()
class MURPHY_API ASessionCharacter : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASessionCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** GameMode가 스폰 직후 호출 — 이 액터가 어떤 플레이어를 나타내는지 지정 */
	void InitializeForPlayer(AMurphyPlayerState* InPlayerState);

	AMurphyPlayerState* GetOwningPlayerState() const
	{
		return OwningPlayerState;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnRep_OwningPlayerState();

	/** PlayerState의 OnSessionRoomStateChanged 델리게이트 콜백 */
	void HandleSessionRoomStateChanged();

	/** OwningPlayerState 값 기준으로 메쉬 + 상태 위젯을 모두 갱신 */
	void RefreshVisuals();

	/** 현재 OwningPlayerState->SelectedCharacter 값에 맞는 메쉬로 갱신 */
	void RefreshCharacterMesh();

	/** 현재 OwningPlayerState->SelectedCharacter 값에 맞는 메타휴먼으로 갱신 */
	void RefreshMetaHumanCharacter();

	/** 현재 SelectedCharacter 값에 맞는 메타휴먼 BP 클래스를 반환 */
	TSubclassOf<AActor> ResolveMetaHumanClass() const;

	/** 닉네임 / 준비완료 표시 위젯 갱신 */
	void RefreshPlayerStateWidget();

private:
#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCapsuleComponent> CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UChildActorComponent> MetaHumanActorComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> PlayerStateWidgetComp;
#pragma endregion

#pragma region Session
	// 이 캐릭터가 나타내는 플레이어 (서버가 스폰 직후 지정, 클라이언트는 복제로 수신)
	UPROPERTY(ReplicatedUsing = OnRep_OwningPlayerState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AMurphyPlayerState> OwningPlayerState;

	// 캐릭터 타입별 표시 메쉬 (BP에서 설정)
	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session")
	TObjectPtr<USkeletalMesh> BoyMesh;

	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session")
	TObjectPtr<USkeletalMesh> GirlMesh;

	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session|MetaHuman")
	TSubclassOf<AActor> BoyMetaHumanClass;

	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session|MetaHuman")
	TSubclassOf<AActor> GirlMetaHumanClass;

	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session|MetaHuman")
	FVector MetaHumanRelativeLocation = FVector(0.f, 0.f, -88.f);

	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session|MetaHuman")
	FRotator MetaHumanRelativeRotation = FRotator(0.f, -90.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category="Murphy|Session|MetaHuman")
	FVector MetaHumanRelativeScale = FVector(1.f, 1.f, 1.f);
#pragma endregion
};
