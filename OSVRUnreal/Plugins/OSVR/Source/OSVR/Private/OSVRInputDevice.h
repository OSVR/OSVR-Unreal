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
 
#if OSVR_INPUTDEVICE_ENABLED

#include "IForceFeedbackSystem.h"
#include "IInputDevice.h"
 
/**
*
*/
class FOSVRInputDevice : public IInputDevice
{
public:
	FOSVRInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& MessageHandler);

	static void RegisterNewKeys();
 
	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override;
 
	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;
 
	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;
 
	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
 
	// IForceFeedbackSystem pass through functions
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override;
 
	void EventReport(const FKey& Key, const FVector& Translation, const FQuat& Orientation);

private:

	TSharedRef< FGenericApplicationMessageHandler > MessageHandler;

};

#endif // OSVR_INPUTDEVICE_ENABLED