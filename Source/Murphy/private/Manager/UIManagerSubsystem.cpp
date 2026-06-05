// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/UIManagerSubsystem.h"
#include "Settings/UIManagerSettings.h"
#include "UI/Base/CommonPopupWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UI/LevelEnterToastPopupWidget.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 위젯 생성은 PlayerController가 준비된 뒤여야 하므로
    // 여기서 CreateWidget 하지 않는다. 클래스 로드도 첫 사용 시점까지 미룬다.
    
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this,
        &UUIManagerSubsystem::HandlePostLoadMapWithWorld
    );
}

void UUIManagerSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    Super::Deinitialize();
}

APlayerController* UUIManagerSubsystem::GetOwningController() const
{
    const ULocalPlayer* LP = GetLocalPlayer();
    if (!LP)
    {
        return nullptr;
    }

    return LP->GetPlayerController(LP->GetWorld());
}

TSubclassOf<UCommonPopupWidget> UUIManagerSubsystem::GetPopupClass()
{
    if (!CachedPopupClass)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        CachedPopupClass = Settings->PopupClass.LoadSynchronous();
    }

    return CachedPopupClass;
}

TSubclassOf<UUserWidget> UUIManagerSubsystem::GetToastClass()
{
    if (!CachedToastClass)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        CachedToastClass = Settings->ToastClass.LoadSynchronous();
    }

    return CachedToastClass;
}

TSubclassOf<ULevelEnterToastPopupWidget> UUIManagerSubsystem::GetLevelEnterToastClass()
{
    if (!CachedLevelEnterToastClass)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        CachedLevelEnterToastClass = Settings->LevelEnterToastClass.LoadSynchronous();
    }

    return CachedLevelEnterToastClass;
}

int32 UUIManagerSubsystem::LayerToZOrder(EUILayer Layer)
{
    // 레이어 간 간격을 넉넉히 둬서 같은 레이어 내 미세 조정 여지를 남긴다.
    switch (Layer)
    {
        case EUILayer::Game:         return 0;
        case EUILayer::Persistent:   return 100;
        case EUILayer::Menu:         return 200;
        case EUILayer::Modal:        return 300;
        case EUILayer::Notification: return 400;
        case EUILayer::System:       return 500;
        default:                     return 0;
    }
}

void UUIManagerSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
    if (!LoadedWorld || !LoadedWorld->IsGameWorld())
    {
        return;
    }

    const FString MapName = LoadedWorld->GetMapName();
    FText ToastText;
    
    if (MapName.Contains(TEXT("Lv_Airplane")))
    {
        ToastText = FText::FromString(TEXT("비행기(기내)"));
    }
    else if (MapName.Contains(TEXT("Lv_Prologue")))
    {
        ToastText = FText::FromString(TEXT("입국심사"));
    }
    else
    {
        return;
    }
    
    LoadedWorld->GetTimerManager().SetTimerForNextTick([this, ToastText]()
    {
        ShowLevelEnterToast(ToastText);
    });
}

void UUIManagerSubsystem::PushToLayer(EUILayer Layer, UUserWidget* Widget)
{
    if (!Widget)
    {
        return;
    }

    Widget->AddToViewport(LayerToZOrder(Layer));
}

void UUIManagerSubsystem::RemoveFromLayer(UUserWidget* Widget)
{
    if (Widget)
    {
        Widget->RemoveFromParent();
    }
}

UCommonPopupWidget* UUIManagerSubsystem::ShowPopup(const FUIPopupDesc& InDesc)
{
    APlayerController* PC = GetOwningController();
    if (!PC)
    {
        return nullptr;
    }

    TSubclassOf<UCommonPopupWidget> PopupClass = GetPopupClass();
    if (!PopupClass)
    {
        return nullptr;
    }

    UCommonPopupWidget* Popup = CreateWidget<UCommonPopupWidget>(PC, PopupClass);
    if (!Popup)
    {
        return nullptr;
    }

    Popup->Setup(InDesc);
    PushToLayer(EUILayer::Modal, Popup);

    return Popup;
}

void UUIManagerSubsystem::ShowToast(const FText& Message, float LifeTime)
{
    // 0 이하이면 Settings의 기본값 사용
    if (LifeTime <= 0.f)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        LifeTime = Settings->DefaultToastLifeTime;
    }

    APlayerController* PC = GetOwningController();
    if (!PC)
    {
        return;
    }

    TSubclassOf<UUserWidget> ToastClass = GetToastClass();
    if (!ToastClass)
    {
        return;
    }

    UUserWidget* Toast = CreateWidget<UUserWidget>(PC, ToastClass);
    if (!Toast)
    {
        return;
    }

    // ToastWidget 구현 후 Setup(Message, LifeTime) 연결 예정
    PushToLayer(EUILayer::Notification, Toast);
}

void UUIManagerSubsystem::ShowLevelEnterToast(const FText& LevelName, float LifeTime)
{
    // 0 이하이면 Settings의 기본값 사용
    if (LifeTime <= 0.f)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        LifeTime = Settings->DefaultToastLifeTime;
    }

    APlayerController* PC = GetOwningController();
    if (!PC)
    {
        return;
    }

    TSubclassOf<UUserWidget> LevelEnterToastClass = GetLevelEnterToastClass();
    if (!LevelEnterToastClass)
    {
        return;
    }

    ULevelEnterToastPopupWidget* Toast = CreateWidget<ULevelEnterToastPopupWidget>(PC, LevelEnterToastClass);
    if (!Toast)
    {
        return;
    }
    
    Toast->SetUp(LevelName, LifeTime);
    PushToLayer(EUILayer::Notification, Toast);
    Toast->StartLifeTimeCountdown();
}

void UUIManagerSubsystem::FadeOut(float Duration, FSimpleDelegate OnComplete)
{
    // 0 이하이면 Settings의 기본값 사용
    if (Duration <= 0.f)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        Duration = Settings->DefaultFadeDuration;
    }

    // TransitionWidget 구현 후 연결.
    // 임시: 즉시 완료 콜백 실행 (페이드 없이 동작 흐름만 유지)
    OnComplete.ExecuteIfBound();
}

void UUIManagerSubsystem::FadeIn(float Duration, FSimpleDelegate OnComplete)
{
    // 0 이하이면 Settings의 기본값 사용
    if (Duration <= 0.f)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        Duration = Settings->DefaultFadeDuration;
    }

    OnComplete.ExecuteIfBound();
}