# OSVR-Unreal

[![Join the chat at https://gitter.im/OSVR/OSVR-Unreal](https://badges.gitter.im/OSVR/OSVR-Unreal.svg)](https://gitter.im/OSVR/OSVR-Unreal?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
> Maintained at <https://github.com/OSVR/OSVR-Unreal>
>
> For details, see <http://osvr.github.io>
>
> For support, see <http://support.osvr.com>

## Unreal Engine Plugin for OSVR

This OSVR plugin is now integrated in the official Unreal Engine 4.12.x release. To use
it, simply enable the OSVR plugin from your project's plugin page - it is listed
in the Virtual Reality section.

The code for the plugin is maintained at <https://github.com/OSVR/OSVR-Unreal>
and source code drops are sent from here to the Unreal engine code, so community
contributions should be made there first.

The `OSVRUnreal` folder contains a sample application using the plugin from source, for ease of development.

The plugin itself is inside `OSVRUnreal/Plugins/OSVR`.

Currently works with Unreal Engine 4.12.x. If you are on 4.11.x or 4.10.x,
you may need to modify `IOSVR.h` to change `OSVR_UNREAL_4_12` to 0 (for 4.11)
and additionally `OSVR_UNREAL_4_11` to 0, if targeting unreal 4.10. However, 4.10 and 4.11
support will be limited, going forward (community contributions always welcome).

## Using the Plugin From Source
To update your game to the latest OSVR-Unreal plugin source, you need to install
the plugin as a local plugin in your project, just as was done prior to the official
release of the built-in plugin. You will need to disable the built-in plugin first,
if you previously enabled it.

### Dependencies

 1. You need OSVR SDK (32-bit and 64-bit).
  * Prebuilt binaries are available here: http://osvr.github.io/using/
  * NOTE: Installing the 64-bit version currently uninstalls the 32-bit version.  A workaround is to install the 32-bit version, copy the SDK directory to a temp dir, install the 64-bit version, then copy the temp 32-bit SDK directory back into place.
 2. You will also need the OSVR-Android binaries.
  * Prebuild binaries are available here: http://resource.osvr.com/public_download/artifacts/osvr-android-ndk/osvr-android-ndk.tar.bz2

### Integrating the plugin from source with an existing project
The current recommended way to integrate the OSVR Unreal plugin with an existing project is directly from source.

 1. Clone the OSVR-Unreal source code, or download a zip from github.
 2. Run the ImportFromSDK.cmd script and specify the SDK paths as it prompts you.   
  * NOTE: It's easiest to drag the folders to the console window when it prompts you for SDK paths, as this will automatically wrap the paths in quotes as needed. If you enter them in manually, please use quotes around any paths with spaces in them.
  * Example command line: ```ImportFromSDK.cmd "C:\Program Files (x86)\OSVR\SDK" "C:\Program Files\OSVR\SDK" D:\3rdParty\osvr-android-ndk```
 3. Check that the OSVR binaries have been copied to the correct place in OSVRUnreal/Plugins/OSVR/Source/OSVRClientKit/bin, include, and lib.
 4. You do not need to build OSVR-Unreal's project. Instead, copy the OSVRUnreal/Plugins/OSVR directory to your existing project's Plugins directory.
 5. You can either load your .uproject file and let UE4Editor automatically handle any Plugin rebuilding needed, or you can do it manually following the next steps.
 5. Select Generate Visual Studio Project, or Refresh Visual Studio Project, from the File menu of your .uproject file
  * NOTE: if your project is a pure Blueprint project, you may need to add a dummy C++ game module to your project to get Unreal to generate a Visual Studio project for you. Otherwise it may complain about there not being any code to compile (having local plugins isn't enough).
 6. Open the generated or refreshed Visual Studio project and rebuild using the Development Editor build configuration.  The file will be in the project's top-level directory.  Make sure to select Win64 as the target.
 7. Confirm that the OSVR plugin binaries were copied correctly to YourProject/Binaries/Win64 and YourProject/Binaries/Android/armeabi-v7a. If not, you will need to copy binaries from YourProject/Plugins/OSVR/Binaries (or YourProject/Plugins/OSVR/Source/OSVRClientKit/bin if Binaries is not available) to YourProject/Binaries.

 > Note: Android support is currently preliminary/alpha-quality. It is not yet ready for production. If you still want to build for Android without OSVR support enabled, remove Android from the WhiteListPlatforms of both OSVR and OSVRInput modules in /OSVRUnreal/Plugins/OSVR/OSVR.uplugin.

## More documentation

More detailed documentation, including documentation for the controller/motion-controller support is included in [Documentation.md](Documentation.md).

## Demonstration video

Video showing how to integrate OSVR into an Unreal project is here: <http://youtu.be/jLCkuJb--7w>

## License

This project: Licensed under the Apache License, Version 2.0.
