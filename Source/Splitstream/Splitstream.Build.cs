// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Splitstream : ModuleRules
{
	public Splitstream(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"RHI",
			"RenderCore",
            "GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });


		PublicIncludePaths.AddRange(new string[] {
			"Splitstream"
		});
		// Uncomment if you are using Slate UI
		        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "UMG",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "NetCore" // <-- needed for ENetCloseResult and related helpers
        });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
