// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ArrivalCard/ArrivalCardWidget.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Framework/MurphyPlayerState.h"
#include "Manager/DataManager.h"
#include "Data/RandomDataTypes.h"

void UArrivalCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (txt_SurnameDisplay)
	{
		txt_SurnameDisplay->SetVisibility(ESlateVisibility::Visible);
	}
	if (txt_GivennameDisplay)
	{
		txt_GivennameDisplay->SetVisibility(ESlateVisibility::Visible);
	}
	if (etxt_Surname)
	{
		etxt_Surname->OnTextChanged.AddDynamic(this, &UArrivalCardWidget::OnSurnameTextChanged);
	}
	if (etxt_Givenname)
	{
		etxt_Givenname->OnTextChanged.AddDynamic(this, &UArrivalCardWidget::OnGivennameTextChanged);
	}
	
	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UArrivalCardWidget::OnCloseClicked);
	}
	
	if (AMurphyPlayerState* PS = GetOwningPlayerState<AMurphyPlayerState>())
	{
		// PlayerState의 데이터가 바뀌면 내 UpdateUI 함수를 실행해라!
		PS->OnArrivalDataUpdated.AddUniqueDynamic(this, &UArrivalCardWidget::UpdateUI);
        
		// 창이 처음 열렸을 때 이미 값이 도착해 있을 수 있으니 수동으로 1회 갱신
		UpdateUI();
	}
}

void UArrivalCardWidget::SetReadOnlyData(const FText& InSurname, const FText& InGivenname)
{
	if (!etxt_Surname || !etxt_Givenname || !txt_SurnameDisplay || !txt_GivennameDisplay) return;
	
	// 텍스트 복사
	txt_SurnameDisplay->SetText(InSurname);
	txt_GivennameDisplay->SetText(InGivenname);
	
	etxt_Surname->SetVisibility(ESlateVisibility::Collapsed);
	etxt_Givenname->SetVisibility(ESlateVisibility::Collapsed);
	txt_SurnameDisplay->SetVisibility(ESlateVisibility::Visible);
	txt_GivennameDisplay->SetVisibility(ESlateVisibility::Visible);
}

void UArrivalCardWidget::OnCloseClicked()
{
	RemoveFromParent();
}

void UArrivalCardWidget::OnSurnameTextChanged(const FText& Text)
{
	FString InputStr = Text.ToString();
	FString FilteredStr = "";
	int32 MaxLength = 20;	// 최대 글자 수
	
	// 한 글자씩 검사 -> 영문만 통과
	for (TCHAR Char : InputStr)
	{
		if ((Char >= 'A' && Char <= 'Z') || (Char >= 'a' && Char <= 'z'))
		{
			FilteredStr.AppendChar(Char);
		}
	}
	
	// 글자 수 제한 (MaxLength 초과 시 뒷부분 잘라내기)
	if (FilteredStr.Len() > MaxLength)
	{
		FilteredStr = FilteredStr.Left(MaxLength);
	}
	
	// 필터링된 결과가 원본과 다를 때만 SetText 실행 (무한루프 방지)
	if (!InputStr.Equals(FilteredStr, ESearchCase::CaseSensitive))
	{
		etxt_Surname->SetText(FText::FromString(FilteredStr));
	}
}

void UArrivalCardWidget::OnGivennameTextChanged(const FText& Text)
{
	FString InputStr = Text.ToString();
	FString FilteredStr = "";
	int32 MaxLength = 20;	// 최대 글자 수
	
	// 한 글자씩 검사 -> 영문만 통과
	for (TCHAR Char : InputStr)
	{
		if ((Char >= 'A' && Char <= 'Z') || (Char >= 'a' && Char <= 'z'))
		{
			FilteredStr.AppendChar(Char);
		}
	}
	
	// 글자 수 제한 (MaxLength 초과 시 뒷부분 잘라내기)
	if (FilteredStr.Len() > MaxLength)
	{
		FilteredStr = FilteredStr.Left(MaxLength);
	}
	
	// 필터링된 결과가 원본과 다를 때만 SetText 실행 (무한루프 방지)
	if (!InputStr.Equals(FilteredStr, ESearchCase::CaseSensitive))
	{
		etxt_Givenname->SetText(FText::FromString(FilteredStr));
	}
}

void UArrivalCardWidget::UpdateUI()
{
	// PlayerState와 DataManager를 가져옴
	AMurphyPlayerState* PS = GetOwningPlayerState<AMurphyPlayerState>();
	UDataManager* DataManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDataManager>() : nullptr;
    
	if (!PS || !DataManager) return;

	FString LocID = PS->CurrentLocationID;
	FString ItmID = PS->CurrentItemID;

	// === 방문 장소 Row 확인 및 갱신 ===
	if (!LocID.IsEmpty() && txt_VisitLocation)
	{
		if (FLocationTextData* FoundLoc = DataManager->GetLocationData(FName(*LocID)))
		{
			FString CombinedStr = FString::Printf(TEXT("%s (%s)"), *FoundLoc->NameEN, *FoundLoc->NameKR);
			txt_VisitLocation->SetText(FText::FromString(CombinedStr));
            
			// [성공] 화면에 초록색으로 띄움
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[Row 성공] 장소: %s"), *CombinedStr));
		}
		else
		{
			// [실패] 데이터 테이블에 해당 ID(RowName)가 없을 때 빨간색으로 띄움
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("[Row 실패] 장소 ID '%s'를 데이터 테이블에서 찾을 수 없음!"), *LocID));
		}
	}

	// === 세관 신고 물품 Row 확인 및 갱신 ===
	if (!ItmID.IsEmpty() && txt_CustomsItem)
	{
		if (FCustomsItemTextData* FoundItm = DataManager->GetCustomsItemData(FName(*ItmID)))
		{
			FString CombinedStr = FString::Printf(TEXT("%s (%s)"), *FoundItm->NameEN, *FoundItm->NameKR);
			txt_CustomsItem->SetText(FText::FromString(CombinedStr));

			// [성공] 화면에 초록색으로 띄움
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[Row 성공] 물품: %s"), *CombinedStr));
		}
		else
		{
			// [실패] 데이터 테이블에 해당 ID(RowName)가 없을 때 빨간색으로 띄움
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("[Row 실패] 물품 ID '%s'를 데이터 테이블에서 찾을 수 없음!"), *ItmID));
		}
	}
}

void UArrivalCardWidget::InitFromItemUse_Implementation(const FItemTableRow& ItemInfo)
{
	AMurphyPlayerState* PS = Cast<AMurphyPlayerState>(GetOwningPlayerState());
	if (PS)
	{
		SetReadOnlyData(FText::FromString(PS->SavedSurname), FText::FromString(PS->SavedGivenname));
		UpdateUI();
	}
}
