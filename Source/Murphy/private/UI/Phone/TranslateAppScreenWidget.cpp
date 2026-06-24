// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Phone/TranslateAppScreenWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Murphy.h"
#include "UI/Phone/AgentDialogBlockWidget.h"
#include "UI/Phone/ScenarioCategoryButtonWidget.h"
#include "UI/Phone/TranslateDialogManager.h"
#include "UI/Phone/UserDialogBlockWidget.h"

void UTranslateAppScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetConnecting(false);
}

void UTranslateAppScreenWidget::InitializeWithManager(UTranslateDialogManager* InDialogManager)
{
	if (DialogManager)
	{
		DialogManager->OnDialogAdded.RemoveDynamic(this, &UTranslateAppScreenWidget::HandleDialogAdded);
		DialogManager->OnCategoryRegistered.RemoveDynamic(this, &UTranslateAppScreenWidget::HandleCategoryAdded);
	}

	DialogManager = InDialogManager;

	if (!DialogManager)
	{
		PRINTLOG_SH(TEXT("Translate DialogManager is null"));
		return;
	}

	DialogManager->OnDialogAdded.AddDynamic(this, &UTranslateAppScreenWidget::HandleDialogAdded);
	DialogManager->OnCategoryRegistered.AddDynamic(this, &UTranslateAppScreenWidget::HandleCategoryAdded);

	SyncExistingCategories();
}

void UTranslateAppScreenWidget::HandleDialogAdded(FName InCategoryName, const FDialogEntry& Entry)
{
	if (InCategoryName == CurrentCategory)
	{
		RefreshDialog(CurrentCategory);
	}
}

void UTranslateAppScreenWidget::HandleCategoryAdded(FName InCategoryName, const FText& DisplayName)
{
	if (!CategoryButtonNameSet.Contains(InCategoryName))
	{
		if (AddCategoryButton(InCategoryName, DisplayName))
		{
			CategoryButtonNameSet.Add(InCategoryName);
		}
	}

	if (CurrentCategory.IsNone())
	{
		OnCategoryButtonSelected(InCategoryName);
	}
}

void UTranslateAppScreenWidget::SetConnecting(bool bIsConnecting)
{
	if (Img_Connecting)
	{
		Img_Connecting->SetColorAndOpacity(bIsConnecting ? ConnectingColor : DisconnectedColor);
	}

	if (Anim_Connecting)
	{
		if (bIsConnecting)
		{
			PlayAnimation(Anim_Connecting, 0.f, 0);
		}
		else
		{
			StopAnimation(Anim_Connecting);
		}
	}

	PRINTLOG_SH(TEXT("SetConnecting: %s"), bIsConnecting ? TEXT("true") : TEXT("false"));
}

UUserWidget* UTranslateAppScreenWidget::AcquireDialogWidget(const FDialogEntry& Entry)
{
	if (Entry.Type == EDialogType::User)
	{
		return AcquireUserDialogWidget();
	}

	return AcquireAgentDialogWidget();
}

UUserDialogBlockWidget* UTranslateAppScreenWidget::AcquireUserDialogWidget()
{
	for (UUserDialogBlockWidget* Widget : UserDialogWidgetPool)
	{
		if (Widget && !ActiveDialogWidgets.Contains(Widget))
		{
			PRINTLOG_SH(TEXT("====================Reusing(User)====================="));
			return Widget;
		}
	}

	if (!UserDialogBlockClass)
	{
		PRINTLOG_SH(TEXT("UserDialogBlockClass is not set"));
		return nullptr;
	}

	UUserDialogBlockWidget* NewWidget = CreateWidget<UUserDialogBlockWidget>(this, UserDialogBlockClass);
	if (!NewWidget)
	{
		PRINTLOG_SH(TEXT("Failed to create UserDialogBlock"));
		return nullptr;
	}

	UserDialogWidgetPool.Add(NewWidget);
	PRINTLOG_SH(TEXT("====================Create(User)====================="));
	return NewWidget;
}

UAgentDialogBlockWidget* UTranslateAppScreenWidget::AcquireAgentDialogWidget()
{
	for (UAgentDialogBlockWidget* Widget : AgentDialogWidgetPool)
	{
		if (Widget && !ActiveDialogWidgets.Contains(Widget))
		{
			PRINTLOG_SH(TEXT("====================Reusing(Agent)====================="));
			return Widget;
		}
	}

	if (!AgentDialogBlockClass)
	{
		PRINTLOG_SH(TEXT("AgentDialogBlockClass is not set"));
		return nullptr;
	}

	UAgentDialogBlockWidget* NewWidget = CreateWidget<UAgentDialogBlockWidget>(this, AgentDialogBlockClass);
	if (!NewWidget)
	{
		PRINTLOG_SH(TEXT("Failed to create AgentDialogBlock"));
		return nullptr;
	}

	AgentDialogWidgetPool.Add(NewWidget);
	PRINTLOG_SH(TEXT("====================Create(Agent)====================="));
	return NewWidget;
}

UScenarioCategoryButtonWidget* UTranslateAppScreenWidget::AddCategoryButton(FName InCategoryName, const FText& DisplayName)
{
	if (!ScenarioCategoryButtonClass)
	{
		PRINTLOG_SH(TEXT("ScenarioCategoryButtonClass is not set"));
		return nullptr;
	}

	UScenarioCategoryButtonWidget* Button = CreateWidget<UScenarioCategoryButtonWidget>(this, ScenarioCategoryButtonClass);
	if (!Button)
	{
		PRINTLOG_SH(TEXT("Failed to create ScenarioCategoryButton"));
		return nullptr;
	}

	Button->SetCategoryData(InCategoryName, DisplayName);
	Button->OnCategorySelected.AddDynamic(this, &UTranslateAppScreenWidget::OnCategoryButtonSelected);

	if (HB_ScenarioCategory)
	{
		HB_ScenarioCategory->AddChild(Button);
	}

	return Button;
}

void UTranslateAppScreenWidget::ApplyDialogData(UUserWidget* DialogWidget, const FDialogEntry& Entry)
{
	if (Entry.Type == EDialogType::User)
	{
		if (UUserDialogBlockWidget* UserWidget = Cast<UUserDialogBlockWidget>(DialogWidget))
		{
			UserWidget->SetDialogData(Entry.Time, Entry.Content);
		}

		return;
	}

	if (UAgentDialogBlockWidget* AgentWidget = Cast<UAgentDialogBlockWidget>(DialogWidget))
	{
		AgentWidget->SetDialogData(Entry.Name, Entry.Time, Entry.Content);
	}
}

void UTranslateAppScreenWidget::HideActiveDialogWidgets()
{
	for (UUserWidget* Widget : ActiveDialogWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	ActiveDialogWidgets.Reset();
}

void UTranslateAppScreenWidget::RefreshDialog(FName InCategoryName)
{
	if (!VB_Dialog)
	{
		return;
	}

	HideActiveDialogWidgets();

	if (!DialogManager)
	{
		PRINTLOG_SH(TEXT("Translate DialogManager is null"));
		return;
	}

	const TArray<FDialogEntry>* Dialogs = DialogManager->GetDialogs(InCategoryName);
	if (!Dialogs)
	{
		PRINTLOG_SH(TEXT("Dialog data not found: Category=%s"), *InCategoryName.ToString());
		return;
	}

	for (const FDialogEntry& Entry : *Dialogs)
	{
		UUserWidget* DialogWidget = AcquireDialogWidget(Entry);
		if (DialogWidget)
		{
			ApplyDialogData(DialogWidget, Entry);
			DialogWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			VB_Dialog->AddChild(DialogWidget);
			ActiveDialogWidgets.Add(DialogWidget);
		}
	}

	PRINTLOG_SH(TEXT("Dialog Refresh: Category=%s, Count=%d"), *InCategoryName.ToString(), Dialogs->Num());
}

void UTranslateAppScreenWidget::SyncExistingCategories()
{
	if (!DialogManager)
	{
		return;
	}

	for (const TPair<FName, FText>& CategoryPair : DialogManager->GetCategoryDisplayNameMap())
	{
		HandleCategoryAdded(CategoryPair.Key, CategoryPair.Value);
	}

	if (!CurrentCategory.IsNone())
	{
		RefreshDialog(CurrentCategory);
	}
}

void UTranslateAppScreenWidget::OnCategoryButtonSelected(FName InCategoryName)
{
	CurrentCategory = InCategoryName;
	RefreshDialog(InCategoryName);
	PRINTLOG_SH(TEXT("Category Changed: %s"), *InCategoryName.ToString());
}
