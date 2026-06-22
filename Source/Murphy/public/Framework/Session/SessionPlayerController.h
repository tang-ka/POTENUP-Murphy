// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/GameDataTypes.h"
#include "SessionPlayerController.generated.h"

class USessionMainHUDWidget;

/**
 * Lv_Session(룸)에서 사용하는 PlayerController
 */
UCLASS()
class MURPHY_API ASessionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 캐릭터 선택 (모든 플레이어)
	UFUNCTION(Server, Reliable)
	void Server_SelectCharacter(EPlayerCharacterType NewCharacter);

	// 준비 완료 토글 (클라이언트)
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bReady);

	// 여행지 선택 (호스트 전용)
	UFUNCTION(Server, Reliable)
	void Server_SelectDestination(ETravelDestination NewDestination);

	// 게임 시작 요청 (호스트 전용)
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();

	virtual void OnRep_PlayerState() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	virtual void BeginPlay() override;
	
private:
	/** PS가 준비된 시점에 HUD에 역할 반영 및 델리게이트 바인딩 */
	void TryBindPlayerStateToHUD();

private:
	/** 에디터 BP에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|UI")
	TSubclassOf<USessionMainHUDWidget> SessionMainHUDClass;

	UPROPERTY()
	TObjectPtr<USessionMainHUDWidget> SessionMainHUDWidget;

	/** PS 바인딩 중복 방지 플래그 */
	bool bPlayerStateBoundToHUD = false;
};
