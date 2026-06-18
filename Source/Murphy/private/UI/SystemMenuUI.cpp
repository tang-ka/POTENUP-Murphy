// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemMenuUI.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void USystemMenuUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_ExitGame) btn_ExitGame->OnClicked.AddDynamic(this, &USystemMenuUI::OnExitGameButtonClicked);
	if (btn_ExitGame) btn_Option->OnClicked.AddDynamic(this, &USystemMenuUI::OnOptionButtonClicked);
	if (btn_ExitGame) btn_Quit->OnClicked.AddDynamic(this, &USystemMenuUI::OnQuitButtonClicked);
}

void USystemMenuUI::OnExitGameButtonClicked()
{
	
}

void USystemMenuUI::OnOptionButtonClicked()
{
}

void USystemMenuUI::OnQuitButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
}
