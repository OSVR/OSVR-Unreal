
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
            PublicAdditionalLibraries.Add("osvrRenderManager.lib");

            var osvrDlls = new string[] {
                "osvrClientKit.dll",
                "osvrClient.dll",
                "osvrCommon.dll",
                "osvrUtil.dll",
                "osvrRenderManager.dll",
                "d3dcompiler_47.dll",
                "glew32.dll",
                "SDL2.dll"
              };

            PublicDelayLoadDLLs.AddRange(osvrDlls);

            // There are convenient methods to get the name of the current game,
            // as well as the binary folder. But apparently not if you're a plugin?
            // @todo see if there is a more robust way to accomplish getting the "binaries" folder
            // from a plugin build file.
            // @todo: can we do multiple copies from the same source file? If so,
            // include the RuntimeDependencies code below inside this loop and copy the binaries
            // into each game folder.
            var gameFolders = UEBuildTarget.DiscoverAllGameFolders();
            string gameName = "";
            if (gameFolders.Count > 0)
            {
                string gameFolder = gameFolders[0];
                var lastSlash = gameFolder.LastIndexOf('\\');
                gameName = String.Format("/{0}", System.IO.Path.GetFileName(gameFolder));
            }
            string DllFormat = "{0}/bin/{1}/{2}";
            foreach (var dll in osvrDlls)
            {
                var src = String.Format(DllFormat, ModuleDirectory, PlatformAbbrev, dll);
                var dst = String.Format("{0}/Binaries/{1}/{2}", gameName, PlatformAbbrev, dll);
                RuntimeDependencies.Add(new RuntimeDependency(src, dst));
            }
        }
    }
}
