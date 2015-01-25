
#include "OSVRPrivatePCH.h"

#include "OSVRInterfaceCollection.h"
#include "OSVREntryPoint.h"

#if OSVR_ENABLED
OSVR_ClientContext osvrClientContext(nullptr);
#endif // OSVR_ENABLED

OSVREntryPoint::OSVREntryPoint()
{
#if OSVR_ENABLED
	osvrClientContext = osvrClientInit("OSVR Unreal Engine 4 Plugin");
#endif // OSVR_ENABLED

	InterfaceCollection = MakeShareable(new OSVRInterfaceCollection(
#if OSVR_ENABLED
		osvrClientContext
#else
		nullptr
#endif // OSVR_ENABLED
	));
}

OSVREntryPoint::~OSVREntryPoint()
{
	InterfaceCollection = nullptr;

#if OSVR_ENABLED
	osvrClientShutdown(osvrClientContext);
#endif // OSVR_ENABLED
}

void OSVREntryPoint::Tick(float DeltaTime)
{
#if OSVR_ENABLED
	osvrClientUpdate(osvrClientContext);
#endif // OSVR_ENABLED
}

OSVRInterfaceCollection* OSVREntryPoint::GetInterfaceCollection()
{
	return InterfaceCollection.Get();
}
