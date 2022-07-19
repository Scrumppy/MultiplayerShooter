// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GEII_PortalProject : ModuleRules
{
	public GEII_PortalProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemUtils", "OnlineSubsystemNull" });
	}
	
}
