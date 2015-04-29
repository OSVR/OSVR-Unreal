// Fill out your copyright notice in the Description page of Project Settings.

#include "OSVRPrivatePCH.h"
#include "OSVRInputComponent.h"

#include "IOSVR.h"
#include "OSVREntryPoint.h"
#include "OSVRInterfaceCollection.h"

UOSVRInputComponent::UOSVRInputComponent(const class FObjectInitializer& PCIP)
	: UActorComponent(PCIP)
{
	bWantsInitializeComponent = true;
}

void UOSVRInputComponent::InitializeComponent()
{
	Super::InitializeComponent();

	WorldToMetersScale = 100.f;
	UWorld* w = GetWorld();
	if (w != nullptr)
	{
		AWorldSettings* ws = w->GetWorldSettings();
		if (ws != nullptr)
		{
			WorldToMetersScale = ws->WorldToMeters;
		}
	}

	auto InterfaceCollection = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection();

	OSVRInterfaceCollection::RegistrationToken RegToken =
		InterfaceCollection->RegisterOnStateChangedCallback(
			[=](OSVRInterface* Interface, uint32 State)
			{
		/*
		FTransform Pose;
		if (((State & OSVRInterface::POSE_STATE_AVAILABLE) > 0) && Interface->GetPose(Pose, false))
		{
			Pose.ScaleTranslation(WorldToMetersScale);
			OnPoseChanged.Broadcast(Interface->GetName(), Pose);
		}
		*/

		FVector Position;
		if (((State & OSVRInterface::POSITION_STATE_AVAILABLE) > 0) && Interface->GetPosition(Position, false))
			OnPositionChanged.Broadcast(Interface->GetName(), Position * WorldToMetersScale);

		FQuat Orientation;
		if (((State & OSVRInterface::ORIENTATION_STATE_AVAILABLE) > 0) && Interface->GetOrientation(Orientation, false))
			OnOrientationChanged.Broadcast(Interface->GetName(), Orientation.Rotator());

		float Analog;
		if (((State & OSVRInterface::ANALOG_STATE_AVAILABLE) > 0) && Interface->GetAnalogState(Analog, false))
			OnAnalogValueChanged.Broadcast(Interface->GetName(), Analog);

		uint8 Button;
		if (((State & OSVRInterface::BUTTON_STATE_AVAILABLE) > 0) && Interface->GetButtonState(Button, false))
			OnButtonStateChanged.Broadcast(Interface->GetName(), EButtonState::Type(Button));
			});

	RegistrationToken = RegToken.Token;
}

void UOSVRInputComponent::UninitializeComponent()
{
	auto InterfaceCollection = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection();

	InterfaceCollection->DeregisterOnStateChangedCallback(OSVRInterfaceCollection::RegistrationToken(RegistrationToken));

	Super::UninitializeComponent();
}
