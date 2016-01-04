using UnrealBuildTool;

public class OSVR : ModuleRules
{
    public OSVR(TargetInfo Target)
    {
        PrivateIncludePaths.AddRange(
            new string[] {
				"OSVR/Private",
				// ... add other private include paths required here ...
			}
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
			{
				"OSVRClientKit",
                "Core",
				"CoreUObject",      // Provides Actors and Structs
				"Engine",           // Used by Actor
				"Slate",            // Used by InputDevice to fire bespoke FKey events
				"InputCore",        // Provides LOCTEXT and other Input features
				"InputDevice",      // Provides IInputInterface
				"RHI",
				"RenderCore",
				"Renderer",
				"ShaderCore",
				"HeadMountedDisplay",
				"Json"
				// ... add private dependencies that you statically link with here ...
			}
            );
        if(UEBuildConfiguration.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        if(Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "D3D11RHI" });
            PrivateIncludePaths.AddRange(
                new string[] {
 					        @"C:\Program Files\Epic Games\4.10\Engine\Source\Runtime\Windows\D3D11RHI\Private",
 					        @"C:\Program Files\Epic Games\4.10\Engine\Source\Runtime\Windows\D3D11RHI\Private\Windows",
                            @"D:\unreal\Epic Games\4.10\Engine\Source\Runtime\Windows\D3D11RHI\Private",
 					        @"D:\unreal\Epic Games\4.10\Engine\Source\Runtime\Windows\D3D11RHI\Private\Windows",
    				        });
        }
    }
}
