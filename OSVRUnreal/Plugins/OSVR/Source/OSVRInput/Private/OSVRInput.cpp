// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "OSVRInputPrivatePCH.h"
#include "IOSVRInput.h"
#include "OSVRInputDevice.h"

#include "InputCoreTypes.h"
#include "GameFramework/InputSettings.h"

class FOSVRInput : public IOSVRInput
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	TSharedPtr< class FOSVRInputDevice > InputDevice;
};

// class FOSVR : public IOSVR
// {
// 	/** IModuleInterface implementation */
// 	virtual void StartupModule() override;
// 	virtual void ShutdownModule() override;

// 	/** IHeadMountedDisplayModule implementation */
// 	virtual TSharedPtr< class IHeadMountedDisplay, ESPMode::ThreadSafe > CreateHeadMountedDisplay() override;

// 	// Pre-init the HMD module (optional).
// 	//virtual void PreInit() override;

// 	virtual OSVREntryPoint* GetEntryPoint() override;

// 	TSharedPtr< class OSVREntryPoint > EntryPoint;
// };

IMPLEMENT_MODULE(FOSVRInput, OSVRInput)

TSharedPtr< class IInputDevice > FOSVRInput::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	FOSVRInputDevice::RegisterNewKeys();

	InputDevice = MakeShareable(new FOSVRInputDevice(InMessageHandler));
	return InputDevice;
}

void FOSVRInput::StartupModule()
{
	IInputDeviceModule::StartupModule();

	//EntryPoint = MakeShareable(new OSVREntryPoint());
}

void FOSVRInput::ShutdownModule()
{
	//EntryPoint = nullptr;

	IInputDeviceModule::ShutdownModule();
}
