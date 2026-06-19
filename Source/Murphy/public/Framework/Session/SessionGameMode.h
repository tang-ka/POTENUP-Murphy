// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SessionGameMode.generated.h"

/**
 * 멀티플레이 세션 레벨에서 사용하는 GameMode
 */
UCLASS()
class MURPHY_API ASessionGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASessionGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void TryStartGame();

protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	                              const FString& Options, const FString& Portal) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	FName InGameLevelKey = TEXT("Airplane");
};
