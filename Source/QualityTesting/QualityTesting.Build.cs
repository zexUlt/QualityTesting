// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QualityTesting : ModuleRules
{
	public QualityTesting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		
		PublicIncludePaths.AddRange(new string[] { "QualityTesting/Public" });
	}
}
