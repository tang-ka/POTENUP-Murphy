// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Components/PlayerViewComponent.h" // EChatViewMode
#include "MurphyGameModeBase.generated.h"

class UCinematicSequenceData;

UCLASS()
class MURPHY_API AMurphyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 씬(맵) 단위로 대화 시점 전환 방식을 설정 - AMurphyPlayer::BeginPlay에서 읽어와 ChatViewMode를 덮어씀
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|View")
	EChatViewMode ChatViewMode = EChatViewMode::ThirdPersonFocus;

	// 이 레벨 진입 시 재생할 시네마틱 시퀀스. 미지정이면 시퀀스 없음.
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Cinematic")
	TObjectPtr<UCinematicSequenceData> LevelCinematic;

	// 멀티 동기 시작 인원 (싱글/테스트=1, 인게임 2인=2). 인원이 모두 도착하면 1회 시작.
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Cinematic")
	int32 RequiredPlayersToStart = 1;

	// 게임 오버 트리거
	UFUNCTION(BlueprintCallable, Category = "Murphy|Game")
	void TriggerGameOver();

protected:
	// PlayerState의 SelectedCharacter 값에 따라 스폰할 폰 클래스를 분기한다.
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// PlayerState의 SelectedCharacter 값에 따라 스폰 위치(PlayerStart)를 분기한다.
	// 레벨의 PlayerStart에 PlayerStartTag("Boy" / "Girl")를 설정해야 동작한다.
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 필요 인원 도착 시 레벨 시네마틱 시퀀스를 1회 시작한다.
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	bool bCinematicSequenceStarted = false;

	// Boy 선택 시 스폰할 폰 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Pawn")
	TSubclassOf<APawn> BoyPawnClass;

	// Girl 선택 시 스폰할 폰 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Pawn")
	TSubclassOf<APawn> GirlPawnClass;
};
