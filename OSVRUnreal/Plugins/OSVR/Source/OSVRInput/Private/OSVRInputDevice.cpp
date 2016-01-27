//
// Copyright 2014, 2015 Razer Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "OSVRInputPrivatePCH.h"

#include "GenericPlatformMath.h"
#include "OSVREntryPoint.h"
#include "OSVRInputDevice.h"

#include "SlateBasics.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "WindowsCursor.h"
#include "GenericApplicationMessageHandler.h"

#include "OSVRTypes.h"
#include "IOSVR.h"
#include "OSVRHMD.h"

#include <osvr/ClientKit/InterfaceStateC.h>

OSVR_ClientContext context;

//DEFINE_LOG_CATEGORY(LogOSVRInputDevice);
enum OSVRButtonType {
    OSVR_BUTTON_TYPE_DIGITAL,
    OSVR_BUTTON_TYPE_ANALOG,
    OSVR_BUTTON_TYPE_ANALOG_THRESHOLD
};

class OSVRButton {
private:
    OSVR_ClientContext context;

public:
    OSVRButton() {}
    OSVRButton(OSVRButtonType _type, FName _key, const std::string& _ifacePath) :
        type(_type), key(_key), ifacePath(_ifacePath) {}

    virtual ~OSVRButton() {
        if (iface && context) {
            osvrClientFreeInterface(context, iface);
        }
    }

    inline void init(OSVR_ClientContext _context) {
        context = _context;
        if (osvrClientGetInterface(context, ifacePath.c_str(), &iface) != OSVR_RETURN_SUCCESS) {
            isValid = false;
        }
    }

    bool oldState = false;
    bool isValid = false;
    float threshold = 0.75f;
    FName key;
    std::string ifacePath;
    OSVR_ClientInterface iface = nullptr;
    OSVRButtonType type;
    std::queue<bool> digitalStateQueue;
    std::queue<float> analogStateQueue;
};

namespace {
    inline void CheckOSVR(OSVR_ReturnCode rc, const char* msg)
    {
        if (rc == OSVR_RETURN_FAILURE) {
            // error checking
            //UE_LOG(LogOSVRInputDevice, Warning, TEXT(msg));
        }
    }

    void buttonCallback(void *userdata, const OSVR_TimeValue *timestamp, const OSVR_ButtonReport *report) {
        OSVRButton* button = static_cast<OSVRButton*>(userdata);
        button->digitalStateQueue.push(report->state == OSVR_BUTTON_PRESSED);
    }

    void analogCallback(void *userdata, const OSVR_TimeValue *timestamp, const OSVR_AnalogReport *report) {
        OSVRButton* button = static_cast<OSVRButton*>(userdata);
        if (button->type == OSVR_BUTTON_TYPE_ANALOG_THRESHOLD) {
            bool newState = report->state > threshold;
            if (newState != button->oldState) {
                button->digitalStateQueue.push(newState);
            }
            button->oldState = newState;
        }
        else {
            button->analogStateQueue.push(report->state);
        }
    }
}

void FOSVRInputDevice::RegisterNewKeys()
{
}

FOSVRInputDevice::FOSVRInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
    : MessageHandler(InMessageHandler)
{
    // make sure OSVR module is loaded.
    context = osvrClientInit("com.osvr.unreal.plugin.input");

    auto key = FGamepadKeyNames::SpecialLeft;
    // @todo fill in buttons
    //Buttons[(int32)EControllerHand::Left][buttonIndex] = FGamepadKeyNames::UnrealButtonKeyName;
    //Buttons[(int32]EControllerHand::Right][buttonIndex] = FGamepadKeyNames::UnrealButtonKeyName;
    // etc...

    auto lh = (int32)EControllerHand::Left;
    auto rh = (int32)EControllerHand::Right;

    osvrButtons = {
        // left hand
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::SpecialLeft, "/controller/left/middle"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Shoulder, "/controller/left/bumper"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Thumbstick, "/controller/left/joystick/button"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_FaceButton1, "/controller/left/1"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_FaceButton2, "/controller/left/2"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_FaceButton3, "/controller/left/3"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_FaceButton4, "/controller/left/4"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::MotionController_Left_Thumbstick_X, "/controller/left/joystick/x"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::MotionController_Left_Thumbstick_Y, "/controller/left/joystick/y"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::MotionController_Left_TriggerAxis, "/controller/left/trigger"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG_THRESHOLD, FGamepadKeyNames::MotionController_Left_Trigger, "/controller/left/trigger"),


        // right hand
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::SpecialRight, "/controller/right/middle"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Right_Shoulder, "/controller/right/bumper"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Right_Thumbstick, "/controller/right/joystick/button"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Right_FaceButton1, "/controller/right/1"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Right_FaceButton2, "/controller/right/2"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Right_FaceButton3, "/controller/right/3"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Right_FaceButton4, "/controller/right/4"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::MotionController_Right_Thumbstick_X, "/controller/right/joystick/x"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::MotionController_Right_Thumbstick_Y, "/controller/right/joystick/y"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::MotionController_Right_TriggerAxis, "/controller/right/trigger"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG_THRESHOLD, FGamepadKeyNames::MotionController_Right_Trigger, "/controller/right/trigger"),

        // "controller" (like xbox360)
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::RightShoulder, "/controller/right/bumper"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::RightThumb, "/controller/right/joystick/button"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::FaceButtonBottom, "/controller/right/1"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::FaceButtonRight, "/controller/right/2"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::FaceButtonLeft, "/controller/right/3"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::FaceButtonTop, "/controller/right/4"),

        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::LeftShoulder, "/controller/left/bumper"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::LeftThumb, "/controller/left/joystick/button"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::DPadDown, "/controller/left/1"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::DPadRight, "/controller/left/2"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::DPadLeft, "/controller/left/3"),
        OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::DPadUp, "/controller/left/4"),

        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::LeftAnalogX, "/controller/left/joystick/x"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::LeftAnalogY, "/controller/left/joystick/y"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::RightAnalogX, "/controller/right/joystick/x"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::RightAnalogY, "/controller/right/joystick/y"),

        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::LeftTriggerAnalog, "/controller/left/trigger"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG, FGamepadKeyNames::RightTriggerAnalog, "/controller/right/trigger"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG_THRESHOLD, FGamepadKeyNames::LeftTriggerThreshold, "/controller/left/trigger"),
        OSVRButton(OSVR_BUTTON_TYPE_ANALOG_THRESHOLD, FGamepadKeyNames::RightTriggerThreshold, "/controller/right/trigger"),
    };

    for (size_t i = 0; i < osvrButtons.size(); i++) {
        osvrButtons[i].init(context);

        if (osvrButtons[i].type == OSVR_BUTTON_TYPE_DIGITAL) {
            using namespace std::placeholders;
            osvrRegisterButtonCallback(osvrButtons[i].iface, buttonCallback, &osvrButtons[i]);
        }

        if (osvrButtons[i].type == OSVR_BUTTON_TYPE_ANALOG ||
            osvrButtons[i].type == OSVR_BUTTON_TYPE_ANALOG_THRESHOLD) {
            osvrRegisterAnalogCallback(osvrButtons[i].iface, analogCallback, &osvrButtons[i]);
        }
    }

    CheckOSVR(osvrClientGetInterface(context, "/me/hands/left", &leftHand),
        "Couldn't get left hand interface.");

    CheckOSVR(osvrClientGetInterface(context, "/me/hands/right", &rightHand),
        "Couldn't get right hand interface.");

    IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);

    // This may need to be removed in a future version of the engine.
    // From the SteamVR plugin: "construction of the controller happens after InitializeMotionControllers(), so we manually add this to the array here"
    GEngine->MotionControllerDevices.AddUnique(this);
}

FOSVRInputDevice::~FOSVRInputDevice()
{
    //GEngine->MotionControllerDevices.Remove(this); // This crashes. Maybe they changed something in the engine since the steamvr plugin was written?
    if (leftHand) {
        osvrClientFreeInterface(context, leftHand);
    }
    if (rightHand) {
        osvrClientFreeInterface(context, rightHand);
    }
    osvrClientShutdown(context);
}

void FOSVRInputDevice::EventReport(const FKey& Key, const FVector& Translation, const FQuat& Orientation)
{
}

/**
* Returns the calibration-space orientation of the requested controller's hand.
*
* @param ControllerIndex	The Unreal controller (player) index of the contoller set
* @param DeviceHand		Which hand, within the controller set for the player, to get the orientation and position for
* @param OutOrientation	(out) If tracked, the orientation (in calibrated-space) of the controller in the specified hand
* @param OutPosition		(out) If tracked, the position (in calibrated-space) of the controller in the specified hand
* @return					True if the device requested is valid and tracked, false otherwise
*/
bool FOSVRInputDevice::GetControllerOrientationAndPosition(const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition) const
{
    bool RetVal = false;
    if (ControllerIndex == 0) {
        if (osvrClientCheckStatus(context) == OSVR_RETURN_SUCCESS) {
            auto iface = DeviceHand == EControllerHand::Left ? leftHand : rightHand;
            OSVR_PoseState state;
            OSVR_TimeValue tvalue;
            if (osvrGetPoseState(iface, &tvalue, &state) == OSVR_RETURN_SUCCESS) {
                float worldToMetersScale = IOSVR::Get().GetHMD()->GetWorldToMetersScale();
                OutPosition = OSVR2FVector(state.translation) * worldToMetersScale;
                OutOrientation = OSVR2FQuat(state.rotation).Rotator();
                RetVal = true;
            }
        }
    }
    return RetVal;
}

void FOSVRInputDevice::Tick(float DeltaTime)
{
    osvrClientUpdate(context);
}

void FOSVRInputDevice::SendControllerEvents()
{
    const int32 controllerId = 0;
    for (size_t i = 0; i < osvrButtons.size(); i++) {
        auto& button = osvrButtons[i];
        while (!button.digitalStateQueue.empty()) {
            auto state = button.digitalStateQueue.front();
            button.digitalStateQueue.pop();
            if (state) {
                MessageHandler->OnControllerButtonPressed(button.key, controllerId, false);
            }
            else {
                MessageHandler->OnControllerButtonReleased(button.key, controllerId, false);
            }
        }
        while (!button.analogStateQueue.empty()) {
            auto state = button.analogStateQueue.front();
            button.analogStateQueue.pop();
            MessageHandler->OnControllerAnalog(button.key, controllerId, state);
        }
    }
}

void FOSVRInputDevice::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
    MessageHandler = InMessageHandler;
}

bool FOSVRInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
    return true;
}

void FOSVRInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
}

void FOSVRInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{
}
