# OSVR - Unreal Engine 4 plugin

## Installation from Source

Follow the steps to integrate OSVR into your UE4 project as described in README.md.

## Controller and Motion Controller support

OSVR-Unreal supports the Unreal controller and motion controller interfaces. This means you can use the standard built-in OSVR-Unreal blueprint events for either the standard controller type or the motion controller style.

See the following tutorial on Unreal's site to learn how to integrate motion controls and controller buttons into your Unreal game:
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

APIs identify OSVR devices/resources using the same OSVR resource path notation.
Following UE4 input management, as general rule:

- state of continuous valued resources ("axis" inputs in UE4) is queried by polling, using functions defined in the OSVR Blueprint Function Library.
- state of discrete valued resources ("action" inputs in UE4) is reported by events. As UE4 currently does not support the definition of global events, OSVR events are wrapped in `OSVRInputComponent` and `OSVRActor` classes.

## `OSVRActor`

Predefined Actor class containing an `OSVRInputComponent`.
It can be used as a base class for custom Actors or can be placed into a level to manage OSVR action events directly from the "Level Blueprint".

## `OSVRInputComponent`

If there are buttons or other tracker interfaces that don't fit into the standard Unreal motion controller or standard controller model, then `OSVRInputComponent` can be used to expose OSVR action events for that interface. You can use it to bind custom actions to OSVR action events like buttons press, etc.

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
