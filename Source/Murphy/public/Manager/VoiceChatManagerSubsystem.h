// Fill out your copyright notice in the Description page of Project Settings.

// VoiceChatManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceChatManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVoiceChatState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Connecting  UMETA(DisplayName = "Connecting"),
	Connected   UMETA(DisplayName = "Connected"),
	Recording   UMETA(DisplayName = "Recording"),
	Disconnected UMETA(DisplayName = "Disconnected"),
	Failed      UMETA(DisplayName = "Failed"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChatStateChanged, EVoiceChatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChatConnected, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoiceChatDisconnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChatMuteChanged, bool, bIsMuted);

UCLASS()
class MURPHY_API UVoiceChatManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 연결 관리 ──
	UFUNCTION(BlueprintCallable, Category = "VoiceChat")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "VoiceChat")
	void Disconnect();

	// ── 음성 입력 제어 ──
	UFUNCTION(BlueprintCallable, Category = "VoiceChat")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category = "VoiceChat")
	void StopRecording();

	// ── 음소거 ──
	UFUNCTION(BlueprintCallable, Category = "VoiceChat")
	void SetMuted(bool bMuted);

	UFUNCTION(BlueprintPure, Category = "VoiceChat")
	bool IsMuted() const { return bIsMuted; }

	// ── 상태 조회 ──
	UFUNCTION(BlueprintPure, Category = "VoiceChat")
	EVoiceChatState GetVoiceChatState() const { return CurrentState; }

	// ── 브로드캐스트 ──
	UPROPERTY(BlueprintAssignable, Category = "VoiceChat")
	FOnVoiceChatStateChanged OnVoiceChatStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "VoiceChat")
	FOnVoiceChatConnected OnVoiceChatConnected;

	UPROPERTY(BlueprintAssignable, Category = "VoiceChat")
	FOnVoiceChatDisconnected OnVoiceChatDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "VoiceChat")
	FOnVoiceChatMuteChanged OnVoiceChatMuteChanged;

private:
	void SetVoiceChatState(EVoiceChatState NewState);

private:
	EVoiceChatState CurrentState = EVoiceChatState::Idle;

	bool bIsMuted = false;
};

