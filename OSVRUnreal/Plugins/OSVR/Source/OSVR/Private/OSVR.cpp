// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "OSVRPrivatePCH.h"

#include "OSVRInputDevice.h"

#include "InputCoreTypes.h"
#include "GameFramework/InputSettings.h"

#include "OSVREntryPoint.h"

#include "OSVRHMD.h"

#if 0
class FOSVR : public IOSVR
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	TSharedPtr< class FOSVRInputDevice > InputDevice;
};
#endif

class FOSVR : public IOSVR
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IHeadMountedDisplayModule implementation */
	virtual TSharedPtr< class IHeadMountedDisplay, ESPMode::ThreadSafe > CreateHeadMountedDisplay() override;

	// Pre-init the HMD module (optional).
	//virtual void PreInit() override;

	virtual OSVREntryPoint* GetEntryPoint() override;

	TSharedPtr< class OSVREntryPoint > EntryPoint;
};

IMPLEMENT_MODULE(FOSVR, OSVR)

OSVREntryPoint* FOSVR::GetEntryPoint()
{
	return EntryPoint.Get();
}

TSharedPtr< class IHeadMountedDisplay, ESPMode::ThreadSafe > FOSVR::CreateHeadMountedDisplay()
{
	TSharedPtr< FOSVRHMD, ESPMode::ThreadSafe > OSVRHMD(new FOSVRHMD());
	if (OSVRHMD->IsInitialized())
	{
		return OSVRHMD;
	}

	return nullptr;
}

//#if OSVR_INPUTDEVICE_ENABLED
//TSharedPtr< class IInputDevice > FOSVR::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
//{
//	FOSVRInputDevice::RegisterNewKeys();
//
//	InputDevice = MakeShareable(new FOSVRInputDevice(InMessageHandler));
//	return InputDevice;
//}
//#endif // OSVR_INPUTDEVICE_ENABLED

void FOSVR::StartupModule()
{
	IHeadMountedDisplayModule::StartupModule();

	EntryPoint = MakeShareable(new OSVREntryPoint());
}

void FOSVR::ShutdownModule()
{
	EntryPoint = nullptr;

	IHeadMountedDisplayModule::ShutdownModule();
}
