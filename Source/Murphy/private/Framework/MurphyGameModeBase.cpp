// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MurphyGameModeBase.h"
#include "Murphy.h"
#include "Data/GameDataTypes.h"
#include "Framework/MurphyPlayerState.h"

UClass* AMurphyGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const AMurphyPlayerState* MurphyPS = InController ? InController->GetPlayerState<AMurphyPlayerState>() : nullptr;
	if (!MurphyPS)
	{
		PRINTLOG_SH(TEXT("GetDefaultPawnClassForController 실패 — PlayerState 없음, 기본 폰으로 대체"));
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	switch (MurphyPS->SelectedCharacter)
	{
	case EPlayerCharacterType::BoyCharacter:
		{
			if (BoyPawnClass)
			{
				PRINTLOG_SH(TEXT("폰 클래스 결정 — Boy"));
				return BoyPawnClass;
			}
			break;
		}
	case EPlayerCharacterType::GirlCharacter:
		{
			if (GirlPawnClass)
			{
				PRINTLOG_SH(TEXT("폰 클래스 결정 — Girl"));
				return GirlPawnClass;
			}
			break;
		}
	default:
		{
			break;
		}
	}

	PRINTLOG_SH(TEXT("GetDefaultPawnClassForController 경고 — 선택값(%s)에 맞는 폰 클래스 미설정, 기본 폰으로 대체"), *UEnum::GetValueAsString(MurphyPS->SelectedCharacter));
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
