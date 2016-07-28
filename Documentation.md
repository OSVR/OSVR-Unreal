# OSVR - Unreal Engine 4 plugin

## Installation from Source

Follow the steps to integrate OSVR into your UE4 project as described in [README.md](README.md).

## Controller and Motion Controller support

OSVR-Unreal supports the standard Unreal controller and motion controller interfaces. This means you can use the standard built-in Unreal blueprint events for either the standard controller type or the motion controller style. OSVR already supports a very wide variety of motion tracking devices and controllers that can now be used easily in any Unreal game. To see the full list of devices that OSVR supports, see this list (and don't forget to look at the large list of additional devices supported by VRPN, which can be used by OSVR with some additional configuration): [OSVR Device Compatibility List](http://osvr.github.io/compatibility/)

Also take a look at the following tutorial on Unreal's site to learn how to integrate motion controls and controller buttons into your Unreal game:
https://docs.unrealengine.com/latest/INT/Platforms/VR/MotionController/index.html

The OSVR-Unreal plugin looks at the following interface paths and maps them to the appropriate standard Unreal interface:
 - Hand Tracking support
    - `/me/hands/left` is mapped to the left hand's position/orientation.
    - `/me/hands/right` is mapped to the right hand's position/orientation.
 - Controller/Buttons - the following paths are mapped the appropriate Unreal buttons:
   - `/controller/left/1`
   - `/controller/left/2`
   - `/controller/left/3`
   - `/controller/left/4`
   - `/controller/left/joystick/button`
    - `/controller/left/bumper`
    - `/controller/left/middle` (maps to the "left special" button in Unreal)
    - All of the above, but substitute `right` in place of `left`.
    - Grip buttons are NOT mapped at this time.
 - Analog buttons/axis
   - `/controller/left/joystick/x`
   - `/controller/left/joystick/y`
   - `/controller/left/trigger`
   - All of the above, but substitute `right` in place of `left`.
   - Grip axis are NOT mapped at this time.
 - Threshold buttons
    - These are analog buttons that map to digital buttons in the Unreal system, by using a threshold. Any analog value over or under a given threshold will trigger the button to be 'pressed'.
    - Left and right triggers (e.g. `/controller/left/trigger`)
    - Up/Down/Left/Right on both joysticks. For example, `/controller/left/joystick/x` and `/controller/left/joystick/y` map to four threshold buttons representing a digital up/down/left/right direction buttons.
 - Standard controller vs motion controller
    - In the current implementation, the hydra style path tree above is mapped not just to the new Unreal motion controller button sets that separate left/right hand buttons, but also the standard "traditional" controller buttons, as you might see on an XBox 360 or XBox One controller. In this case, we map Razor Hydra style controllers to the traditional buttons as follows:
        - Left and right joysticks, joystick buttons, triggers, and bumper buttons map to the left and right joysticks, joystick triggers, and shoulder buttons of a standard controller, as you might expect.
        - 1,2,3,4 face buttons map to the D-Pad buttons on the left hand and the face buttons on the right. In this case, 1 is down, 2 is right, 3 is left, and 4 is up.
        - `/controller/left/middle` and `/controller/right/middle` are mapped to special left and special right, respectively.

## Blueprint API
The original OSVR Blueprint API is being redesigned. The original OSVR Blueprint API should be considered deprecated. It has been archived in /Archive if you need it. To use the original blueprint API, copy the contents of the /Archive/Plugins directory to the OSVRUnreal/Plugins directory, and set `OSVR_DEPRECATED_BLUEPRINT_API_ENABLED` to 1 in `/OSVRUnreal/Plugins/OSVR/Source/Private/OSVRPrivatePCH.h`.

## Server Auto-start
OSVR-Unreal automatically starts the OSVR server during initialization, if it is not already running.
The auto-start API that the plugin uses is driven on Windows, Mac, and Linux by the `OSVR_SERVER_ROOT`
environment variable. The windows OSVR SDK and runtime installers automatically set this
environment variable during installation but you can set it to any server directory you wish.
To turn off auto-start in OSVR-Unreal and other OSVR applications, simply remove
the `OSVR_SERVER_ROOT` environment variable.
