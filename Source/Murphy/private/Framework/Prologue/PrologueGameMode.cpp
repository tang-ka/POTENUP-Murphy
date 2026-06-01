// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Prologue/PrologueGameMode.h"

#include "Murphy.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void APrologueGameMode::BeginPlay()
{
	Super::BeginPlay();

	FLatentActionInfo LatentInfo;
	UGameplayStatics::LoadStreamLevel(
		this,
		TEXT("SubLevel_Immigration"),
		true,
		false,
		LatentInfo);

	// 로드 요청 후 ULevelStreaming 객체에 델리게이트 연결
	ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(
		this,
		TEXT("SubLevel_Immigration"));

	if (StreamingLevel)
	{
		StreamingLevel->OnLevelLoaded.AddDynamic(this, &APrologueGameMode::OnImmigrationLevelLoaded);
		StreamingLevel->OnLevelShown.AddDynamic(this, &APrologueGameMode::OnImmigrationLevelShown);
	}
}

void APrologueGameMode::TransitionToBaggageClaim()
{
	// 1. BaggageClaim 델리게이트 먼저 연결
	ULevelStreaming* BaggageLevel = UGameplayStatics::GetStreamingLevel(
		this, TEXT("SubLevel_BaggageClaim"));

	if (BaggageLevel)
	{
		BaggageLevel->OnLevelShown.AddDynamic(
			this, &APrologueGameMode::OnBaggageClaimLevelShown);
	}

	// 2. Immigration 언로드
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName("OnImmigrationLevelHidden");
	LatentInfo.UUID = 2;
	LatentInfo.Linkage = 0;

	UGameplayStatics::UnloadStreamLevel(
		this,
		TEXT("SubLevel_Immigration"),
		LatentInfo,
		false);
}

void APrologueGameMode::OnImmigrationLevelLoaded()
{
	PRINTLOG_SH(TEXT("OnImmigrationLevelLoaded called"));
}

void APrologueGameMode::OnImmigrationLevelShown()
{
	PRINTLOG_SH(TEXT("OnImmigrationLevelShown called"));
	
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.IsEmpty())
	{
		PRINTLOG_SH(TEXT("PlayerStart not found"));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	Pawn->SetActorLocationAndRotation(
		PlayerStarts[0]->GetActorLocation(),
		PlayerStarts[0]->GetActorRotation());

}

void APrologueGameMode::OnImmigrationLevelHidden()
{
	// 3. Immigration 완전히 내려간 후 BaggageClaim 로드
	FLatentActionInfo LatentInfo;
	UGameplayStatics::LoadStreamLevel(
		this,
		TEXT("SubLevel_BaggageClaim"),
		true,
		false,
		LatentInfo);
}

void APrologueGameMode::OnBaggageClaimLevelShown()
{
	// 4. 플레이어 위치 이동
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.IsEmpty()) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->GetPawn()) return;

	PC->GetPawn()->SetActorLocationAndRotation(
		PlayerStarts[0]->GetActorLocation(),
		PlayerStarts[0]->GetActorRotation());
}
