// Fill out your copyright notice in the Description page of Project Settings.

// UIManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/Base/CommonPopupWidget.h"   // FUIPopupDesc
#include "Data/UILayerTypes.h"             // EUILayer
#include "UIManagerSubsystem.generated.h"

class UQuestToastPopupWidget;
class ULevelEnterToastPopupWidget;
class UCommonPopupWidget;
class UUserWidget;
class UScenarioSubsystem;
enum class EScenarioType : uint8;

UCLASS()
class MURPHY_API UUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 한 줄 생성 API ──
	UCommonPopupWidget* ShowPopup(const FUIPopupDesc& InDesc);
	// LifeTime = 0.f 이면 UIManagerSettings::DefaultToastLifeTime 을 사용
	void ShowToast(const FText& Message, float LifeTime = 0.f);
	void ShowLevelEnterToast(const FText& LevelName, float LifeTime = 0.f);
	void ShowQuestToast(const FText& Title, const FText& Content, float LifeTime = 0.f);


	// ── 레이어 배치 ──
	void PushToLayer(EUILayer Layer, UUserWidget* Widget);
	void RemoveFromLayer(UUserWidget* Widget);

	// ── 페이드 (TransitionWidget 구현 후 연결, 현재는 자리만) ──
	/** @param Duration 페이드 시간(초). 0 이하이면 UIManagerSettings::DefaultFadeDuration 사용 */
	void FadeOut(float Duration, FSimpleDelegate OnComplete);
	/** @param Duration 페이드 시간(초). 0 이하이면 UIManagerSettings::DefaultFadeDuration 사용 */
	void FadeIn(float Duration, FSimpleDelegate OnComplete);

private:
	APlayerController* GetOwningController() const;

	// 위젯 클래스 로드 (지연)
	TSubclassOf<UCommonPopupWidget> GetPopupClass();
	TSubclassOf<UUserWidget> GetToastClass();
	TSubclassOf<ULevelEnterToastPopupWidget> GetLevelEnterToastClass();
	TSubclassOf<UQuestToastPopupWidget> GetQuestToastClass();

	// EUILayer → ZOrder
	static int32 LayerToZOrder(EUILayer Layer);

	UFUNCTION()
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);

	UFUNCTION()
	void HandleScenarioStateChanged(EScenarioType NewScenario);
	
private:
	UPROPERTY()
	TSubclassOf<UCommonPopupWidget> CachedPopupClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> CachedToastClass;
	
	UPROPERTY()
	TSubclassOf<ULevelEnterToastPopupWidget> CachedLevelEnterToastClass;
	
	UPROPERTY()
	TSubclassOf<UQuestToastPopupWidget> CachedQuestToastClass;
};