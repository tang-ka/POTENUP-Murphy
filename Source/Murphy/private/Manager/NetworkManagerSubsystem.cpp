// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager/NetworkManagerSubsystem.h"

#include <string>

#include "Murphy.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Online/OnlineSessionNames.h"

namespace SessionKeys
{
	static const FName SessionName = FName(TEXT("SESSION_NAME"));
	static const FName HostName = FName(TEXT("HOST_NAME"));
}

void UNetworkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();

		OnCreateSessionComplete = FOnCreateSessionCompleteDelegate::CreateUObject(
			this, &UNetworkManagerSubsystem::HandleCreateSessionComplete);
		OnFindSessionsComplete = FOnFindSessionsCompleteDelegate::CreateUObject(
			this, &UNetworkManagerSubsystem::HandleFindSessionsComplete);
		OnJoinSessionComplete = FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &UNetworkManagerSubsystem::HandleJoinSessionComplete);
		OnDestroySessionComplete = FOnDestroySessionCompleteDelegate::CreateUObject(
			this, &UNetworkManagerSubsystem::HandleDestroySessionComplete);
	}

	PRINTLOG_SH(TEXT("NetworkManagerSubsystem Initialized."));
}

void UNetworkManagerSubsystem::Deinitialize()
{
	PRINTLOG_SH(TEXT("NetworkManagerSubsystem Deinitialized."));

	Super::Deinitialize();
}

void UNetworkManagerSubsystem::CreateSession(FSessionInfo Info)
{
	if (CurSessionState != ESessionState::Idle)
	{
		PRINTLOG_SH(TEXT("Cannot create session. Current state: %s"), *UEnum::GetValueAsString(CurSessionState));
		return;
	}
	
	if (!SessionInterface)
	{
		PRINTLOG_SH(TEXT("CreateSession failed: SessionInterface is null."));
		return;
	}
	
	// 1. 세션 생성 완료 델리게이트 등록 및 핸들 저장
	OnCreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionComplete);
	
	// 2. 세션 상태를 Creating으로 변경
	SetSessionState(ESessionState::Creating);
	
	// 3. 세션 설정 구성
	FOnlineSessionSettings SessionSettings;
	{
		SessionSettings.bIsDedicated = false;
		SessionSettings.bIsLANMatch = Info.bIsLAN;
		SessionSettings.NumPublicConnections = Info.MaxPlayers;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
		SessionSettings.bAllowJoinViaPresence = false;
		SessionSettings.bAllowJoinInProgress = false;
		// SessionSettings.bUseLobbiesVoiceChatIfAvailable = true; // 나중에 음성채팅 추가시에 넣기
		
		SessionSettings.Set(
			SessionKeys::SessionName, 
			StringBase64Encode(Info.SessionName), 
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
		SessionSettings.Set(
			SessionKeys::HostName, 
			StringBase64Encode(Info.HostName), 
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	}
	
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	
	PRINTLOG_SH(TEXT("세션 생성 시작 : %s"), *Info.ToString());
	SessionInterface->CreateSession(*netID, NAME_GameSession, SessionSettings);
}

void UNetworkManagerSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteHandle);
	
	if (!bWasSuccessful)
	{
		PRINTLOG_SH(TEXT("세션 생성 실패: %s"), *SessionName.ToString());
		SetSessionState(ESessionState::Failed);
		return;
	}

	PRINTLOG_SH(TEXT("세션 생성 성공: %s"), *SessionName.ToString());
	SetSessionState(ESessionState::InSession);

	// 모든 플레이어를 Lobby 맵으로 이동 (?listen 은 ServerTravel이 자동 처리)
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/Maps/Lv_Session")), true, TEXT("listen?port=7777"));
}

void UNetworkManagerSubsystem::FindSessions(int32 MaxSearchResults, bool bIsLAN)
{
	if (!SessionInterface)
	{
		PRINTLOG_SH(TEXT("FindSessions 실패: SessionInterface is null."));
		return;
	}

	SetSessionState(ESessionState::Finding);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery      = bIsLAN;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	OnFindSessionsCompleteHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsComplete);

	FUniqueNetIdPtr NetID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	SessionInterface->FindSessions(*NetID, SessionSearch.ToSharedRef());

	PRINTLOG_SH(TEXT("세션 검색 시작: MaxResults=%d, bIsLAN=%d"), MaxSearchResults, bIsLAN);
}

void UNetworkManagerSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteHandle);

	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		PRINTLOG_SH(TEXT("세션 검색 실패"));
		SetSessionState(ESessionState::Failed);
		OnFindSessionsCompleted.Broadcast(false, TArray<FOnlineSessionSearchResult>());
		return;
	}

	PRINTLOG_SH(TEXT("세션 검색 완료: %d개 발견"), SessionSearch->SearchResults.Num());
	SetSessionState(ESessionState::Idle);
	OnFindSessionsCompleted.Broadcast(true, SessionSearch->SearchResults);
}

void UNetworkManagerSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!SessionInterface)
	{
		PRINTLOG_SH(TEXT("JoinSession 실패: SessionInterface is null."));
		return;
	}

	SetSessionState(ESessionState::Joining);

	OnJoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionComplete);

	FUniqueNetIdPtr NetID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	SessionInterface->JoinSession(*NetID, NAME_GameSession, SearchResult);

	PRINTLOG_SH(TEXT("세션 참가 시작"));
}

void UNetworkManagerSubsystem::JoinSessionByIndex(int32 Index)
{
	if (!SessionSearch.IsValid())
	{
		PRINTLOG_SH(TEXT("JoinSessionByIndex 실패: SessionSearch is null."));
		return;
	}

	if (!SessionSearch->SearchResults.IsValidIndex(Index))
	{
		PRINTLOG_SH(TEXT("JoinSessionByIndex 실패: 유효하지 않은 인덱스 %d"), Index);
		return;
	}

	JoinSession(SessionSearch->SearchResults[Index]);
}

void UNetworkManagerSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteHandle);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		PRINTLOG_SH(TEXT("세션 참가 실패: Result=%d"), static_cast<int32>(Result));
		SetSessionState(ESessionState::Failed);
		return;
	}

	PRINTLOG_SH(TEXT("세션 참가 성공: %s"), *SessionName.ToString());
	SetSessionState(ESessionState::InSession);

	FString TravelURL;
	if (SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UNetworkManagerSubsystem::DestroySession()
{
	if (!SessionInterface)
	{
		PRINTLOG_SH(TEXT("DestroySession 실패: SessionInterface is null."));
		return;
	}

	SetSessionState(ESessionState::Destroying);

	OnDestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionComplete);
	SessionInterface->DestroySession(NAME_GameSession);

	PRINTLOG_SH(TEXT("세션 삭제 시작"));
}

void UNetworkManagerSubsystem::SetSessionState(ESessionState NewState)
{
	if (CurSessionState == NewState)
	{
		PRINTLOG_SH(TEXT("NewState is the same as current state(%s)."), *UEnum::GetValueAsString(NewState));
		return;
	}

	const ESessionState OldState = CurSessionState;
	CurSessionState = NewState;
	PRINTLOG_SH(TEXT("SessionState Changed %s → %s"),
	            *UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
}



void UNetworkManagerSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteHandle);

	if (!bWasSuccessful)
	{
		PRINTLOG_SH(TEXT("세션 삭제 실패: %s"), *SessionName.ToString());
		SetSessionState(ESessionState::Failed);
		return;
	}

	PRINTLOG_SH(TEXT("세션 삭제 완료: %s"), *SessionName.ToString());
	SetSessionState(ESessionState::Idle);
}

FSessionInfo UNetworkManagerSubsystem::ExtractSessionInfo(const FOnlineSessionSearchResult& SearchResult)
{
	FSessionInfo Info;

	FString EncodedSessionName;
	if (SearchResult.Session.SessionSettings.Get(SessionKeys::SessionName, EncodedSessionName))
	{
		Info.SessionName = StringBase64Decode(EncodedSessionName);
	}

	FString EncodedHostName;
	if (SearchResult.Session.SessionSettings.Get(SessionKeys::HostName, EncodedHostName))
	{
		Info.HostName = StringBase64Decode(EncodedHostName);
	}

	Info.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
	Info.bIsLAN     = SearchResult.Session.SessionSettings.bIsLANMatch;

	return Info;
}

FString UNetworkManagerSubsystem::StringBase64Encode(const FString& str)
{
	// Set 할 때 :: FString -> UTF8(std::string) -> TArray<uint8> -> base64 로 Encode
	std::string utf8String = TCHAR_TO_UTF8(*str);
	TArray<uint8> arrayData = TArray<uint8>((uint8*)(utf8String.c_str()), utf8String.length());
	return FBase64::Encode(arrayData);
}

FString UNetworkManagerSubsystem::StringBase64Decode(const FString& str)
{
	// Get 할 때 :: base64 로 Decode -> TArray<uint8> -> TCHAR
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)(arrayData.GetData()), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}
