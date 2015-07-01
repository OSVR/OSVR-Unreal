using UnrealBuildTool;

using System.IO;

public class OSVRClientKit : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
    }

    public OSVRClientKit(TargetInfo Target)
    {
        Type = ModuleType.External;

        PublicIncludePaths.Add(ModulePath + "/include");

        if ((Target.Platform == UnrealTargetPlatform.Win64)
            || (Target.Platform == UnrealTargetPlatform.Win32))
        {

            string LibraryPath = ModulePath + "/lib";
            string DllPath = ModulePath + "/bin";

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                LibraryPath += "/Win64";
                DllPath += "/Win64";
            }
            else if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                LibraryPath += "/Win32";
                DllPath += "/Win32";
            }

            PublicLibraryPaths.Add(LibraryPath);
            PublicAdditionalLibraries.Add("osvrClientKit.lib");
            PublicDelayLoadDLLs.AddRange(
        			new string[] {
                "osvrClientKit.dll",
                "osvrClient.dll",
                "osvrCommon.dll",
                "osvrUtil.dll"
              });

            DllPath += "/";

            foreach (var dll in PublicDelayLoadDLLs)
            {
                RuntimeDependencies.Add(new RuntimeDependency(DllPath + dll));
            }
        }
    }
}
