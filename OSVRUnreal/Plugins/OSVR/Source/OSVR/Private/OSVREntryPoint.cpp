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

#if OSVR_DEPRECATED_BLUEPRINT_API_ENABLED
#include "OSVRInterfaceCollection.h"
#endif

#include "OSVREntryPoint.h"

#include <chrono>
#include <thread>

DEFINE_LOG_CATEGORY(OSVREntryPointLog);

OSVREntryPoint::OSVREntryPoint()
{
	osvrClientContext = osvrClientInit("com.osvr.unreal.plugin");

    {
        bool clientContextOK = false;
        bool failure = false;
        auto begin = std::chrono::system_clock::now();
        auto end = begin + std::chrono::milliseconds(1000);

        while (std::chrono::system_clock::now() < end && !clientContextOK && !failure)
        {
            clientContextOK = osvrClientCheckStatus(osvrClientContext) == OSVR_RETURN_SUCCESS;
            if (!clientContextOK)
            {
                failure = osvrClientUpdate(osvrClientContext) == OSVR_RETURN_FAILURE;
                if (failure)
                {
                    UE_LOG(OSVREntryPointLog, Warning, TEXT("osvrClientUpdate failed during startup. Treating this as \"HMD not connected\""));
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        if (!clientContextOK)
        {
            UE_LOG(OSVREntryPointLog, Warning, TEXT("OSVR client context did not initialize correctly. Most likely the server isn't running. Treating this as if the HMD is not connected."));
        }
    }

#if OSVR_DEPRECATED_BLUEPRINT_API_ENABLED
	InterfaceCollection = MakeShareable(new OSVRInterfaceCollection(osvrClientContext));
#endif
}

OSVREntryPoint::~OSVREntryPoint()
{
#if OSVR_DEPRECATED_BLUEPRINT_API_ENABLED
	InterfaceCollection = nullptr;
#endif

	osvrClientShutdown(osvrClientContext);
}

void OSVREntryPoint::Tick(float DeltaTime)
{
	osvrClientUpdate(osvrClientContext);
}

bool OSVREntryPoint::IsOSVRConnected()
{
    return osvrClientContext && osvrClientCheckStatus(osvrClientContext) == OSVR_RETURN_SUCCESS;
}

#if OSVR_DEPRECATED_BLUEPRINT_API_ENABLED
OSVRInterfaceCollection* OSVREntryPoint::GetInterfaceCollection()
{
	return InterfaceCollection.Get();
}
#endif
