// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager/VoiceChatManagerSubsystem.h"

#include "Murphy.h"

void UVoiceChatManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PRINTLOG_SH(TEXT("VoiceChatManagerSubsystem Initialized."));
}

void UVoiceChatManagerSubsystem::Deinitialize()
{
	if (CurrentState == EVoiceChatState::Connected || CurrentState == EVoiceChatState::Recording)
	{
		Disconnect();
	}

	PRINTLOG_SH(TEXT("VoiceChatManagerSubsystem Deinitialized."));

	Super::Deinitialize();
}

void UVoiceChatManagerSubsystem::SetVoiceChatState(EVoiceChatState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	OnVoiceChatStateChanged.Broadcast(CurrentState);

	PRINTLOG_SH(TEXT("VoiceChatState Changed: %d"), static_cast<int32>(CurrentState));
}

void UVoiceChatManagerSubsystem::Connect()
{
	if (CurrentState == EVoiceChatState::Connected || CurrentState == EVoiceChatState::Connecting)
	{
		PRINTLOG_SH(TEXT("VoiceChat is already connecting or connected."));
		return;
	}

	SetVoiceChatState(EVoiceChatState::Connecting);

	// TODO: 실제 VoiceChat 연결 로직 구현
	PRINTLOG_SH(TEXT("VoiceChat Connect called."));

	// 연결 성공 시 아래를 호출
	SetVoiceChatState(EVoiceChatState::Connected);
	OnVoiceChatConnected.Broadcast(true);
}

void UVoiceChatManagerSubsystem::Disconnect()
{
	if (CurrentState == EVoiceChatState::Idle || CurrentState == EVoiceChatState::Disconnected)
	{
		PRINTLOG_SH(TEXT("VoiceChat is already disconnected."));
		return;
	}

	// TODO: 실제 VoiceChat 연결 해제 로직 구현
	PRINTLOG_SH(TEXT("VoiceChat Disconnect called."));

	SetVoiceChatState(EVoiceChatState::Disconnected);
	OnVoiceChatDisconnected.Broadcast();
}

void UVoiceChatManagerSubsystem::StartRecording()
{
	if (CurrentState != EVoiceChatState::Connected)
	{
		PRINTLOG_SH(TEXT("VoiceChat is not connected. Cannot start recording."));
		return;
	}

	if (bIsMuted)
	{
		PRINTLOG_SH(TEXT("VoiceChat is muted. Cannot start recording."));
		return;
	}

	// TODO: 실제 녹음 시작 로직 구현
	SetVoiceChatState(EVoiceChatState::Recording);

	PRINTLOG_SH(TEXT("VoiceChat StartRecording called."));
}

void UVoiceChatManagerSubsystem::StopRecording()
{
	if (CurrentState != EVoiceChatState::Recording)
	{
		PRINTLOG_SH(TEXT("VoiceChat is not recording."));
		return;
	}

	// TODO: 실제 녹음 중지 로직 구현
	SetVoiceChatState(EVoiceChatState::Connected);

	PRINTLOG_SH(TEXT("VoiceChat StopRecording called."));
}

void UVoiceChatManagerSubsystem::SetMuted(bool bMuted)
{
	if (bIsMuted == bMuted)
	{
		return;
	}

	bIsMuted = bMuted;
	OnVoiceChatMuteChanged.Broadcast(bIsMuted);

	if (bIsMuted && CurrentState == EVoiceChatState::Recording)
	{
		StopRecording();
	}

	PRINTLOG_SH(TEXT("VoiceChat SetMuted: %d"), bIsMuted);
}

