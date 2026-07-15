// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MurphyNetSettings.generated.h"

/**
 * AI 서버 통신용 네트워크 설정.
 * POST(/respond), WebSocket(STT stream), NPC 오디오 다운로드가 모두 이 호스트를 참조한다.
 * 서버 IP 변경 시 Config/DefaultGame.ini 또는 프로젝트 세팅에서 한 곳만 수정한다.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Murphy Net Settings"))
class MURPHY_API UMurphyNetSettings : public UDeveloperSettings
{
	GENERATED_BODY()


public:
	// AI 서버 호스트 (IP 또는 도메인)
	UPROPERTY(Config, EditAnywhere, Category="Murphy|Net")
	FString ServerHost = TEXT("172.16.30.64");

	// AI 서버 포트
	UPROPERTY(Config, EditAnywhere, Category="Murphy|Net")
	int32 ServerPort = 8000;

	// http://host:port
	FString GetHttpBase() const
	{
		return FString::Printf(TEXT("http://%s:%d"), *ServerHost, ServerPort);
	}

	// ws://host:port
	FString GetWebSocketBase() const
	{
		return FString::Printf(TEXT("ws://%s:%d"), *ServerHost, ServerPort);
	}

	// 서버가 자기 기준 localhost(127.0.0.1)로 내려준 절대 URL을 실제 도달 가능한 호스트로 치환.
	// 상대경로면 호스트를 앞에 붙인다.
	FString ResolveAudioURL(const FString& InURL) const
	{
		if (!InURL.StartsWith(TEXT("http")))
		{
			return GetHttpBase() + InURL;
		}

		FString Out = InURL;
		Out = Out.Replace(*FString::Printf(TEXT("http://127.0.0.1:%d"), ServerPort), *GetHttpBase());
		Out = Out.Replace(*FString::Printf(TEXT("http://localhost:%d"), ServerPort), *GetHttpBase());
		return Out;
	}
};
