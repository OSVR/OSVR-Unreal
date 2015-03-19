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

#include "GameFramework/Actor.h"
#include "OSVRButtonState.h"
#include "OSVRActor.generated.h"

class UOSVRInputComponent;

//USTRUCT()
//struct FButtonData
//{
//	GENERATED_USTRUCT_BODY()
//
//	UPROPERTY()
//	FName Name;
//};

UCLASS()
class OSVR_API AOSVRActor : public AActor
{
	GENERATED_BODY()

public:
	AOSVRActor(const class FObjectInitializer& OI);

	UPROPERTY(VisibleAnywhere, Category = "OSVR")
	class UOSVRInputComponent* osvrInput;

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnPoseChanged(FName Interface, const FTransform & Pose);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnPositionChanged(FName Interface, FVector Position);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnOrientationChanged(FName Interface, FRotator Rotation);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnAnalogValueChanged(FName Interface, float Value);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnButtonStateChanged(FName Interface, EButtonState::Type State);
};
