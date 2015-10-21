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
#include "OSVRInterface.h"
#include "OSVRTypes.h"

/*
static void OSVRPoseCallback(void * Userdata, const OSVR_TimeValue* Timestamp, const OSVR_PoseReport* Report)
{
	auto This = reinterpret_cast<OSVRInterface*>(Userdata);
	if (This && Report)
	{
		This->PoseState.SetTranslation(OSRV2FVector(Report->pose.translation));
		This->PoseState.SetRotation(OSRV2FQuat(Report->pose.rotation));

		This->Callback(This, OSVRInterface::POSE_STATE_AVAILABLE);
	}
}
*/

static void OSVRPositionCallback(void* Userdata, const OSVR_TimeValue* Timestamp, const OSVR_PositionReport* Report)
{
	auto This = reinterpret_cast< OSVRInterface* >(Userdata);
	if (This && Report)
	{
		This->PoseState.SetTranslation(OSVR2FVector(Report->xyz));

		This->Callback(This, OSVRInterface::POSITION_STATE_AVAILABLE);
	}
}

static void OSVROrientationCallback(void* Userdata, const OSVR_TimeValue* Timestamp, const OSVR_OrientationReport* Report)
{
	auto This = reinterpret_cast< OSVRInterface* >(Userdata);
	if (This && Report)
	{
		This->PoseState.SetRotation(OSVR2FQuat(Report->rotation));

		This->Callback(This, OSVRInterface::ORIENTATION_STATE_AVAILABLE);
	}
}

static void OSVRButtonCallback(void* Userdata, const OSVR_TimeValue* Timestamp, const OSVR_ButtonReport* Report)
{
	auto This = reinterpret_cast< OSVRInterface* >(Userdata);
	if (This && Report)
	{
		This->ButtonState = Report->state;

		This->Callback(This, OSVRInterface::BUTTON_STATE_AVAILABLE);
	}
}

static void OSVRAnalogCallback(void* Userdata, const OSVR_TimeValue* Timestamp, const OSVR_AnalogReport* Report)
{
	auto This = reinterpret_cast< OSVRInterface* >(Userdata);
	if (This && Report)
	{
		This->AnalogState = Report->state;

		This->Callback(This, OSVRInterface::ANALOG_STATE_AVAILABLE);
	}
}


OSVRInterface::OSVRInterface()
	: OSVRClientInterface(nullptr),
	  OSVRClientContext(nullptr),
	  Capabilities(0),
	  Callback(nullptr),
	  PoseState(FTransform::Identity),
	  AnalogState(0.0f),
	  ButtonState(0)
{
}

OSVRInterface::~OSVRInterface()
{
	Shutdown();
}

void OSVRInterface::OnStateChangedCallback(StateChangedCallback Function)
{
	Callback = Function;
}

bool OSVRInterface::Init(OSVR_ClientContext OSVRClientContext, const FName& InterfaceName)
{
	bool Result(true);

	OSVR_ReturnCode ReturnCode = osvrClientGetInterface(OSVRClientContext, InterfaceName.GetPlainANSIString(), &OSVRClientInterface);
	Result = ReturnCode == OSVR_RETURN_SUCCESS;

	this->OSVRClientContext = OSVRClientContext;

	RefreshCapabilities();
	RegisterCallbacks();

	Name = InterfaceName;

	return Result;
}

void OSVRInterface::Shutdown()
{
	DeregisterCallbacks();

	if (OSVRClientInterface != nullptr)
	{
		OSVR_ReturnCode ReturnCode = osvrClientFreeInterface(OSVRClientContext, OSVRClientInterface);
		check(ReturnCode == OSVR_RETURN_SUCCESS);

		OSVRClientInterface = nullptr;
	}
}

const FName& OSVRInterface::GetName() const
{
	return Name;
}

const uint32 OSVRInterface::GetCapabilities() const
{
	return Capabilities;
}

bool OSVRInterface::HasPositionState() const
{
	return (GetCapabilities() & POSITION_STATE_AVAILABLE) > 0;
}

bool OSVRInterface::HasOrientationState() const
{
	return (GetCapabilities() & ORIENTATION_STATE_AVAILABLE) > 0;
}

bool OSVRInterface::HasPoseState() const
{
	return (GetCapabilities() & POSE_STATE_AVAILABLE) > 0;
}

bool OSVRInterface::HasButtonState() const
{
	return (GetCapabilities() & BUTTON_STATE_AVAILABLE) > 0;
}

bool OSVRInterface::HasAnalogState() const
{
	return (GetCapabilities() & ANALOG_STATE_AVAILABLE) > 0;
}

bool OSVRInterface::GetPosition(FVector& Value, bool Latest) const
{
	if (Latest)
	{
        OSVR_TimeValue Time;
		OSVR_PositionState Position;
		OSVR_ReturnCode ReturnCode = osvrGetPositionState(OSVRClientInterface, &Time, &Position);

		Value = OSVR2FVector(Position);

		return ReturnCode == OSVR_RETURN_SUCCESS;
	}
	else
	{
		Value = PoseState.GetLocation();
	}

	return HasPositionState();
}

bool OSVRInterface::GetOrientation(FQuat& Value, bool Latest) const
{
	if (Latest)
	{
		OSVR_TimeValue Time;
		OSVR_OrientationState Orientation;
		OSVR_ReturnCode ReturnCode = osvrGetOrientationState(OSVRClientInterface, &Time, &Orientation);

		Value = OSVR2FQuat(Orientation);

		return ReturnCode == OSVR_RETURN_SUCCESS;
	}
	else
	{
		Value = PoseState.GetRotation();
	}

	return HasOrientationState();
}

bool OSVRInterface::GetPose(FTransform& Value, bool Latest) const
{
	if (Latest)
	{
		OSVR_TimeValue Time;
		OSVR_PoseState Pose;
		OSVR_ReturnCode ReturnCode = osvrGetPoseState(OSVRClientInterface, &Time, &Pose);

		Value.SetTranslation(OSVR2FVector(Pose.translation));
		Value.SetRotation(OSVR2FQuat(Pose.rotation));

		return ReturnCode == OSVR_RETURN_SUCCESS;
	}
	else
	{
		Value = PoseState;
	}

	return HasPoseState();
}

bool OSVRInterface::GetButtonState(uint8& Value, bool Latest) const
{
	if (Latest)
	{
		OSVR_TimeValue Time;
		OSVR_ButtonState Button;
		OSVR_ReturnCode ReturnCode = osvrGetButtonState(OSVRClientInterface, &Time, &Button);

		Value = Button;

		return ReturnCode == OSVR_RETURN_SUCCESS;
	}
	else
	{
		Value = ButtonState;
	}

	return HasButtonState();
}

bool OSVRInterface::GetAnalogState(float& Value, bool Latest) const
{
	if (Latest)
	{
		OSVR_TimeValue Time;
		OSVR_AnalogState Analog;
		OSVR_ReturnCode ReturnCode = osvrGetAnalogState(OSVRClientInterface, &Time, &Analog);

		Value = Analog;

		return ReturnCode == OSVR_RETURN_SUCCESS;
	}
	else
	{
		Value = AnalogState;
	}

	return HasAnalogState();
}

OSVR_ClientInterface OSVRInterface::GetRawInterface()
{
	return OSVRClientInterface;
}

void OSVRInterface::RefreshCapabilities()
{

	if (OSVRClientInterface == nullptr)
		return;

#if 0
	// Detection currently doesn't work (v0.1-261-gb6c8db7)

	OSVR_TimeValue Time;
	OSVR_PositionState Position;
	OSVR_OrientationState Orientation;
	OSVR_ButtonState Button;
	OSVR_AnalogState Analog;

	OSVR_ReturnCode HasPositonState = osvrGetPositionState(OSVRClientInterface, &Time, &Position);
	Capabilities |= (HasPositonState == OSVR_RETURN_SUCCESS) ? POSITION_STATE_AVAILABLE : 0;

	OSVR_ReturnCode HasOrientationState = osvrGetOrientationState(OSVRClientInterface, &Time, &Orientation);
	Capabilities |= (HasOrientationState == OSVR_RETURN_SUCCESS) ? ORIENTATION_STATE_AVAILABLE : 0;

	OSVR_ReturnCode HasButtonState = osvrGetButtonState(OSVRClientInterface, &Time, &Button);
	Capabilities |= (HasButtonState == OSVR_RETURN_SUCCESS) ? BUTTON_STATE_AVAILABLE : 0;

	OSVR_ReturnCode HasAnalogState = osvrGetAnalogState(OSVRClientInterface, &Time, &Analog);
	Capabilities |= (HasAnalogState == OSVR_RETURN_SUCCESS) ? ANALOG_STATE_AVAILABLE : 0;
#else
	Capabilities = POSITION_STATE_AVAILABLE | ORIENTATION_STATE_AVAILABLE | BUTTON_STATE_AVAILABLE | ANALOG_STATE_AVAILABLE;
#endif

}

void OSVRInterface::RegisterCallbacks()
{

	if (HasPoseState())
	{
		//osvrRegisterPoseCallback(GetRawInterface(), &OSVRPoseCallback, this);
	}
	/*else*/ if (HasPositionState())
	{
		osvrRegisterPositionCallback(GetRawInterface(), &OSVRPositionCallback, this);
	}
	/*else*/ if (HasOrientationState())
	{
		osvrRegisterOrientationCallback(GetRawInterface(), &OSVROrientationCallback, this);
	}

	if (HasAnalogState())
	{
		osvrRegisterAnalogCallback(GetRawInterface(), &OSVRAnalogCallback, this);
	}

	if (HasButtonState())
	{
		osvrRegisterButtonCallback(GetRawInterface(), &OSVRButtonCallback, this);
	}

}

void OSVRInterface::DeregisterCallbacks()
{
	//@TODO: at the moment it's not possible to unregister callbacks in OSVR
}
