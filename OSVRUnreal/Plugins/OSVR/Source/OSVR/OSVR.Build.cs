using UnrealBuildTool;

public class OSVR : ModuleRules
{
    public OSVR(TargetInfo Target)
    {
        PrivateIncludePaths.AddRange(
            new string[] {
				"OSVR/Private"
				// ... add other private include paths required here ...
			}
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
			{
				"OSVRClientKit",
                "OSVRRenderManager",
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
    }
}
