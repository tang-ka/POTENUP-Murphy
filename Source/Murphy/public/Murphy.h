// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Settings/LogSettings.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(JW, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(HJ, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(SH, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(CW, Warning, All);

// === 전체 로그 킬스위치 (1: 전부 끔) ===
#define DISABLE_ALL_LOGS 0
// ====================================

#if !UE_BUILD_SHIPPING && !DISABLE_ALL_LOGS
#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

#define PRINTLOG_JW(format, ...) \
if (GetDefault<ULogSettings>()->bEnableLog_JW) \
{ UE_LOG(JW, Log, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__)); }

#define PRINTLOGW_JW(format, ...) \
if (GetDefault<ULogSettings>()->bEnableLog_JW) \
{ UE_LOG(JW, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__)); }

#define PRINTLOGE_JW(format, ...) \
if (GetDefault<ULogSettings>()->bEnableLog_JW) \
{ UE_LOG(JW, Error, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__)); }

#define PRINTLOG_HJ(format, ...) \
if (GetDefault<ULogSettings>()->bEnableLog_HJ) \
{ UE_LOG(HJ, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__)); }

#define PRINTLOG_SH(format, ...) \
if (GetDefault<ULogSettings>()->bEnableLog_SH) \
{ UE_LOG(SH, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__)); }

#define PRINTLOG_CW(format, ...) \
if (GetDefault<ULogSettings>()->bEnableLog_CW) \
{ UE_LOG(CW, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__)); }

#else
#define PRINTLOG_JW(format, ...)
#define PRINTLOG_HJ(format, ...)
#define PRINTLOG_SH(format, ...)
#define PRINTLOG_CW(format, ...)
#endif

// --- 개별 로그 활성화 설정 (1: 켬, 0: 끔) ---
// #define USE_LOG_JW 0
// #define USE_LOG_HJ 0
// #define USE_LOG_SH 0
// #define USE_LOG_CW 1
// ------------------------------------------
// Shipping 빌드에서는 무조건 비활성화, 그 외 빌드에서는 개별 설정 참조
// #if !UE_BUILD_SHIPPING
// 	#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
//
// 	// JW 로그 제어
// 	#if USE_LOG_JW
// 		#define PRINTLOG_JW(format, ...) UE_LOG(JW, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__))
// 	#else
// 		#define PRINTLOG_JW(format, ...)
// 	#endif
//
// 	// HJ 로그 제어
// 	#if USE_LOG_HJ
// 		#define PRINTLOG_HJ(format, ...) UE_LOG(HJ, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__))
// 	#else
// 		#define PRINTLOG_HJ(format, ...)
// 	#endif
//
// 	// SH 로그 제어
// 	#if USE_LOG_SH
// 		#define PRINTLOG_SH(format, ...) UE_LOG(SH, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__))
// 	#else
// 		#define PRINTLOG_SH(format, ...)
// 	#endif
//
// 	// CW 로그 제어
// 	#if USE_LOG_CW
// 		#define PRINTLOG_CW(format, ...) UE_LOG(CW, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(format, ##__VA_ARGS__))
// 	#else
// 		#define PRINTLOG_CW(format, ...)
// 	#endif
//
// #else
// 	// Shipping 빌드 전체 비활성화
// 	#define PRINTLOG_JW(format, ...)
// 	#define PRINTLOG_HJ(format, ...)
// 	#define PRINTLOG_SH(format, ...)
// 	#define PRINTLOG_CW(format, ...)
// #endif