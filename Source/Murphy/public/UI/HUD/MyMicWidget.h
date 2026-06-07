// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyMicWidget.generated.h"

/**
 * 
 */
UCLASS()
class MURPHY_API UMyMicWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	void SetRecordingState(bool bIsRecording);
	
};
