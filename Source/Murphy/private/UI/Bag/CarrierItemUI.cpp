
#include "UI/Bag/CarrierItemUI.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Framework/MurphyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/DataManager.h"

void UCarrierItemUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UCarrierItemUI::OnCloseClicked);
	}
}

void UCarrierItemUI::InitFromItemUse_Implementation(const FItemTableRow& ItemInfo)
{
	AMurphyPlayerState* PS = GetOwningPlayerState<AMurphyPlayerState>();
	if (!PS)
	{
		return;
	}

	CurItemID = FName(*PS->CurrentItemID);
	if (CurItemID.IsNone())
	{
		return;
	}
	
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UDataManager* DataManager = GI->GetSubsystem<UDataManager>())
		{
			if (FCustomsItemTextData* CustomItem = DataManager->GetCustomsItemData(CurItemID))
			{
				if (Img_RandomCustom && !CustomItem->ItemTexture.IsNull())
				{
					Img_RandomCustom->SetBrushFromTexture(CustomItem->ItemTexture.LoadSynchronous());
				}

				if (Txt_RandomCustom_K)
				{
					Txt_RandomCustom_K->SetText(FText::FromString(CustomItem->NameKR));
				}

				if (Txt_RandomCustom_E)
				{
					Txt_RandomCustom_E->SetText(FText::FromString(CustomItem->NameEN));
				}
			}
		}
	}
}

void UCarrierItemUI::OnCloseClicked()
{
	RemoveFromParent();
}
