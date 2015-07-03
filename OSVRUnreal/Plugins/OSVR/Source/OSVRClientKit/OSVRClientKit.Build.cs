
using System;
using UnrealBuildTool;

public class OSVRClientKit : ModuleRules
{

    public OSVRClientKit(TargetInfo Target)
    {
        Type = ModuleType.External;

        PublicIncludePaths.Add(ModuleDirectory + "/include");

        if ((Target.Platform == UnrealTargetPlatform.Win64)
            || (Target.Platform == UnrealTargetPlatform.Win32))
        {

            string PlatformAbbrev = "Win32";
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PlatformAbbrev = "Win64";
            }


            PublicLibraryPaths.Add(String.Format("{0}/lib/{1}", ModuleDirectory, PlatformAbbrev));
            PublicAdditionalLibraries.Add("osvrClientKit.lib");
            PublicDelayLoadDLLs.AddRange(
                    new string[] {
                "osvrClientKit.dll",
                "osvrClient.dll",
                "osvrCommon.dll",
                "osvrUtil.dll"
              });


            string DllFormat = "{0}/bin/{1}/{2}";
            foreach (var dll in PublicDelayLoadDLLs)
            {
                RuntimeDependencies.Add(new RuntimeDependency(String.Format(DllFormat, ModuleDirectory, PlatformAbbrev, dll)));
            }
        }
    }
}
