// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PlayerViewComponent.h"

#include "Murphy.h"
#include "Actors/Characters/MurphyPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

UPlayerViewComponent::UPlayerViewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerViewComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheDefaultCameraTransform();
}

void UPlayerViewComponent::CacheDefaultCameraTransform()
{
	if (bDefaultCameraCached)
	{
		return;
	}

	AMurphyPlayer* Player = Cast<AMurphyPlayer>(GetOwner());
	if (!Player)
	{
		return;
	}

	USpringArmComponent* CameraBoom = Player->GetCameraBoom();
	UCameraComponent* FollowCamera = Player->GetFollowCamera();
	if (!CameraBoom || !FollowCamera)
	{
		return;
	}

	DefaultBoomAttachParent = CameraBoom->GetAttachParent();
	DefaultBoomAttachSocket = CameraBoom->GetAttachSocketName();
	DefaultArmLength = CameraBoom->TargetArmLength;
	DefaultBoomRelativeLocation = CameraBoom->GetRelativeLocation();
	DefaultCameraRelativeLocation = FollowCamera->GetRelativeLocation();

	bDefaultCameraCached = true;
}

void UPlayerViewComponent::RequestViewState(EPlayerViewState NewState, AActor* InFocusTarget)
{
	if (bIsTransitioning)
	{
		// 정렬이 끝나기 전 들어온 요청은 보류 (기존 bPendingEndChat)
		PendingViewState = NewState;
		return;
	}

	if (CurViewState == NewState)
	{
		return;
	}

	switch (NewState)
	{
	case EPlayerViewState::ThirdPersonFocus:
		{
			FocusTarget = InFocusTarget;
			PendingViewState = NewState;
			bIsTransitioning = true;
			break;
		}
	case EPlayerViewState::FirstPersonTalk:
		{
			ApplyFirstPersonTalk();
			CurViewState = NewState;
			OnViewTransitionComplete.Broadcast(CurViewState);
			break;
		}
	case EPlayerViewState::Idle:
		{
			ApplyExitFirstPerson();
			FocusTarget = nullptr;
			CurViewState = NewState;
			OnViewTransitionComplete.Broadcast(CurViewState);
			break;
		}
	}
}

void UPlayerViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsTransitioning)
	{
		return;
	}

	TickThirdPersonFocusAlign(DeltaTime);
}

void UPlayerViewComponent::TickThirdPersonFocusAlign(float DeltaSeconds)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !FocusTarget)
	{
		bIsTransitioning = false;
		return;
	}

	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(OwnerCharacter->GetActorLocation(), FocusTarget->GetActorLocation());
	TargetRot.Pitch = 0.0f;
	TargetRot.Roll = 0.0f;

	FRotator Rot = FMath::RInterpTo(OwnerCharacter->GetActorRotation(), TargetRot, DeltaSeconds, RotationInterpSpeed);
	OwnerCharacter->SetActorRotation(Rot);

	bool bActorAligned = OwnerCharacter->GetActorRotation().Equals(TargetRot, AlignToleranceDeg);
	bool bCamAligned = true;

	if (AController* Controller = OwnerCharacter->GetController())
	{
		FRotator CamRot = FMath::RInterpTo(Controller->GetControlRotation(), ThirdPersonFocusCamRot, DeltaSeconds, RotationInterpSpeed);
		Controller->SetControlRotation(CamRot);

		bCamAligned = Controller->GetControlRotation().Equals(ThirdPersonFocusCamRot, AlignToleranceDeg);
	}

	if (bActorAligned && bCamAligned)
	{
		bIsTransitioning = false;
		CurViewState = PendingViewState;

		if (CurViewState == EPlayerViewState::Idle)
		{
			ApplyExitFirstPerson();
			FocusTarget = nullptr;
		}

		OnViewTransitionComplete.Broadcast(CurViewState);
	}
}

void UPlayerViewComponent::ApplyFirstPersonTalk()
{
	AMurphyPlayer* Player = Cast<AMurphyPlayer>(GetOwner());
	if (!Player)
	{
		PRINTLOG_SH(TEXT("[View] Owner가 AMurphyPlayer가 아님"));
		return;
	}

	USpringArmComponent* CameraBoom = Player->GetCameraBoom();
	UCameraComponent* FollowCamera = Player->GetFollowCamera();
	if (!CameraBoom || !FollowCamera)
	{
		return;
	}

	// 복귀 기준값을 아직 못 캐싱했다면 여기서 확보
	CacheDefaultCameraTransform();

	CameraBoom->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName(TEXT("headSocket")));
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->SetRelativeLocation(FVector::ZeroVector);

	FollowCamera->SetRelativeLocation(FVector::ZeroVector);

	PRINTLOG_SH(TEXT("[View] Enter FirstPersonTalk"));
}

void UPlayerViewComponent::ApplyExitFirstPerson()
{
	if (CurViewState != EPlayerViewState::FirstPersonTalk)
	{
		return;
	}

	AMurphyPlayer* Player = Cast<AMurphyPlayer>(GetOwner());
	if (!Player)
	{
		return;
	}

	USpringArmComponent* CameraBoom = Player->GetCameraBoom();
	UCameraComponent* FollowCamera = Player->GetFollowCamera();
	if (!CameraBoom || !FollowCamera)
	{
		return;
	}

	if (bDefaultCameraCached && DefaultBoomAttachParent)
	{
		CameraBoom->AttachToComponent(DefaultBoomAttachParent, FAttachmentTransformRules::KeepRelativeTransform, DefaultBoomAttachSocket);
		CameraBoom->TargetArmLength = DefaultArmLength;
		CameraBoom->SetRelativeLocation(DefaultBoomRelativeLocation);
		FollowCamera->SetRelativeLocation(DefaultCameraRelativeLocation);
	}

	PRINTLOG_SH(TEXT("[View] Exit FirstPersonTalk"));
}
