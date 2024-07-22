// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Techdarkness_Dev : ModuleRules
{
	public Techdarkness_Dev(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
