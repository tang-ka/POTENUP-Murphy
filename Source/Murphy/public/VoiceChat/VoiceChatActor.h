// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoiceChatActor.generated.h"

class UVoiceRecorderComponent;

UCLASS()
class MURPHY_API AVoiceChatActor : public APawn
{
	GENERATED_BODY()

public:
	AVoiceChatActor();
	
	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	void OnPressRecord();
	void OnPressStop();
	void OnPressPlay();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VoiceChat")
	TObjectPtr<UVoiceRecorderComponent> VoiceRecorderComp;
};
