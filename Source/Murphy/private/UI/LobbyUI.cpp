// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LobbyUI.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Manager/LevelStreamingSubsystem.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_SinglePlay) btn_SinglePlay->OnClicked.AddDynamic(this, &ULobbyUI::OnSinglePlayButtonClicked);
	if (btn_MultiPlay) btn_MultiPlay->OnClicked.AddDynamic(this, &ULobbyUI::OnMultiPlayButtonClicked);
	if (btn_Achievement) btn_Achievement->OnClicked.AddDynamic(this, &ULobbyUI::OnAchievementButtonClicked);
	if (btn_Option) btn_Option->OnClicked.AddDynamic(this, &ULobbyUI::OnOptionButtonClicked);
	if (btn_Quit) btn_Quit->OnClicked.AddDynamic(this, &ULobbyUI::OnQuitButtonClicked);
}

void ULobbyUI::OnSinglePlayButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULevelStreamingSubsystem* LevelSubsystem = GI->GetSubsystem<ULevelStreamingSubsystem>())
		{
			LevelSubsystem->TravelAllPlayers(FName("Airplane"));
		}
	}
}

void ULobbyUI::OnMultiPlayButtonClicked()
{
}

void ULobbyUI::OnAchievementButtonClicked()
{
}

void ULobbyUI::OnOptionButtonClicked()
{
}

void ULobbyUI::OnQuitButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
}
