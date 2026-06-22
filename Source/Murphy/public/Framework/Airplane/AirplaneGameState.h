// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MurphyGameStateBase.h"
#include "AirplaneGameState.generated.h"

/**
 * 기내 시나리오 전용 GameState입니다.
 * 공통 AMurphyGameStateBase를 통해 개인 퀘스트 진행도와 시나리오 종료 정책을 사용합니다.
 */
UCLASS()
class MURPHY_API AAirplaneGameState : public AMurphyGameStateBase
{
	GENERATED_BODY()
};

