

#include "Actors/Items/ItemBaseActor.h"
#include "Actors/Characters/MurphyPlayer.h"

#include "Components/BoxComponent.h"
#include "Components/QuestEventNotifyComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/HUD/BagPopupWidget.h"
#include "UI/HUD/MainHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/DataManager.h"
#include "Murphy.h"

AItemBaseActor::AItemBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetRootComponent(InteractionBox);
	
	InteractMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractMesh"));
	InteractMesh->SetupAttachment(InteractionBox);

	// 힌트 UI 컴포넌트 생성 - 기본 숨김, 에디터에서 WBP 지정
	InteractUIWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractUIWidget"));
	InteractUIWidget->SetupAttachment(RootComponent);
	InteractUIWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractUIWidget->SetVisibility(false);

	QuestEventNotifier = CreateDefaultSubobject<UQuestEventNotifyComponent>(TEXT("QuestEventNotifier"));
}

void AItemBaseActor::BeginPlay()
{
	Super::BeginPlay();

	SyncQuestEventTarget();
	
	if (InteractUIWidget && InteractUIClass)
	{
		InteractUIWidget->SetWidgetClass(InteractUIClass);
	}
	
	// Overlap 이벤트 바인딩 - 힌트 UI 표시/숨김 제어
	if (InteractionBox)
	{
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AItemBaseActor::OnInteractionBoxBeginOverlap);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AItemBaseActor::OnInteractionBoxEndOverlap);
	}
}

void AItemBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemBaseActor::ConfigureQuestItem(FName InItemID, FName InQuestTargetID)
{
	ItemID = InItemID;

	if (QuestEventNotifier)
	{
		QuestEventNotifier->SetQuestTargetID(InQuestTargetID.IsNone() ? ItemID : InQuestTargetID);
	}
}

void AItemBaseActor::Interact(AMurphyPlayer* Player)
{
	// 1. 가방에 아이템 추가 (DataManager 연동)
	if (GetItem(Player) == false)
	{
		return;
	}

	// 2. 서버에 GetItem 퀘스트 완료 통보
	SyncQuestEventTarget();
	if (QuestEventNotifier && !ResolveQuestTargetID().IsNone())
	{
		QuestEventNotifier->NotifyQuestComplete(Player, EQuestCondition::GetItem);
	}

	// 3. 아이템 획득 처리 - 월드에서 숨김
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

bool AItemBaseActor::GetItem(AMurphyPlayer* Player)
{
	if (!IsValid(Player))
	{
		return false;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UDataManager* DataManager = GI->GetSubsystem<UDataManager>())
		{
			if (FItemTableRow* ItemInfo = DataManager->GetItemData(ItemID))
			{
				if (UMainHUD* MainHUD = Player->GetMainHUD())
				{
					if (UBagPopupWidget* BagWidget = MainHUD->GetBagPopupWidget())
					{
						BagWidget->AddItem(*ItemInfo);
					}
				}
			}
			else
			{
				PRINTLOGW_JW(TEXT("[ItemBaseActor] DataManager에서 아이템 정보를 찾을 수 없습니다: %s"), *ItemID.ToString());
				return false;
			}
		}
	}

	return true;
}

bool AItemBaseActor::IsInPlayerSight(const AMurphyPlayer* Player) const
{
	if (!IsValid(Player))
	{
		return false;
	}

	// 플레이어 전방 벡터
	const FVector PlayerForward = Player->GetActorForwardVector();
	// 플레이어 → 아이템 방향 벡터
	const FVector ToItem = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();

	// Dot Product로 각도 계산
	const float DotResult = FVector::DotProduct(PlayerForward, ToItem);
	const float ThresholdCos = FMath::Cos(FMath::DegreesToRadians(InteractAngleDeg));

	return DotResult >= ThresholdCos;
}

void AItemBaseActor::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMurphyPlayer* Player = Cast<AMurphyPlayer>(OtherActor);
	// 로컬 플레이어에게만 Interaction UI 표시 (멀티플레이 안전)
	if (IsValid(Player) && Player->IsLocallyControlled())
	{
		if (InteractUIWidget)
		{
			InteractUIWidget->SetVisibility(true);
			PRINTLOG_JW(TEXT("뭐니?"));
		}
	}
}

void AItemBaseActor::OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMurphyPlayer* Player = Cast<AMurphyPlayer>(OtherActor);
	// 로컬 플레이어가 벗어날 때만 Interaction UI 숨김
	if (IsValid(Player) && Player->IsLocallyControlled())
	{
		if (InteractUIWidget)
		{
			InteractUIWidget->SetVisibility(false);
		}
	}
}

FName AItemBaseActor::ResolveQuestTargetID() const
{
	if (QuestEventNotifier && !QuestEventNotifier->GetQuestTargetID().IsNone())
	{
		return QuestEventNotifier->GetQuestTargetID();
	}

	return ItemID;
}

void AItemBaseActor::SyncQuestEventTarget()
{
	if (QuestEventNotifier)
	{
		QuestEventNotifier->SetQuestTargetID(ResolveQuestTargetID());
	}
}
