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

#include "OSVRPrivatePCH.h"

#include "GenericPlatformMath.h"

#include "OSVRInputDevice.h"

#include "SlateBasics.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "WindowsCursor.h"
#include "GenericApplicationMessageHandler.h"

#include "OSVRTypes.h"

extern OSVR_ClientContext osvrClientContext;

DEFINE_LOG_CATEGORY_STATIC(LogOSVRInputDevice, Log, All);

void FOSVRInputDevice::RegisterNewKeys()
{
}

FOSVRInputDevice::FOSVRInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
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

    //FSteamVRHMD* SteamVRHMD = GetSteamVRHMD();
    //if (SteamVRHMD)
    //{
    //    FQuat DeviceOrientation = FQuat::Identity;
    //    RetVal = SteamVRHMD->GetControllerHandPositionAndOrientation(ControllerIndex, DeviceHand, OutPosition, DeviceOrientation);
    //    OutOrientation = DeviceOrientation.Rotator();
    //}

    return RetVal;
}

void FOSVRInputDevice::Tick(float DeltaTime)
{
	osvrClientUpdate(osvrClientContext);
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
