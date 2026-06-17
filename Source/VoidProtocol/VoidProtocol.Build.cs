// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoidProtocol : ModuleRules
{
	public VoidProtocol(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
            "OnlineSubsystem",     
			"OnlineSubsystemUtils",
			"OnlineSubsystemNull",
            "Sockets", 
			"Networking"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"VoidProtocol",
			"VoidProtocol/Variant_Platforming",
			"VoidProtocol/Variant_Platforming/Animation",
			"VoidProtocol/Variant_Combat",
			"VoidProtocol/Variant_Combat/AI",
			"VoidProtocol/Variant_Combat/Animation",
			"VoidProtocol/Variant_Combat/Gameplay",
			"VoidProtocol/Variant_Combat/Interfaces",
			"VoidProtocol/Variant_Combat/UI",
			"VoidProtocol/Variant_SideScrolling",
			"VoidProtocol/Variant_SideScrolling/AI",
			"VoidProtocol/Variant_SideScrolling/Gameplay",
			"VoidProtocol/Variant_SideScrolling/Interfaces",
			"VoidProtocol/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
