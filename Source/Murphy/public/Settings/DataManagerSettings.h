
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DataManagerSettings.generated.h"

/**
 * DataManager가 사용하는 DataTable 에셋 경로를 에디터에서 관리하는 설정 클래스
 * Project Settings -> Game -> Murphy Data Settings 에서 편집 가능
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Murphy Data Settings"))
class MURPHY_API UDataManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 시나리오 DataTable (FScenarioTableRow 기반 CSV) */
	UPROPERTY(Config, EditAnywhere, Category = "DataTable")
	TSoftObjectPtr<UDataTable> ScenarioDataTable;

	/** 퀘스트 DataTable (FQuestTableRow 기반 CSV) */
	UPROPERTY(Config, EditAnywhere, Category = "DataTable")
	TSoftObjectPtr<UDataTable> QuestDataTable;
};
