#include "Manager/ScenarioSubsystem.h"

#include "Framework/MurphyGameStateBase.h"
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
	if (AMurphyGameStateBase* MurphyGameState = ResolveMurphyGameState())
	{
		MurphyGameState->StartScenario(NewScenario);
		CurScenario = MurphyGameState->GetCurrentScenario();
		OnScenarioStateChanged.Broadcast(CurScenario);
		return;
	}

	if (CurScenario == NewScenario)
	{
		return;
	}

	CurScenario = NewScenario;
	PRINTLOGW_JW(TEXT("[ScenarioSubsystem] GameState 없이 fallback 시나리오 시작: %d"), static_cast<int32>(CurScenario));
	OnScenarioStateChanged.Broadcast(CurScenario);
}

void UScenarioSubsystem::EndScenario(bool bSuccess)
{
	if (AMurphyGameStateBase* MurphyGameState = ResolveMurphyGameState())
	{
		const EScenarioType EndedScenario = MurphyGameState->GetCurrentScenario();
		MurphyGameState->EndScenario(bSuccess);
		CurScenario = MurphyGameState->GetCurrentScenario();
		OnScenarioEnded.Broadcast(EndedScenario, bSuccess);
		OnScenarioStateChanged.Broadcast(CurScenario);
		return;
	}

	if (CurScenario == EScenarioType::None)
	{
		return;
	}

	const EScenarioType EndedScenario = CurScenario;
	CurScenario = EScenarioType::None;

	PRINTLOGW_JW(TEXT("[ScenarioSubsystem] GameState 없이 fallback 시나리오 종료: %d"), static_cast<int32>(EndedScenario));
	OnScenarioEnded.Broadcast(EndedScenario, bSuccess);
	OnScenarioStateChanged.Broadcast(CurScenario);
}

bool UScenarioSubsystem::IsInScenario() const
{
	if (const AMurphyGameStateBase* MurphyGameState = ResolveMurphyGameState())
	{
		return MurphyGameState->IsInScenario();
	}

	return CurScenario != EScenarioType::None;
}

EScenarioType UScenarioSubsystem::GetCurScenario() const
{
	if (const AMurphyGameStateBase* MurphyGameState = ResolveMurphyGameState())
	{
		return MurphyGameState->GetCurrentScenario();
	}

	return CurScenario;
}

AMurphyGameStateBase* UScenarioSubsystem::ResolveMurphyGameState() const
{
	UWorld* World = GetWorld();
	return World ? World->GetGameState<AMurphyGameStateBase>() : nullptr;
}
