// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TriggerBaseActor.h"
#include "BaggageTriggerActor.generated.h"

UCLASS()
class MURPHY_API ABaggageTriggerActor : public ATriggerBaseActor
{
	GENERATED_BODY()

public:
	ABaggageTriggerActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
