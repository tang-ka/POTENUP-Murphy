// Fill out your copyright notice in the Description page of Project Settings.


#include "LogTestActor.h"

#include "Murphy.h"
#include "Settings/LogSettings.h"


// Sets default values
ALogTestActor::ALogTestActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALogTestActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALogTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	PRINTLOG_JW(TEXT("JW 로그 테스트: DeltaTime = %f"), DeltaTime);
	PRINTLOG_HJ(TEXT("HJ 로그 테스트: DeltaTime = %f"), DeltaTime);
	PRINTLOG_SH(TEXT("SH 로그 테스트: DeltaTime = %f"), DeltaTime);
	PRINTLOG_CW(TEXT("CW 로그 테스트: DeltaTime = %f"), DeltaTime);
}

