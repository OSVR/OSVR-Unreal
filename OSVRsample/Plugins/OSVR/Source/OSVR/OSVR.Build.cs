// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class OSVR : ModuleRules
	{
		public OSVR(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"Developer/OSVR/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "CoreUObject",      // Provides Actors and Structs
                    "Engine",           // Used by Actor
                    "Slate",            // Used by InputDevice to fire bespoke FKey events
                    "InputCore",        // Provides LOCTEXT and other Input features
                    //"InputDevice",      // Provides IInputInterface
                    "RHI",
            		"RenderCore",
					"Renderer",
                    "ShaderCore",
                    "HeadMountedDisplay",
                    "Json"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);

            LoadOSVRLib(Target);
		}
        private string ModulePath
        {
            get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
        }

        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
        }

        public bool LoadOSVRLib(TargetInfo Target)
        {
            bool isLibrarySupported = false;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "OSVR", "lib");
            
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                isLibrarySupported = true;
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Win64", "osvrClientKit.lib"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                isLibrarySupported = true;
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Win32", "osvrClientKit.lib"));
            }

            if (isLibrarySupported)
            {
                // Include path
                PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "OSVR", "include"));
            }
            
            return isLibrarySupported;
        }
	}
}
