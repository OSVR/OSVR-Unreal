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

#include <osvr/renderkit/RenderManager.h>

OSVR_ClientContext osvrClientContext(nullptr);
osvr::renderkit::RenderManager* gRenderManager(nullptr);

OSVREntryPoint::OSVREntryPoint()
{
	osvrClientContext = osvrClientInit("com.osvr.unreal.plugin");

#if PLATFORM_WINDOWS
    gRenderManager = osvr::renderkit::createRenderManager(osvrClientContext, "DirectX11");
#else
    gRenderManager = osvr::renderkit::createRenderManager(osvrClientContext, "OpenGL");
#endif

	InterfaceCollection = MakeShareable(new OSVRInterfaceCollection(
		osvrClientContext
		));
}

OSVREntryPoint::~OSVREntryPoint()
{
	InterfaceCollection = nullptr;
    if (nullptr != gRenderManager) {
        delete gRenderManager;
    }
	osvrClientShutdown(osvrClientContext);
}

void OSVREntryPoint::Tick(float DeltaTime)
{
	osvrClientUpdate(osvrClientContext);
}

OSVRInterfaceCollection* OSVREntryPoint::GetInterfaceCollection()
{
	return InterfaceCollection.Get();
}
