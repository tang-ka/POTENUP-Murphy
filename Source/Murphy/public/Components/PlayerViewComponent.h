// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerViewComponent.generated.h"

class USceneComponent;

UENUM(BlueprintType)
enum class EPlayerViewState : uint8
{
	Idle,				// 3인칭 자유 시점
	ThirdPersonFocus,	// 3인칭 대화 (NPC 방향/카메라 고정)
	FirstPersonTalk		// 1인칭 대화
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewTransitionComplete, EPlayerViewState, ReachedState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MURPHY_API UPlayerViewComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerViewComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 시점 상태 전환 요청. ThirdPersonTalk 요청 시 InFocusTarget을 바라보도록 보간 시작 */
	UFUNCTION(BlueprintCallable, Category = "Murphy|View")
	void RequestViewState(EPlayerViewState NewState, AActor* InFocusTarget = nullptr);

	UFUNCTION(BlueprintPure, Category = "Murphy|View")
	EPlayerViewState GetViewState() const
	{
		return CurViewState;
	}

	UFUNCTION(BlueprintPure, Category = "Murphy|View")
	bool IsTransitioning() const
	{
		return bIsTransitioning;
	}

	/** 보간(정렬)이 끝나고 CurViewState가 실제로 갱신된 시점에 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "Murphy|View")
	FOnViewTransitionComplete OnViewTransitionComplete;

protected:
	virtual void BeginPlay() override;

private:
	void TickThirdPersonFocusAlign(float DeltaSeconds);

	void ApplyFirstPersonTalk();
	void ApplyExitFirstPerson();

	/** 게임 시작 시점의 CameraBoom/FollowCamera 배치를 캐싱 (1인칭 복귀 시 기준값으로 사용) */
	void CacheDefaultCameraTransform();

private:
	// 3인칭 대화 시 고정시킬 카메라 회전 (기존 CamTargetRot)
	UPROPERTY(EditAnywhere, Category = "Murphy|View")
	FRotator ThirdPersonFocusCamRot = FRotator(345.0f, 245.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Murphy|View")
	float RotationInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Murphy|View")
	float AlignToleranceDeg = 2.0f;

	UPROPERTY(VisibleAnywhere, Category = "Murphy|View")
	EPlayerViewState CurViewState = EPlayerViewState::Idle;

	// 보간 완료 후 실제로 적용할 목표 상태 (기존 bPendingEndChat의 일반화)
	EPlayerViewState PendingViewState = EPlayerViewState::Idle;

	UPROPERTY()
	TObjectPtr<AActor> FocusTarget;

	// 기존 bIsAligningWithNPC
	bool bIsTransitioning = false;

	// === 1인칭 복귀용 기본 카메라 배치 캐시 ===
	bool bDefaultCameraCached = false;

	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultBoomAttachParent;

	FName DefaultBoomAttachSocket = NAME_None;

	float DefaultArmLength = 100.0f;
	FVector DefaultBoomRelativeLocation = FVector::ZeroVector;
	FVector DefaultCameraRelativeLocation = FVector::ZeroVector;
};
