#include "SheetSyncer.h"

#include "SheetSyncerSettings.h"
#include "SheetSyncerSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "FSheetSyncerModule"

void FSheetSyncerModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule =
	FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		USheetSyncerSettings::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FSheetSyncerSettingsCustomization::MakeInstance));
}

void FSheetSyncerModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule =
			FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(
			USheetSyncerSettings::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSheetSyncerModule, SheetSyncer)