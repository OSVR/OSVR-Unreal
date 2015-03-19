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
