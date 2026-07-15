// Fill out your copyright notice in the Description page of Project Settings.

// NetworkManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetworkManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class ESessionState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Creating    UMETA(DisplayName = "Creating"),
	Finding     UMETA(DisplayName = "Finding"),
	Joining     UMETA(DisplayName = "Joining"),
	InSession   UMETA(DisplayName = "InSession"),
	Destroying  UMETA(DisplayName = "Destroying"),
	Failed      UMETA(DisplayName = "Failed"),
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFindSessionsCompletedDelegate, bool, const TArray<FOnlineSessionSearchResult>&);

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SessionName;
	
	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 2;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsLAN = true;

	inline FString ToString() const
	{
		return FString::Printf(TEXT("SessionName: %s, HostName: %s, MaxPlayers: %d, bIsLAN: %s"),
			*SessionName, *HostName, MaxPlayers, bIsLAN ? TEXT("true") : TEXT("false"));
	}
};

UCLASS()
class MURPHY_API UNetworkManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	

#pragma region Session Management
	void CreateSession(FSessionInfo Info);
	void FindSessions(int32 MaxSearchResults, bool bIsLAN);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void JoinSessionByIndex(int32 Index);
	void DestroySession();
#pragma endregion

#pragma region Getters & Setters
	UFUNCTION(BlueprintPure)
	ESessionState GetSessionState() const { return CurSessionState; }
	
	UFUNCTION(BlueprintCallable)
	void SetSessionState(ESessionState NewState);
	
	FSessionInfo ExtractSessionInfo(const FOnlineSessionSearchResult& SearchResult);
#pragma endregion

	// ── 세션 검색 완료 브로드캐스트 ──
	FOnFindSessionsCompletedDelegate OnFindSessionsCompleted;
	
#pragma region Encoding/Decoding
	FString StringBase64Encode(const FString& str);
	FString StringBase64Decode(const FString& str);
#pragma endregion
	
private:
	// ── OnlineSession 콜백 ──
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
private:
#pragma region OnlineSubsystem Delegates
	FOnCreateSessionCompleteDelegate OnCreateSessionComplete;
	FOnFindSessionsCompleteDelegate OnFindSessionsComplete;
	FOnJoinSessionCompleteDelegate OnJoinSessionComplete;
	FOnDestroySessionCompleteDelegate OnDestroySessionComplete;
	
	FDelegateHandle OnCreateSessionCompleteHandle;
	FDelegateHandle OnFindSessionsCompleteHandle;
	FDelegateHandle OnJoinSessionCompleteHandle;
	FDelegateHandle OnDestroySessionCompleteHandle;
#pragma endregion
	
private:
	ESessionState CurSessionState = ESessionState::Idle;
};

