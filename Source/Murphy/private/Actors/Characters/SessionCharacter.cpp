// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Characters/SessionCharacter.h"

#include "Murphy.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Framework/MurphyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/SessionPlayerStateWidget.h"

// Sets default values
ASessionCharacter::ASessionCharacter()
{
	// 정적으로 메쉬만 보여주는 액터이므로 Tick 불필요
	PrimaryActorTick.bCanEverTick = false;

	// GameMode(서버)가 스폰하므로 모든 클라이언트에 복제되어야 함
	bReplicates = true;

	// ACharacter와 동일한 표준 치수 (Radius 34 / HalfHeight 88)
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(34.f, 88.f);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = CapsuleComp;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CapsuleComp);
	// 캡슐 바닥에 발이 닿도록 오프셋 (ACharacter 기본값과 동일)
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlayerStateWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerStateWidgetComp"));
	PlayerStateWidgetComp->SetupAttachment(CapsuleComp);
	PlayerStateWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	PlayerStateWidgetComp->SetDrawSize(FVector2D(200.f, 80.f));
	PlayerStateWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	PlayerStateWidgetComp->SetWidgetClass(USessionPlayerStateWidget::StaticClass());
}

void ASessionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASessionCharacter, OwningPlayerState);
}

// Called when the game starts or when spawned
void ASessionCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASessionCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwningPlayerState)
	{
		OwningPlayerState->OnSessionRoomStateChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ASessionCharacter::InitializeForPlayer(AMurphyPlayerState* InPlayerState)
{
	if (!InPlayerState)
	{
		PRINTLOG_SH(TEXT("InitializeForPlayer 실패 — PlayerState가 유효하지 않습니다."));
		return;
	}

	OwningPlayerState = InPlayerState;
	OwningPlayerState->OnSessionRoomStateChanged.AddUObject(this, &ASessionCharacter::HandleSessionRoomStateChanged);

	RefreshVisuals();

	PRINTLOG_SH(TEXT("SessionCharacter 초기화 완료 — Player: %s"), *InPlayerState->GetPlayerName());
}

void ASessionCharacter::OnRep_OwningPlayerState()
{
	if (!OwningPlayerState)
	{
		return;
	}

	// 클라이언트는 복제로 OwningPlayerState를 받은 시점에 바인딩
	OwningPlayerState->OnSessionRoomStateChanged.AddUObject(this, &ASessionCharacter::HandleSessionRoomStateChanged);

	RefreshVisuals();
}

void ASessionCharacter::HandleSessionRoomStateChanged()
{
	RefreshVisuals();
}

void ASessionCharacter::RefreshVisuals()
{
	RefreshCharacterMesh();
	RefreshPlayerStateWidget();
}

void ASessionCharacter::RefreshCharacterMesh()
{
	if (!OwningPlayerState || !MeshComp)
	{
		PRINTLOG_SH(TEXT("RefreshCharacterMesh 실패 — OwningPlayerState 또는 MeshComp가 유효하지 않습니다."));
		return;
	}

	USkeletalMesh* NewMesh = nullptr;
	switch (OwningPlayerState->SelectedCharacter)
	{
	case EPlayerCharacterType::BoyCharacter:
		NewMesh = BoyMesh;
		break;
	case EPlayerCharacterType::GirlCharacter:
		NewMesh = GirlMesh;
		break;
	default:
		NewMesh = BoyMesh;
		break;
	}

	MeshComp->SetSkeletalMesh(NewMesh);

	PRINTLOG_SH(TEXT("캐릭터 메쉬 갱신 — %s"), *UEnum::GetValueAsString(OwningPlayerState->SelectedCharacter));
}

void ASessionCharacter::RefreshPlayerStateWidget()
{
	if (!OwningPlayerState || !PlayerStateWidgetComp)
	{
		return;
	}

	USessionPlayerStateWidget* StateWidget = Cast<USessionPlayerStateWidget>(PlayerStateWidgetComp->GetUserWidgetObject());
	if (!StateWidget)
	{
		// 데디케이트 서버 등 위젯이 생성되지 않는 환경에서는 정상적으로 스킵됨
		return;
	}

	StateWidget->SetPlayerName(OwningPlayerState->GetPlayerName());
	StateWidget->SetReadyState(OwningPlayerState->bIsReady);
}
