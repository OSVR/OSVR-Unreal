using UnrealBuildTool;
using System.IO;

public class OSVR : ModuleRules
{
    public OSVR(TargetInfo Target)
    {
        PrivateIncludePathModuleNames.Add("TargetPlatform");

        var EngineDir = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
        var openglDrvPrivatePath = Path.Combine(EngineDir, @"Source\Runtime\OpenGLDrv\Private");
        var openglPath = Path.Combine(EngineDir, @"Source\ThirdParty\OpenGL");

        PrivateIncludePaths.AddRange(
            new string[] {
				"OSVR/Private",
                openglDrvPrivatePath,
                openglPath
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
				"Json",
                "OpenGLDrv"
				// ... add private dependencies that you statically link with here ...
			}
            );
        if(UEBuildConfiguration.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");

        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "D3D11RHI" });

            // Required for some private headers needed for the rendering support.
            
            PrivateIncludePaths.AddRange(
                new string[] {
                            Path.Combine(EngineDir, @"Source\Runtime\Windows\D3D11RHI\Private"),
                            Path.Combine(EngineDir, @"Source\Runtime\Windows\D3D11RHI\Private\Windows")
    				        });
        }
    }
}
