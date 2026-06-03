
#include "Manager/ScenarioSubsystem.h"

#include "Murphy.h"


void UScenarioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurScenario = EScenarioType::None;
}

void UScenarioSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UScenarioSubsystem::StartScenario(EScenarioType NewScenario)
{
	if (CurScenario == NewScenario) return;
	
	CurScenario = NewScenario;
	
	FString EnumName = StaticEnum<EScenarioType>()->GetNameStringByValue((int64)CurScenario);
	PRINTLOGW_JW(TEXT("현재 시나리오 타입: %s"), *EnumName);

	OnScenarioStateChanged.Broadcast(CurScenario);
}

void UScenarioSubsystem::EndScenario(bool bSuccess)
{
	if (CurScenario == EScenarioType::None) return;
	
	EScenarioType EndScene = CurScenario;
	CurScenario = EScenarioType::None;
	
	OnScenarioEnded.Broadcast(EndScene, bSuccess);
	OnScenarioStateChanged.Broadcast(CurScenario);
}
