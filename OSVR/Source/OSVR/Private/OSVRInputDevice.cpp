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

#if OSVR_ENABLED
extern OSVR_ClientContext osvrClientContext;
#endif // OSVR_ENABLED

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
#if OSVR_ENABLED
	osvrClientUpdate(osvrClientContext);
#endif // OSVR_ENABLED
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

void FOSVRInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values)
{
}

#endif // OSVR_INPUTDEVICE_ENABLED