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

namespace {
    inline void CheckOSVR(OSVR_ReturnCode rc, const char* msg)
    {
        if (rc == OSVR_RETURN_FAILURE) {
            // error checking
            //UE_LOG(LogOSVRInputDevice, Warning, TEXT(msg));
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

    // @todo fill in buttons
    //Buttons[(int32)EControllerHand::Left][buttonIndex] = FGamepadKeyNames::UnrealButtonKeyName;
    //Buttons[(int32]EControllerHand::Right][buttonIndex] = FGamepadKeyNames::UnrealButtonKeyName;
    // etc...
    


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
#if 0
	MessageHandler->OnControllerButtonPressed(...);
	MessageHandler->OnControllerButtonReleased(...);
	MessageHandler->OnControllerAnalog(...);
#endif
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
