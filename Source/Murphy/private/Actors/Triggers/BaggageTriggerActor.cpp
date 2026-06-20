// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Triggers/BaggageTriggerActor.h"


// Sets default values
ABaggageTriggerActor::ABaggageTriggerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABaggageTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaggageTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

