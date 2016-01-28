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

#pragma once

#include "OSVRInterfaceCollection.h"

OSVR_API class OSVREntryPoint : public FTickableGameObject
{
public:
	OSVREntryPoint();
	virtual ~OSVREntryPoint();

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override
	{
		return (!GWorld->HasBegunPlay() && GIsEditor) ? false : true;
	}
	virtual bool IsTickableWhenPaused() const override
	{
		return true;
	}
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(OSVREntryPoint, STATGROUP_Tickables);
	}

	OSVRInterfaceCollection* GetInterfaceCollection();

private:
	TSharedPtr< OSVRInterfaceCollection > InterfaceCollection;
};
