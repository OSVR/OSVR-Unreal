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

#include "OSVRButtonState.h"
#include "OSVRBPFunctionLibrary.generated.h"

UCLASS()
class UOSVRBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static FTransform GetInterfacePose(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static FVector GetInterfacePosition(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static FRotator GetInterfaceRotation(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static EButtonState::Type GetInterfaceButtonState(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static float GetInterfaceAnalogValue(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input|HeadMountedDisplay")
	static void SetCurrentHmdOrientationAndPositionAsBase();
};