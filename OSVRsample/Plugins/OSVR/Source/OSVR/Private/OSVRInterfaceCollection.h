
#pragma once

#include "OSVRInterface.h"
#include <functional>

class OSVRInterfaceCollection
{
public:

	OSVRInterfaceCollection(OSVR_ClientContext osvrClientContext);

	OSVRInterface* GetInterface(const FName& Name){ return GetInterfaceImpl(Name, false); }
	OSVRInterface* GetOrCreateInterface(const FName& Name){ return GetInterfaceImpl(Name, true); }

	struct RegistrationToken
	{
		static const int32 INVALID_TOKEN = -1;

		RegistrationToken() : Token(INVALID_TOKEN){}
		explicit RegistrationToken(int32 InToken) : Token(InToken) {}

		int32 Token;
	};

	RegistrationToken RegisterOnStateChangedCallback(OSVRInterface::StateChangedCallback Function);
	void DeregisterOnStateChangedCallback(RegistrationToken Token);

private:

	void AddDefaultInterfaces();
	void ClearAllInterfaces();

	OSVRInterface* GetInterfaceImpl(const FName& Name, bool CreateIfNotExist);

	void PropagateCallbacks(OSVRInterface* Interface, uint32 Caps);

	TArray<OSVRInterface::StateChangedCallback>		Callbacks;
	TArray<TSharedPtr<OSVRInterface> >				Interfaces;
	OSVR_ClientContext								osvrClientContext;
};
