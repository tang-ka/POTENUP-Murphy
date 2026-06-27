
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/AIDataTypes.h"
#include "Data/GameDataTypes.h"
#include "Data/PhoneDataTypes.h"
#include "Data/RandomDataTypes.h"
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
#pragma region ScenarioData
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
#pragma endregion

	// === 퀘스트 데이터 조회 ===
#pragma region QuestData
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
#pragma endregion

	// === 핸드폰 앱 데이터 조회 ===
#pragma region PhoneAppData
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
#pragma endregion

	// === 아이템 데이터 조회 ===
#pragma region ItemData
	/**
	 * Row Name으로 아이템 데이터를 반환합니다.
	 * @param RowName CSV의 행 이름 (예: I_Passport)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	// UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Item")
	FItemTableRow* GetItemData(const FName& RowName) const;

	/**
	 * 로드된 아이템 DataTable의 모든 Row Name 목록을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Item")
	TArray<FName> GetAllItemRowNames() const;
#pragma endregion

	// === 감정 데이터 조회 ===
#pragma region EmotionData
	/**
	 * Row Name으로 NPC 감정 데이터를 반환합니다.
	 * @param RowName DataTable의 행 이름 (예: Normal, Joy, Anger)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	FAI_EmotionData* GetEmotionData(const FName& RowName) const;

	/**
	 * 로드된 감정 DataTable의 모든 Row Name 목록을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Emotion")
	TArray<FName> GetAllEmotionRowNames() const;
#pragma endregion
	
	// === 입국심사 장소 데이터 조회 ===
#pragma region LocationData
	/**
	 * Row Name으로 장소 데이터를 반환합니다.
	 * @param RowName CSV의 행 이름 (예: LOC_DOWNTOWN_HOTEL)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	FLocationTextData* GetLocationData(const FName& RowName) const;

	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Location")
	TArray<FName> GetAllLocationRowNames() const;
#pragma endregion

	// === 입국심사 물품 데이터 조회 ===
#pragma region CustomsItemData
	/**
	 * Row Name으로 입국심사 의심 물품 데이터를 반환합니다.
	 * @param RowName CSV의 행 이름 (예: ITM_SUSPICIOUS_WATCH)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	FCustomsItemTextData* GetCustomsItemData(const FName& RowName) const;

	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|CustomsItem")
	TArray<FName> GetAllCustomsItemRowNames() const;
#pragma endregion
	
	// === 티어 데이터 조회 ===
#pragma region TierData
	/**
	 * Row Name으로 티어 데이터를 반환합니다.
	 * @param RowName DataTable의 행 이름 (예: Diamond, Gold)
	 * @return 해당 Row 포인터. 없으면 nullptr
	 */
	FTierUIDataRow* GetTierData(const FName& RowName) const;

	UFUNCTION(BlueprintCallable, Category = "Murphy|Data|Tier")
	TArray<FName> GetAllTierRowNames() const;
#pragma endregion
	
private:
	/** DataManagerSettings에서 DataTable을 동기 로드합니다. */
	void LoadDataTables();

	UPROPERTY()
	TObjectPtr<UDataTable> ScenarioDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> QuestDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> PhoneAppDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> ItemDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> EmotionDataTable;
	
	UPROPERTY()
	TObjectPtr<UDataTable> LocationDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> CustomsItemDataTable;
	
	UPROPERTY()
	TObjectPtr<UDataTable> TierDataTable;
};
