// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Components/PlayerViewComponent.h" // EChatViewMode
#include "MurphyGameModeBase.generated.h"

UCLASS()
class MURPHY_API AMurphyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 씬(맵) 단위로 대화 시점 전환 방식을 설정 - AMurphyPlayer::BeginPlay에서 읽어와 ChatViewMode를 덮어씀
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|View")
	EChatViewMode ChatViewMode = EChatViewMode::ThirdPersonFocus;

	// 게임 오버 트리거
	UFUNCTION(BlueprintCallable, Category = "Murphy|Game")
	void TriggerGameOver();

protected:
	// GameState 생성 직후 서버에서 ChatViewMode를 GameState로 복사 (클라 복제용)
	virtual void InitGameState() override;

	// PlayerState의 SelectedCharacter 값에 따라 스폰할 폰 클래스를 분기한다.
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// PlayerState의 SelectedCharacter 값에 따라 스폰 위치(PlayerStart)를 분기한다.
	// 레벨의 PlayerStart에 PlayerStartTag("Boy" / "Girl")를 설정해야 동작한다.
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// 각 머신 로컬 인트로 완료를 PlayerController RPC로 전달받는 서버측 진입점.
	// 파생 GameMode가 시나리오 시작 시점을 결정한다.
	virtual void NotifyIntroCinematicFinished();

private:
	// Boy 선택 시 스폰할 폰 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Pawn")
	TSubclassOf<APawn> BoyPawnClass;

	// Girl 선택 시 스폰할 폰 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Pawn")
	TSubclassOf<APawn> GirlPawnClass;
};
