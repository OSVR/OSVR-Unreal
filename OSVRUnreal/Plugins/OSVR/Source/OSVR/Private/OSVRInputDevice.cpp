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

#if OSVR_INPUTDEVICE_ENABLED

#include "GenericPlatformMath.h"

#include "OSVRInputDevice.h"

#include "Slate.h"

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

#endif // OSVR_INPUTDEVICE_ENABLED