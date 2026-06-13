
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GameDataTypes.h"
#include "Data/PhoneDataTypes.h"
#include "DataManager.generated.h"

class UDataTable;

/**
 * 게임 전역 데이터(시나리오, 퀘스트)를 관리하는 GameInstance Subsystem
 * DataManagerSettings에 지정된 DataTable을 로드하고 Row 데이터를 제공합니다.
 */
UCLASS()
class MURPHY_API UDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	
	// === 시나리오 데이터 조회 ===
	
	/**
	 * Row Name으로 시나리오 데이터를 반환합니다.
	 * @param RowName CSV의 행 이름 (예: S_Airplane)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	// UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Scenario")
	FScenarioTableRow* GetScenarioData(const FName& RowName) const;

	/**
	 * 로드된 시나리오 DataTable의 모든 Row Name 목록을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Scenario")
	TArray<FName> GetAllScenarioRowNames() const;


	// === 퀘스트 데이터 조회 ===
	
	/**
	 * Row Name으로 퀘스트 데이터를 반환합니다.
	 * @param RowName CSV의 행 이름 (예: Q_FindPassport)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	// UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Quest")
	FQuestTableRow* GetQuestData(const FName& RowName) const;

	/**
	 * 로드된 퀘스트 DataTable의 모든 Row Name 목록을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Quest")
	TArray<FName> GetAllQuestRowNames() const;


	// === 핸드폰 앱 데이터 조회 ===

	/**
	 * Row Name으로 핸드폰 앱 데이터를 반환합니다.
	 * @param RowName DataTable의 행 이름 (예: App_Search)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	FPhoneAppRow* GetPhoneAppData(const FName& RowName) const;

	/**
	 * 로드된 핸드폰 앱 DataTable의 모든 Row Name 목록을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Phone")
	TArray<FName> GetAllPhoneAppRowNames() const;

	/**
	 * 로드된 핸드폰 앱 DataTable의 모든 Row 데이터를 배열로 반환합니다.
	 */
	TArray<FPhoneAppRow*> GetAllPhoneAppRows() const;

private:
	/** DataManagerSettings에서 DataTable을 동기 로드합니다. */
	void LoadDataTables();

	UPROPERTY()
	TObjectPtr<UDataTable> ScenarioDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> QuestDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> PhoneAppDataTable;
};
