
using System;
using System.IO;
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

            string baseBinaryDirectory = ModuleDirectory + "/bin";
            if (!System.IO.Directory.Exists(baseBinaryDirectory))
            {
                baseBinaryDirectory = "$(EngineDir)/Binaries/ThirdParty/OSVRClientKit/bin";
            }

            string DllFormat = "{0}/{1}/{2}";
            foreach (var dll in osvrDlls)
            {
                var src = String.Format(DllFormat, baseBinaryDirectory, PlatformAbbrev, dll);
                RuntimeDependencies.Add(new RuntimeDependency(src));
            }
        }
        else if(Target.Platform == UnrealTargetPlatform.Android)
        {
            string PlatformAbbrev = "armeabi-v7a";

            PublicLibraryPaths.Add(String.Format("{0}/bin/Android/{1}", ModuleDirectory, PlatformAbbrev));
            PublicAdditionalLibraries.Add("osvrClientKit");

            var basePath = Utils.MakePathRelativeTo(ModuleDirectory, BuildConfiguration.RelativeEnginePath);
            var xmlPath = Path.Combine(ModuleDirectory, "OSVR_APL.xml");
            //System.Console.WriteLine("xmlPath: {0}", xmlPath);
            AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", xmlPath));
        }
        else if(Target.Platform == UnrealTargetPlatform.Linux)
        {
            string PlatformAbbrev = "Linux";

            PublicLibraryPaths.Add(String.Format("{0}/bin/{1}", ModuleDirectory, PlatformAbbrev));
            PublicAdditionalLibraries.Add("osvrClientKit");
            PublicAdditionalLibraries.Add("osvrRenderManager");

            var osvrDlls = new string[] {
                "libosvrClientKit.so",
                "libosvrClient.so",
                "libosvrCommon.so",
                "libosvrUtil.so",
                "libosvrRenderManager.so",
                "libjsoncpp.so",
                "libGLEW.so",
                "libSDL2-2.0.so"
              };

            PublicDelayLoadDLLs.AddRange(osvrDlls);

            string baseBinaryDirectory = ModuleDirectory + "/bin";
            if (!System.IO.Directory.Exists(baseBinaryDirectory))
            {
                baseBinaryDirectory = "$(EngineDir)/Binaries/ThirdParty/OSVRClientKit/bin";
            }

            string DllFormat = "{0}/{1}/{2}";
            foreach (var dll in osvrDlls)
            {
                var src = String.Format(DllFormat, baseBinaryDirectory, PlatformAbbrev, dll);
                RuntimeDependencies.Add(new RuntimeDependency(src));
            }
            
        }
    }
}
