# OSVR-Unreal

[![Join the chat at https://gitter.im/OSVR/OSVR-Unreal](https://badges.gitter.im/OSVR/OSVR-Unreal.svg)](https://gitter.im/OSVR/OSVR-Unreal?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
> Maintained at <https://github.com/OSVR/OSVR-Unreal>
>
> For details, see <http://osvr.github.io>
>
> For support, see <http://support.osvr.com>

## Unreal Engine Plugin for OSVR

The `OSVRUnreal` folder contains a sample application using the plugin from source, for ease of development.

The plugin itself is inside `OSVRUnreal/Plugins/OSVR`.

Currently works with Unreal Engine 4.10.x. If you are still on 4.9.x, there is a ue4_9 branch, but it is recommended that you move to 4.10.x as soon as possible. Changes from master may not be merged into that branch on a regular basis except by request.

### Dependencies
You need OSVR-Core (32-bit and 64-bit) and Render Manager. Prebuilt binaries are available here:
 > http://osvr.github.io/using/

### Integrating the plugin from source with an existing project
The current recommended way to integrate the OSVR Unreal plugin with an existing project is directly from source.

 1. Clone the OSVR-Unreal source code, or download a zip from github.
 2. Run the ImportFromSDK.cmd script and specify the SDK paths as it prompts you.   
  * NOTE: It's easiest to drag the folders to the console window when it prompts you for SDK paths, as this will automatically wrap the paths in quotes as needed. If you enter them in manually, please use quotes around any paths with spaces in them.
 3. Check that the OSVR-Core and Render Manager binaries have been copied to the correct place in OSVRUnreal/Plugins/OSVR/Source/OSVRClientKit/bin, include, and lib.
 4. You do not need to build OSVR-Unreal's project. Instead, copy the OSVRUnreal/Plugins and OSVRUnreal/Source directories to your existing project's directory.
 5. Temporarily rename the Plugins folder something else, like Plugins_. This will allow you to open the project in the editor without first building the plugin. Without this step, the editor will complain about the OSVR module being missing.
 6. Open the existing project in the Unreal editor.
 7. Rename the Plugins_ folder back to Plugins.
 8. Select Generate Visual Studio Project, or Refresh Visual Studio Project, from the File menu.
  * NOTE: if your project is a pure blueprint project, you may need to add a dummy C++ game module to your project to get Unreal to generate a Visual Studio project for you. Otherwise it may complain about there not being any code to compile (having local plugins isn't enough).
 9. Open the generated or refreshed Visual Studio project and rebuild using the Development Editor build configuration.
 10. Confirm that the OSVR plugin binaries were copied correctly to YourProject/Binaries/Win64. If not, you will need to copy binaries from YourProject/Plugins/OSVR/Binaries/Win64 (or YourProject/Plugins/OSVR/Source/OSVRClientKit/bin if Binaries is not available) to YourProject/Binaries/Win64.

 > Note: If you installed Unreal to a non-standard location, or you are running a custom build of the engine, you may need to make a slight modification to /Plugins/OSVR/Source/OSVR/OSVR.Build.cs. There are some hard-coded paths to the engine at the bottom which are added to the default include directories list. Please adjust these as necessary for your custom install location.

 > Note: There is only a 64-bit installer available for RenderManager, so for now only the 64-bit Unreal targets are supported at this time.

## More documentation

More detailed documentation, including documentation for the controller/motion-controller support is included in [Documentation.md](Documentation.md).

## Demonstration video

Video showing how to integrate OSVR into an Unreal project is here: <http://youtu.be/jLCkuJb--7w>

## License

This project: Licensed under the Apache License, Version 2.0.
