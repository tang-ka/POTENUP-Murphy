
#pragma once

#include "CoreMinimal.h"
#include "Data/GameDataTypes.h"
#include "GameFramework/PlayerState.h"
#include "MurphyPlayerState.generated.h"

/**
 * 모든 레벨에서 공통으로 사용하는 PlayerState
 */
UCLASS()
class MURPHY_API AMurphyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

protected:
	UFUNCTION()
	void OnRep_SessionRoomState();
	
public:
	// AI 대화 결과 저장용 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Murphy|State")
	FString LastDialogResult;
	
	// AI Agent NPC 호감도
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Murphy|State")
	int32 NPCAffection;
	
	// 시나리오 성공 상태 저장 (필요 시 확장)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Murphy|State")
	bool bPassedCurrentScenario = false;
	
#pragma region Session room state
	// 선택한 캐릭터 (Carry)
	UPROPERTY(ReplicatedUsing = OnRep_SessionRoomState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session")
	EPlayerCharacterType  SelectedCharacter = EPlayerCharacterType::None;
	
	// 준비 완료 여부 (Room only)
	UPROPERTY(ReplicatedUsing = OnRep_SessionRoomState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session")
	bool bIsReady = false;
	
	// 호스트 여부
	UPROPERTY(ReplicatedUsing = OnRep_SessionRoomState, VisibleAnywhere, BlueprintReadOnly, Category="Murphy|Session")
	bool bIsHost = false;
	
	// 룸 상태 복제 시 위젯 갱신용 델리게이트
	FSimpleMulticastDelegate OnSessionRoomStateChanged;	
#pragma endregion 
};

