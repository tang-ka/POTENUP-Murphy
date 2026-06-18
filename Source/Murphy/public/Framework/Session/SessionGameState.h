// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SessionGameState.generated.h"

/**
 * 멀티플레이 세션 레벨에서 사용하는 GameState
 */
UCLASS()
class MURPHY_API ASessionGameState : public AGameState
{
	GENERATED_BODY()

public:
	/** 현재 세션에 접속한 플레이어 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Session|State")
	int32 ConnectedPlayerCount = 0;

	/** 세션 최대 수용 인원 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Session|State")
	int32 MaxPlayerCount = 0;

	/** 세션 이름 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Session|State")
	FString SessionName;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};


