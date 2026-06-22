// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SessionGameMode.generated.h"

class ASessionCharacter;

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
	/** 접속한 플레이어 좌석에 SessionCharacter를 스폰 (레벨의 PlayerStart 위치 사용) */
	void SpawnSessionCharacterFor(APlayerController* NewPlayer);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	FName InGameLevelKey = TEXT("Airplane");

	/** 룸에서 보여줄 SessionCharacter 클래스 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TSubclassOf<ASessionCharacter> SessionCharacterClass;

	/** 플레이어 퇴장 시 정리하기 위한 스폰 목록 */
	UPROPERTY()
	TArray<TObjectPtr<ASessionCharacter>> SpawnedSessionCharacters;
};
