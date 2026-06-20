// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MurphyGameStateBase.h"
#include "PrologueGameState.generated.h"

/**
 * 프롤로그 시나리오 전용 GameState입니다.
 * 입국심사는 개인 퀘스트, 수화물 수취장은 공유 퀘스트 정책을 같은 베이스에서 처리합니다.
 */
UCLASS()
class MURPHY_API APrologueGameState : public AMurphyGameStateBase
{
	GENERATED_BODY()
};

