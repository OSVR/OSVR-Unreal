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


#pragma once

#include <functional>

class OSVRInterface
{
public:

	typedef std::function<void(OSVRInterface*, uint32)> StateChangedCallback;

	enum EInterfaceCapabilities
	{
		POSITION_STATE_AVAILABLE	= 1 << 1,
		ORIENTATION_STATE_AVAILABLE = 1 << 2,
		POSE_STATE_AVAILABLE		= (POSITION_STATE_AVAILABLE | ORIENTATION_STATE_AVAILABLE),
		BUTTON_STATE_AVAILABLE		= 1 << 3,
		ANALOG_STATE_AVAILABLE		= 1 << 4,
	};

	const FName& GetName() const;
	const uint32 GetCapabilities() const;

	bool HasPositionState() const;
	bool HasOrientationState() const;
	bool HasPoseState() const;
	bool HasButtonState() const;
	bool HasAnalogState() const;

	bool GetPosition(FVector& Value, bool Latest) const;
	bool GetOrientation(FQuat& Value, bool Latest) const;
	bool GetPose(FTransform& Value, bool Latest) const;
	bool GetButtonState(uint8& Value, bool Latest) const;
	bool GetAnalogState(float& Value, bool Latest) const;

	void OnStateChangedCallback(StateChangedCallback Function);

	OSVRInterface();
	virtual ~OSVRInterface();

	virtual bool Init(OSVR_ClientContext OSVRClientContext, const FName& InterfaceName);
	virtual void Shutdown();

private:

	OSVR_ClientInterface GetRawInterface();

	void RefreshCapabilities();
	void RegisterCallbacks();
	void DeregisterCallbacks();

private:

#if OSVR_ENABLED
	friend static void OSVRPoseCallback(void * Userdata, const OSVR_TimeValue* Timestamp, const OSVR_PoseReport* Report);
	friend static void OSVRPositionCallback(void * Userdata, const OSVR_TimeValue* Timestamp, const OSVR_PositionReport* Report);
	friend static void OSVROrientationCallback(void * Userdata, const OSVR_TimeValue* Timestamp, const OSVR_OrientationReport* Report);
	friend static void OSVRButtonCallback(void * Userdata, const OSVR_TimeValue* Timestamp, const OSVR_ButtonReport* Report);
	friend static void OSVRAnalogCallback(void * Userdata, const OSVR_TimeValue* Timestamp, const OSVR_AnalogReport* Report);
#endif // OSVR_ENABLED

	OSVRInterface(const OSVRInterface&);
	OSVRInterface& operator=(const OSVRInterface&);

	OSVR_ClientInterface	OSVRClientInterface;
	OSVR_ClientContext		OSVRClientContext;

	FName					Name;
	uint32					Capabilities;

	StateChangedCallback	Callback;

	FTransform				PoseState;
	float					AnalogState;
	uint8					ButtonState;
};
