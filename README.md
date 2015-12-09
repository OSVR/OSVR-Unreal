# OSVR-Unreal
> Maintained at <https://github.com/OSVR/OSVR-Unreal>
>
> For details, see <http://osvr.github.io>
>
> For support, see <http://support.osvr.com>

## Unreal Engine Plugin for OSVR
Currently in a functional but preliminary state. Contributions and cleanups gladly accepted.

The `OSVRUnreal` folder contains a sample application using the plugin from source, for ease of development.

The plugin itself is inside `OSVRUnreal/Plugins/OSVR`.

Currently works with Unreal Engine 4.9.x.

### Dependencies
You need OSVR-Core (32-bit and 64-bit) and Render Manager. Prebuilt binaries are available here:
 > http://osvr.github.io/using/

### Integrating the plugin from with an existing project
The current recommended way to integrate the OSVR Unreal plugin with an existing project is directly from source.

 1. Clone the OSVR-Unreal source code, or download a zip from github.
 2. Run the ImportFromSDK.cmd script and specify the SDK paths as it prompts you.   
  * NOTE: this script currently has a known issue with SDK paths with spaces in them. For now, please specify SDK paths without spaces to proceed.
 3. Check that the OSVR-Core and Render Manager binaries have been copied to the correct place in OSVRUnreal/Plugins/OSVR/Source/OSVRClientKit/bin, include, and lib.
 4. You do not need to build OSVR-Unreal's project. Instead, copy the OSVRUnreal/Plugins directory to your existing project's directory.
 5. Open the existing project in the Unreal editor.
 6. Select Generate Visual Studio Project, or Refresh Visual Studio Project, from the File menu.
 7. Open the generated or refreshed Visual Studio project and rebuild.

## License

This project: Licensed under the Apache License, Version 2.0.
