// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PhonePopupWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Manager/DataManager.h"
#include "Murphy.h"
#include "UI/Phone/ApplicationWidget.h"

void UPhonePopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Overlay_Phone)
	{
		FWidgetTransform PhonePopupTransform = Overlay_Phone->GetRenderTransform();
		PhonePopupTransform.Translation.Y = 560.0f;
		Overlay_Phone->SetRenderTransform(PhonePopupTransform);
	}

	// AppScreenSwitcher는 시작 시 숨김
	if (AppScreenSwitcher)
	{
		AppScreenSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Btn_Call: DataManager에서 Call AppScreen 생성 후 바인딩
	if (Btn_Call)
	{
		InitCallWidget();
		Btn_Call->OnClicked.AddDynamic(this, &UPhonePopupWidget::HandleCallClicked);
	}

	// Home 버튼은 AppScreen이 켜질 때만 표시
	if (Btn_Home)
	{
		Btn_Home->SetVisibility(ESlateVisibility::Collapsed);
		Btn_Home->OnClicked.AddDynamic(this, &UPhonePopupWidget::HandleHomeClicked);
	}

	// 각 앱 위젯 초기화 (Row Name은 DataTable에서 사용한 이름과 일치해야 합니다)
	InitAppWidget(WBP_Travelgram,  	FName("Travelgram"));
	InitAppWidget(WBP_Translate,	FName("Translate"));
	InitAppWidget(WBP_Camera,      	FName("Camera"));
	InitAppWidget(WBP_Photos,      	FName("Photos"));
	SystemColorChanged(true);

	UpdateTime();
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Clock,
		this,
		&UPhonePopupWidget::UpdateTime,
		60.0f,
		true
	);

	PRINTLOG_SH(TEXT("PhonePopupWidget 초기화 완료"));
}

void UPhonePopupWidget::InitAppWidget(UApplicationWidget* Widget, const FName& RowName)
{
	if (!Widget)
	{
		PRINTLOG_SH(TEXT("[%s] ApplicationWidget이 null입니다."), *RowName.ToString());
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UDataManager* DM = GI ? GI->GetSubsystem<UDataManager>() : nullptr;
	if (!DM)
	{
		PRINTLOG_SH(TEXT("[%s] DataManager를 찾을 수 없습니다."), *RowName.ToString());
		return;
	}

	FPhoneAppRow* Row = DM->GetPhoneAppData(RowName);
	if (!Row)
	{
		PRINTLOG_SH(TEXT("[%s] DataTable에서 Row를 찾을 수 없습니다. Row Name을 확인하세요."), *RowName.ToString());
		return;
	}

	// AppScreenWidgetClass가 있으면 인스턴스 생성
	UUserWidget* AppScreen = nullptr;
	if (!Row->AppScreenWidgetClass.IsNull())
	{
		if (TSubclassOf<UUserWidget> WidgetClass = Row->AppScreenWidgetClass.LoadSynchronous())
		{
			AppScreen = CreateWidget<UUserWidget>(GetOwningPlayer(), WidgetClass);
			if (AppScreen)
			{
				PRINTLOG_SH(TEXT("[%s] AppScreen 위젯 생성 성공: %s"), *RowName.ToString(), *WidgetClass->GetName());
			}
			else
			{
				PRINTLOG_SH(TEXT("[%s] AppScreen 위젯 생성 실패"), *RowName.ToString());
			}
		}
		else
		{
			PRINTLOG_SH(TEXT("[%s] AppScreenWidgetClass 로드 실패"), *RowName.ToString());
		}
	}
	else
	{
		PRINTLOG_SH(TEXT("[%s] AppScreenWidgetClass가 비어있습니다. 가짜 앱으로 처리됩니다."), *RowName.ToString());
	}

	// AppName / AppIcon / AppScreen 할당
	Widget->SetAppData(*Row, AppScreen);

	// AppScreenSwitcher에 AppScreen 등록
	if (AppScreenSwitcher && AppScreen)
	{
		AppScreenSwitcher->AddChild(AppScreen);
		PRINTLOG_SH(TEXT("[%s] AppScreen을 AppScreenSwitcher에 등록했습니다."), *RowName.ToString());
	}

	// 아이콘 클릭 델리게이트 연결
	Widget->OnAppIconClicked.AddDynamic(this, &UPhonePopupWidget::HandleAppIconClicked);
}

void UPhonePopupWidget::InitCallWidget()
{
	UGameInstance* GI = GetGameInstance();
	UDataManager* DM = GI ? GI->GetSubsystem<UDataManager>() : nullptr;
	if (!DM)
	{
		PRINTLOG_SH(TEXT("[Call] DataManager를 찾을 수 없습니다."));
		return;
	}

	FPhoneAppRow* Row = DM->GetPhoneAppData(FName("Call"));
	if (!Row)
	{
		PRINTLOG_SH(TEXT("[Call] DataTable에서 Row를 찾을 수 없습니다. Row Name을 확인하세요."));
		return;
	}

	if (Row->AppScreenWidgetClass.IsNull())
	{
		PRINTLOG_SH(TEXT("[Call] AppScreenWidgetClass가 비어있습니다."));
		return;
	}

	TSubclassOf<UUserWidget> WidgetClass = Row->AppScreenWidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		PRINTLOG_SH(TEXT("[Call] AppScreenWidgetClass 로드 실패"));
		return;
	}

	CallAppScreen = CreateWidget<UUserWidget>(GetOwningPlayer(), WidgetClass);
	if (CallAppScreen && AppScreenSwitcher)
	{
		AppScreenSwitcher->AddChild(CallAppScreen);
		PRINTLOG_SH(TEXT("[Call] AppScreen을 AppScreenSwitcher에 등록했습니다."));
	}
}

void UPhonePopupWidget::HandleAppIconClicked(UApplicationWidget* ClickedApp)
{
	if (!ClickedApp || !ClickedApp->AppScreen)
	{
		PRINTLOG_SH(TEXT("앱 아이콘 클릭 처리 실패 — AppScreen이 유효하지 않습니다."));
		return;
	}

	PRINTLOG_SH(TEXT("앱 화면 전환: %s"), *ClickedApp->GetName());
	ShowAppScreen(ClickedApp->AppScreen);

	const bool bIsTranslateApp = ClickedApp->GetAppName() == FName("Translate");
	SystemColorChanged(!bIsTranslateApp);
}

void UPhonePopupWidget::HandleCallClicked()
{
	if (!CallAppScreen)
	{
		PRINTLOG_SH(TEXT("[Call] CallAppScreen이 유효하지 않습니다."));
		return;
	}

	PRINTLOG_SH(TEXT("[Call] 전화 앱 화면 전환"));
	ShowAppScreen(CallAppScreen);
	SystemColorChanged(true);
}

void UPhonePopupWidget::ShowAppScreen(UUserWidget* TargetScreen)
{
	if (!AppScreenSwitcher || !TargetScreen)
	{
		return;
	}

	AppScreenSwitcher->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	AppScreenSwitcher->SetActiveWidget(TargetScreen);

	if (Btn_Home)
	{
		Btn_Home->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPhonePopupWidget::SystemColorChanged(bool bIsLight)
{
	const FLinearColor SystemColor = bIsLight ? FLinearColor::White : FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

	if (Txt_Time)
	{
		Txt_Time->SetColorAndOpacity(FSlateColor(SystemColor));
	}

	if (Img_Receive)
	{
		Img_Receive->SetColorAndOpacity(SystemColor);
	}

	if (Img_Wifi)
	{
		Img_Wifi->SetColorAndOpacity(SystemColor);
	}

	if (Img_Battery)
	{
		Img_Battery->SetColorAndOpacity(SystemColor);
	}

	if (Btn_Home)
	{
		Btn_Home->SetBackgroundColor(SystemColor);
	}
}

void UPhonePopupWidget::HandleHomeClicked()
{
	if (AppScreenSwitcher)
	{
		AppScreenSwitcher->SetVisibility(ESlateVisibility::Collapsed);
		PRINTLOG_SH(TEXT("Home 버튼 — AppScreenSwitcher 닫힘"));
	}

	// AppScreen이 꺼지면 Home 버튼 숨김
	if (Btn_Home)
	{
		Btn_Home->SetVisibility(ESlateVisibility::Collapsed);
	}

	SystemColorChanged(true);
}

void UPhonePopupWidget::UpdateTime()
{
	FDateTime Now = FDateTime::Now();
	const FString TimeStr = FString::Printf(TEXT("%02d:%02d"), Now.GetHour(), Now.GetMinute());

	if (Txt_Time)
	{
		Txt_Time->SetText(FText::FromString(TimeStr));
	}
}

void UPhonePopupWidget::TogglePhone()
{
	if (!Anim_PhoneSlideUp)
	{
		return;
	}
	
	if (!bIsOpen)
	{
		// 닫힌 상태 → 열기
		PlayAnimation(Anim_PhoneSlideUp, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		bIsOpen = true;
	}
	else
	{
		// 열린 상태 → 닫기
		PlayAnimation(Anim_PhoneSlideUp, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.0f);
		bIsOpen = false;
	}

	// 상태 변경 후 델리게이트 브로드캐스트
	OnPhoneToggled.Broadcast(bIsOpen);
}
