// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemMenuUI.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Manager/NetworkManagerSubsystem.h"

void USystemMenuUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_ExitGame) btn_ExitGame->OnClicked.AddDynamic(this, &USystemMenuUI::OnExitGameButtonClicked);
	if (btn_Option) btn_Option->OnClicked.AddDynamic(this, &USystemMenuUI::OnOptionButtonClicked);
	if (btn_Quit) btn_Quit->OnClicked.AddDynamic(this, &USystemMenuUI::OnQuitButtonClicked);
}

void USystemMenuUI::OnExitGameButtonClicked()
{
	APlayerController* OwningPC = GetOwningPlayer();
	if (!OwningPC)
	{
		return;
	}

	UNetworkManagerSubsystem* NetworkManager = OwningPC->GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (!NetworkManager)
	{
		return;
	}

	// 싱글플레이: 세션이 없으므로 로비로 직접 이동
	if (NetworkManager->GetSessionState() != ESessionState::InSession)
	{
		// 참고: 맵 경로가 다를 경우 알맞게 수정해 주세요.
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/Lv_Lobby")));
		return;
	}

	// 멀티플레이: 세션 종료 후 로비 복귀
	NetworkManager->DestroySession();
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
