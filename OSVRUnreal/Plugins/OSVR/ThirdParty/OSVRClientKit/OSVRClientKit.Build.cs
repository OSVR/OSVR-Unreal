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

        PublicIncludePaths.Add(Path.Combine(ModulePath, "include"));

        if ((Target.Platform == UnrealTargetPlatform.Win64)
            || (Target.Platform == UnrealTargetPlatform.Win32))
        {

            string LibraryPath = Path.Combine(ModulePath, "/lib");

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                LibraryPath += "/Win64";
            }
            else if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                LibraryPath += "/Win32";
            }

            PublicLibraryPaths.Add(LibraryPath);
            PublicAdditionalLibraries.Add("osvrClientKit.lib");
        }
    }
}
