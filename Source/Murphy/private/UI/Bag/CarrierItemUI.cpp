
#include "UI/Bag/CarrierItemUI.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Framework/MurphyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/DataManager.h"

void UCarrierItemUI::InitFromItemUse_Implementation(const FItemTableRow& ItemInfo)
{
	AMurphyPlayerState* PS = GetOwningPlayerState<AMurphyPlayerState>();
	if (PS)
	{
		CurItemID = FName(PS->CurrentItemID);
	}
	
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UDataManager* DataManager = GI->GetSubsystem<UDataManager>())
		{
			if (FCustomsItemTextData* CustomItem = DataManager->GetCustomsItemData(CurItemID))
			{
				if (Img_RandomCustom && !CustomItem->ItemTexture.IsNull())
					Img_RandomCustom->SetBrushFromTexture(CustomItem->ItemTexture.LoadSynchronous());
				Txt_RandomCustom_K->SetText(FText::FromString(CustomItem->NameKR));
				Txt_RandomCustom_E->SetText(FText::FromString(CustomItem->NameEN));
			}
		}
	}
}
