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