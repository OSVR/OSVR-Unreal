
#include "OSVRPrivatePCH.h"
#include "OSVRBPFunctionLibrary.h"
#include "OSVRInterfaceCollection.h"
#include "OSVRInterface.h"
#include "OSVREntryPoint.h"
#include "IOSVR.h"
#include "OSVRHMD.h"


float GetWorldToMetersScale()
{
	float w2m = 100.f;
	UWorld * w = GEngine->GetWorld();
	if (w != nullptr)
	{
		AWorldSettings * ws = w->GetWorldSettings();
		if (ws != nullptr)
		{
			w2m = ws->WorldToMeters;
		}
	}
	return w2m;
}



FTransform UOSVRBPFunctionLibrary::GetInterfacePose(FName Name)
{
	auto Interface = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection()->GetOrCreateInterface(Name);
	if (Interface != nullptr)
	{
		FTransform Pose;
		if (Interface->GetPose(Pose, false))
		{
			Pose.ScaleTranslation(GetWorldToMetersScale());
			return Pose;
		}
	}

	return FTransform::Identity;
}

FRotator UOSVRBPFunctionLibrary::GetInterfaceRotation(FName Name)
{
	auto Interface = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection()->GetOrCreateInterface(Name);
	if (Interface != nullptr)
	{
		FQuat Rotation;
		if (Interface->GetOrientation(Rotation, false))
			return Rotation.Rotator();
	}

	return FRotator::ZeroRotator;
}

FVector UOSVRBPFunctionLibrary::GetInterfacePosition(FName Name)
{
	auto Interface = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection()->GetOrCreateInterface(Name);
	if (Interface != nullptr)
	{
		FVector Translation;
		if (Interface->GetPosition(Translation, false))
		{
			return Translation * GetWorldToMetersScale();
		}
	}

	return FVector::ZeroVector;
}

EButtonState::Type UOSVRBPFunctionLibrary::GetInterfaceButtonState(FName Name)
{
	auto Interface = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection()->GetOrCreateInterface(Name);
	if (Interface != nullptr)
	{
		uint8 Button;
		if (Interface->GetButtonState(Button, false))
			return EButtonState::Type(Button);
	}

	return EButtonState::Type(0);
}

float UOSVRBPFunctionLibrary::GetInterfaceAnalogValue(FName Name)
{
	auto Interface = IOSVR::Get().GetEntryPoint()->GetInterfaceCollection()->GetOrCreateInterface(Name);
	if (Interface != nullptr)
	{
		float Analog;
		if (Interface->GetAnalogState(Analog, false))
			return Analog;
	}

	return 0.0f;
}

void UOSVRBPFunctionLibrary::SetCurrentHmdOrientationAndPositionAsBase()
{
	if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D())
	{
		auto HMDDevice = StaticCastSharedPtr<FOSVRHMD>(GEngine->HMDDevice);
		if (HMDDevice.IsValid())
			HMDDevice->SetCurrentHmdOrientationAndPositionAsBase();
	}
}
