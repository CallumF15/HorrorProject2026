// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HorrorProject2026 : ModuleRules
{
	public HorrorProject2026(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"HorrorProject2026",
			"HorrorProject2026/Variant_Horror",
			"HorrorProject2026/Variant_Horror/UI",
			"HorrorProject2026/Variant_Shooter",
			"HorrorProject2026/Variant_Shooter/AI",
			"HorrorProject2026/Variant_Shooter/UI",
			"HorrorProject2026/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
