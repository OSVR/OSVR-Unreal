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

using namespace std::placeholders;

OSVRInterfaceCollection::OSVRInterfaceCollection(OSVR_ClientContext osvrClientContext)
{
	this->osvrClientContext = osvrClientContext;

	AddDefaultInterfaces();
}

void OSVRInterfaceCollection::AddDefaultInterfaces()
{
	// ie)
	//	/me/hands/left		- position / orientation(pose) tracking data from left Razer Hydra controller.
	//	/me/hands/right		- pose from right Hydra controller
	//	/me/hands			- pose from both left and right
	//	/me/head			- orientation from YEI 3Space (HMD)tracker

	FName ValidPaths[] =
		{
			"/me/hands/left",
			"/me/hands/right",
			"/me/hands",
			"/me/head",
			"/controller/left/1",
			"/controller/left/2",
			"/controller/left/3",
			"/controller/left/4",
			"/controller/left/bumper",
			"/controller/left/joystick/button",
			"/controller/left/middle",
			"/controller/right/1",
			"/controller/right/2",
			"/controller/right/3",
			"/controller/right/4",
			"/controller/right/bumper",
			"/controller/right/joystick/button",
			"/controller/right/middle"};

	for (int i = 0; i < sizeof(ValidPaths) / sizeof(FName); ++i)
	{
		GetInterfaceImpl(ValidPaths[i], true);
	}
}

void OSVRInterfaceCollection::ClearAllInterfaces()
{
	Callbacks.Empty();
	Interfaces.Empty();
}

OSVRInterface* OSVRInterfaceCollection::GetInterfaceImpl(const FName& Name, bool CreateIfNotExist)
{
	for (auto& Interface : Interfaces)
	{
		if (Interface->GetName() == Name)
			return Interface.Get();
	}

	if (CreateIfNotExist)
	{
		auto Interface = MakeShareable(new OSVRInterface());
		if (Interface.Object->Init(osvrClientContext, Name))
		{
			Interface.Object->OnStateChangedCallback(std::bind(&OSVRInterfaceCollection::PropagateCallbacks, this, _1, _2));
			Interfaces.Add(Interface);
			return Interface.Object;
		}
	}

	return nullptr;
}

void OSVRInterfaceCollection::PropagateCallbacks(OSVRInterface* Interface, uint32 Caps)
{
	for (auto Callback : Callbacks)
	{
		if (Callback != nullptr)
			Callback(Interface, Caps);
	}
}

OSVRInterfaceCollection::RegistrationToken OSVRInterfaceCollection::RegisterOnStateChangedCallback(OSVRInterface::StateChangedCallback Function)
{
	return RegistrationToken(Callbacks.Add(Function));
}

void OSVRInterfaceCollection::DeregisterOnStateChangedCallback(OSVRInterfaceCollection::RegistrationToken RegistrationToken)
{
	if (RegistrationToken.Token != RegistrationToken::INVALID_TOKEN)
		Callbacks[RegistrationToken.Token] = nullptr;
}
