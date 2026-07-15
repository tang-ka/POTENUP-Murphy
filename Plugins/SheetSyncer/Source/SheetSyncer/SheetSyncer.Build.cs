using UnrealBuildTool;

public class SheetSyncer : ModuleRules
{
    public SheetSyncer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        if (Target.Type != TargetRules.TargetType.Editor)
        {
            throw new System.Exception("SheetSyncer is Editor only.");
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "DeveloperSettings",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "HTTP",
                "Json",
                "DataTableEditor",
                "Slate",
                "SlateCore",
                "PropertyEditor"
            }
        );
    }
}