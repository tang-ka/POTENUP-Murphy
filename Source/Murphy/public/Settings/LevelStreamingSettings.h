// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LevelStreamingSettings.generated.h"

/**
 * Travel할 레벨을 Map 형태로 에디터에서 관리하기 위한 설정 클래스
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Level Streaming Settings"))
class MURPHY_API ULevelStreamingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 레벨 키(FName)와 레벨 경로(TSoftObjectPtr<UWorld>)를 매핑하는 맵
	UPROPERTY(Config, EditAnywhere, Category="Level Streaming")
	TMap<FName, TSoftObjectPtr<UWorld>> LevelMap;
};

