# OSVR - Unreal Engine 4 plugin

## Installation from Source

============================================================
Follow these steps to integrate OSVR into your UE4 project. 
============================================================

Before you can do anything with the plugin you'll first have to obtain the necessary files for it to work. Fortunately all of the files can be downloaded locally to your computer and some of them you should
already have before you even got to this point. When you run 'ImportFromSDK' command script its going to ask you for the locations of a few directories to specific SDKs. The one thing that needs to be remembered
though is none of the directories can have spaces. There is a small bug right now that if the paths do contain spaces then the automated script won't know what to do and fail. Failing is bad so make sure none of 
the directory paths have spaces.

Here is what you'll need to continue with the import:

1) Depending on your system setup you would have either downloaded the x32 or x64 version of the OSVR SDK (OSVR-Core). You will need both versions so visit the Developer Portal (http://osvr.github.io/using/)
and obtain the newest version of the one you don't have (make sure they are both up-to-date though!). From there, make sure to open the root folder and run the 'Add_SDK_To_Registry' script otherwise the 
'Import from SDK' script will not be able to find the SDKs even if you type in the correct paths.

After obtaining both the x32 and x64 versions of the OSVR-Core SDK and moving them to where ever you wanted, run the 'ImportFromSDK' script in the plugin folder and you will now be able to finsh the first two paths
the script asks for. Look below for an example of what you will see in the command window:

EXAMPLE: (These were the actual paths for my personal setup, your's maybe different. I kept all original folder names to avoid confusion and the failure that could happen if there are spaces in the path names.)
Type OSVR 32bit SDK root dir:H:\OSVR\OSVR-Core-Snapshot-v0.6-428-gf10d9cb-build202-vs12-32bit
Type OSVR 64bit SDK root dir:H:\OSVR\OSVR-Core-Snapshot-v0.6-428-gf10d9cb-build202-vs12-64bit


2) You'll be happy to know that the OSVR has Direct Rendering/Input thanks to Nvidia Game Works and that support has extended to the plugin. 
You will need to downloaded the 'osvrRenderManager' (Direct Render SDK) from the Developer Portal for this to work though(http://osvr.github.io/using/). Its an .msi installer so everything you'll need will be included on the install. 

Note: Make sure the version of the osvrRenderManager you download is osvrRenderManager0.6.25 or newer. Version 0.6.24 and lower does not have all the needed files which will cause the plugin to fail at compiling.
Note 2: The installer defaults to C:\Program Files\Sensics\osvrRenderManager0.6.25 as the path to install the osvrRenderManager. There is no way to change it during the install so you'll have to go to that default directory and copy and paste it if you want it else where.
Note 3: A restart is required after the install so make sure to save everything!

After installing the osverRenderManager0.6.25 (or newer) and restart your machine you'll have to start the 'ImportFromSDK' script and follow the steps that you originally did in step 1 again but this time you can fill out all of the steps the script will ask you for.

Below is an example of what the 'ImportFromSDK' command window should look like:

EXAMPLE:
Type OSVR 32bit SDK root dir:H:\OSVR\OSVR-Core-Snapshot-v0.6-428-gf10d9cb-build202-vs12-32bit
Type OSVR 64bit SDK root dir:H:\OSVR\OSVR-Core-Snapshot-v0.6-428-gf10d9cb-build202-vs12-64bit
Type DirectRender 64bit SDK root dir:H:\OSVR\osvrRenderManager0.6.25

Hit return and you'll see the command window start listing all the files its importing from three dictories you just typed in. This will take only a moment because there isn't many files to copy over. Once the script is done running
the command window will close on its own and your done. From there you will open the example map and let the plugin compile when it prompts you to do so. After compiling you the example map 'basic' should open. Nothing else you need to do.

Note: In the very beginning after you start the import you may notice that the script can't find a text file called: *.txt    If you do see this just ignore it, its the .ignore file from GitHub which is not needed.



Follow this steps to integrate OSVR into your UE4 project. It assumes that you have already run `ImportFromSDK` to copy the OSVR ClientKit SDK binaries into the source tree.

- be sure that your project has at least one C++ file
- close UE4Editor
- copy the `OSVRUnreal/Plugins/OSVR/` plugin directory into `<YourProject>/Plugins/`
- open your project and let UBT recompile the OSVR plugin
- under "Window -> Plugins" ensure that OSVR is the only plugin of type "Head Mounted Display" enabled (e.g., be sure to have "Oculus Rift Plugin" and "SteamVR Plugin" both disabled)

Optional, might make things easier:
- copy all contents of `<YourProject>\Plugins\OSVR\Source\OSVRClientKit\bin\` into `<YourProject>\Binaries\`

### Packaging

After having packaged your project:

- copy all contents of `<YourProject>\Plugins\OSVR\Source\OSVRClientKit\bin\` into `<OutputDir>\WindowsNoEditor\<YourProject>\Binaries\`

## Blueprint API

APIs identify OSVR devices/resources using the same OSVR resource path notation.
Following UE4 input management, as general rule:

- state of continuous valued resources ("axis" inputs in UE4) is queried by polling, using functions defined in the OSVR Blueprint Function Library.
+ state of discrete valued resources ("action" inputs in UE4) is reported by events. As UE4 currently does not support the definition of global events, OSVR events are wrapped in `OSVRInputComponent` and `OSVRActor` classes.

## `OSVRActor`

Predefined Actor class containing an `OSVRInputComponent`. 
It can be used as a base class for custom Actors or can be placed into a level to manage OSVR action events directly from the "Level Blueprint".

## `OSVRInputComponent`

Component exposing OSVR action events. You can use it to bind custom actions to OSVR action events like buttons press, etc.

### `void OnPositionChanged (FName name, FVector position);`

Fired when the position of an OSVR resource changes.

Outputs:

- `name`: OSVR resource path;
- `position`: the new position.

### `void OnOrientationChanged (FName name, FRotator orientation);`

Fired when the orientation of an OSVR resource changes.

Outputs:

- `name`: OSVR resource path;
- `position`: the new orientation.

### `void OnButtonStateChanged (FName name, EButtonState::Type state);`
Fired when the state of a button of an OSVR resource is updated.

Outputs:

- `name`: OSVR resource path;
- `state`: the new button state (`PRESSED`, `NOT_PRESSED`).

### `void OnAnalogValueChanged (FName name, float value);`

Fired when the value of a single-valued analog resource changes (e.g. a gamepad trigger analog control).

Outputs:

- `name`: OSVR resource path;
- `value`: the new value, in [0, 1].

## Blueprint function library

### `static FVector GetInterfacePosition(FName Name);`
Return the current position of an OSVR resource.

Inputs:

- `name`: OSVR resource path.

### `static FRotator GetInterfaceRotation(FName Name);`

Return the current orientation of an OSVR resource.

Inputs:

- `name`: OSVR resource path.

### `static EButtonState::Type GetInterfaceButtonState(FName Name);`

Return the current state of an OSVR button.

Inputs:

- `name`: OSVR resource path.

### `static float GetInterfaceAnalogValue(FName Name);`

Return the current value of a single-valued OSVR analog resource (e.g. a gamepad trigger analog control).

Inputs:

- `name`: OSVR resource path.

### `static void SetCurrentHmdOrientationAndPositionAsBase();`

Set the current position and orientation of OSVR HMD device as "rest" position.
