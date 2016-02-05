// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "OSVRButtonState.h"
#include "OSVRInputComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Utility, meta = (BlueprintSpawnableComponent))
class UOSVRInputComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPoseChangedDelegate, FName, Name, const FTransform&, Pose);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPositionChangedDelegate, FName, Name, FVector, Position);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrientationChangedDelegate, FName, Name, FRotator, Orientation);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FButtonStateChangedDelegate, FName, Name, EButtonState::Type, State);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnalogValueChangedDelegate, FName, Name, float, Value);

	UPROPERTY(BlueprintAssignable, Category = "OSVR")
	FPoseChangedDelegate OnPoseChanged;

	UPROPERTY(BlueprintAssignable, Category = "OSVR")
	FPositionChangedDelegate OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "OSVR")
	FOrientationChangedDelegate OnOrientationChanged;

	UPROPERTY(BlueprintAssignable, Category = "OSVR")
	FButtonStateChangedDelegate OnButtonStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "OSVR")
	FAnalogValueChangedDelegate OnAnalogValueChanged;

public:
	void InitializeComponent() override;
	void UninitializeComponent() override;

private:
	// @TODO: OSVRInterfaceCollection::RegistrationToken should be used instead
	uint32 RegistrationToken;

	float WorldToMetersScale;
};
