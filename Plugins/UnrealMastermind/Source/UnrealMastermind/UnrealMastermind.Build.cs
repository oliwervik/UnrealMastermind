// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealMastermind : ModuleRules
{
	public UnrealMastermind(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"EditorStyle",
				"AssetRegistry",
				"PropertyEditor",
				"UMG",
				"Projects",
				"DeveloperSettings",
				"HTTP",
				"Json",
				"JsonUtilities",
				"Kismet",
				"BlueprintGraph",
				"ToolMenus",
				"KismetWidgets",
				"GraphEditor",
			}
		);
	}
}