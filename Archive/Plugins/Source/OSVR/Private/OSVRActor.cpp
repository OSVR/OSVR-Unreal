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
#include "OSVRActor.h"

#include "IOSVR.h"
#include "OSVREntryPoint.h"
#include "OSVRInterfaceCollection.h"
#include "OSVRInputComponent.h"

AOSVRActor::AOSVRActor(const FObjectInitializer& OI)
	: AActor(OI)
{
	osvrInput = OI.CreateDefaultSubobject< UOSVRInputComponent >(this, TEXT("osvrInput"));
	osvrInput->OnPoseChanged.AddDynamic(this, &AOSVRActor::OnPoseChanged);
	osvrInput->OnPositionChanged.AddDynamic(this, &AOSVRActor::OnPositionChanged);
	osvrInput->OnOrientationChanged.AddDynamic(this, &AOSVRActor::OnOrientationChanged);
	osvrInput->OnButtonStateChanged.AddDynamic(this, &AOSVRActor::OnButtonStateChanged);
	osvrInput->OnAnalogValueChanged.AddDynamic(this, &AOSVRActor::OnAnalogValueChanged);
}
