// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/UIManagerSubsystem.h"

#include "Murphy.h"
#include "Settings/UIManagerSettings.h"
#include "UI/Base/CommonPopupWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Manager/ScenarioSubsystem.h"
#include "UI/LevelEnterToastPopupWidget.h"
#include "UI/QuestToastPopupWidget.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 위젯 생성은 PlayerController가 준비된 뒤여야 하므로
    // 여기서 CreateWidget 하지 않는다. 클래스 로드도 첫 사용 시점까지 미룬다.
    
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this,
        &UUIManagerSubsystem::HandlePostLoadMapWithWorld
    );
    
    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UGameInstance* GI = LP->GetGameInstance())
        {
            UScenarioSubsystem* ScenarioSS = GI->GetSubsystem<UScenarioSubsystem>();
            if (ScenarioSS)
            {
                ScenarioSS->OnScenarioStateChanged.AddDynamic(this, &UUIManagerSubsystem::HandleScenarioStateChanged);
                ScenarioSS->OnQuestStarted.AddDynamic(this, &UUIManagerSubsystem::HandleQuestStarted);
            }
        }
    }
}

void UUIManagerSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UGameInstance* GI = LP->GetGameInstance())
        {
            UScenarioSubsystem* ScenarioSS = GI->GetSubsystem<UScenarioSubsystem>();
            if (ScenarioSS)
            {
                ScenarioSS->OnScenarioStateChanged.RemoveDynamic(this, &UUIManagerSubsystem::HandleScenarioStateChanged);
                ScenarioSS->OnQuestStarted.RemoveDynamic(this, &UUIManagerSubsystem::HandleQuestStarted);
            }
        }
    }

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

TSubclassOf<UQuestToastPopupWidget> UUIManagerSubsystem::GetQuestToastClass()
{
    if (!CachedQuestToastClass)
    {
        const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>();
        CachedQuestToastClass = Settings->QuestToastClass.LoadSynchronous();
    }
    
    return CachedQuestToastClass;
}

int32 UUIManagerSubsystem::LayerToZOrder(EUILayer Layer)
{
	return GetUILayerZOrder(Layer);
}

void UUIManagerSubsystem::HandleScenarioStateChanged(EScenarioType NewScenario)
{
    FText Title = FText::GetEmpty();
    FText Content = FText::GetEmpty();
    bool bShowToast = true;
    switch (NewScenario)
    {
        case EScenarioType::Tutorial_Airplane:
            Title   = FText::FromString(TEXT("기내 친구 사귀기"));
            Content = FText::FromString(TEXT("옆자리 승객과 대화하여 친해지세요."));
            break;

        case EScenarioType::Prologue_Immigration:
            Title   = FText::FromString(TEXT("입국 심사"));
            Content = FText::FromString(TEXT("입국 심사관과 대화하여 입국심사를 통과하세요."));
            break;

        case EScenarioType::Prologue_Baggage:
            Title   = FText::FromString(TEXT("수하물 찾기"));
            Content = FText::FromString(TEXT("수하물을 잃어버렸습니다. 직원에게 문의하여 수하물을 찾아보세요."));
            break;

        case EScenarioType::None:
            bShowToast = false;
            break;
        default:
            return;
    }

    if (bShowToast)
    {
        ShowQuestToast(Title, Content);
    }
}

void UUIManagerSubsystem::HandleQuestStarted(FName QuestID, FText QuestTitle, FText QuestDescription)
{
    // QuestTitle이 비어있으면 "돌발 미션" 폴백 텍스트 사용
    const FText DisplayTitle = QuestTitle.IsEmpty()
        ? FText::FromString(TEXT("돌발 미션"))
        : QuestTitle;

    ShowQuestToast(DisplayTitle, QuestDescription, 5);
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

void UUIManagerSubsystem::ShowQuestToast(const FText& Title, const FText& Content, float LifeTime)
{
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

    TSubclassOf<UQuestToastPopupWidget> QuestToastClass = GetQuestToastClass();
    if (!QuestToastClass)
    {
        PRINTLOG_SH(TEXT("QuestToastClass is nullptr. Check UIManagerSettings."));
        return;
    }

    UQuestToastPopupWidget* Toast = CreateWidget<UQuestToastPopupWidget>(PC, QuestToastClass);
    if (!Toast)
    {
        PRINTLOG_SH(TEXT("Failed to create QuestToastPopupWidget."));
        return;
    }

    Toast->SetUp(Title, Content, LifeTime);
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