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
	
protected:
	// PlayerState의 SelectedCharacter 값에 따라 스폰할 폰 클래스를 분기한다.
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// Boy 선택 시 스폰할 폰 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Pawn")
	TSubclassOf<APawn> BoyPawnClass;

	// Girl 선택 시 스폰할 폰 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Murphy|Pawn")
	TSubclassOf<APawn> GirlPawnClass;
};
