
#pragma once

#include "CoreMinimal.h"
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
	// AI 대화 결과 저장용 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Murphy|State")
	FString LastDialogResult;
	
	// AI Agent NPC 호감도
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Murphy|State")
	int32 NPCAffection;
	
	// 시나리오 성공 상태 저장 (필요 시 확장)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Murphy|State")
	bool bPassedCurrentScenario = false;
	
public:
	// 입국심사서에 입력한 이름 저장
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Murphy|CardData")
	FString SavedSurname;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Murphy|CardData")
	FString SavedGivenname;
	
	// 클라->서버 저장 요청
	UFUNCTION(Server, Reliable)
	void ServerSetArrivalData(const FString& InSurname, const FString& InGivenname);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;;
	
};

