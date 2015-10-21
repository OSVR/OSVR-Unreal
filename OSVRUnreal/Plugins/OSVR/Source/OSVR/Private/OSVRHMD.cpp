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
#include "OSVRHMD.h"

#include "OSVRTypes.h"

#include "SharedPointer.h"

extern OSVR_ClientContext osvrClientContext;

//---------------------------------------------------
// IHeadMountedDisplay Implementation
//---------------------------------------------------

bool FOSVRHMD::IsHMDConnected()
{
	// @TODO: we need a hook in OSVR
	return true;
}

bool FOSVRHMD::IsHMDEnabled() const
{
	return bHmdEnabled;
}

void FOSVRHMD::EnableHMD(bool enable)
{
	bHmdEnabled = enable;

	if (!bHmdEnabled)
	{
		EnableStereo(false);
	}
}

EHMDDeviceType::Type FOSVRHMD::GetHMDDeviceType() const
{
	return EHMDDeviceType::DT_ES2GenericStereoMesh;
}

bool FOSVRHMD::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
	if (IsInitialized() && HMDDescription.IsValid())
	{
		HMDDescription.GetMonitorInfo(MonitorDesc);
		return true;
	}
	else
	{
		MonitorDesc.MonitorName = "";
		MonitorDesc.MonitorId = 0;
		MonitorDesc.DesktopX = MonitorDesc.DesktopY = MonitorDesc.ResolutionX = MonitorDesc.ResolutionY = 0;
	}

	return false;
}

bool FOSVRHMD::DoesSupportPositionalTracking() const
{
	return true;
}

bool FOSVRHMD::HasValidTrackingPosition()
{
	return bHmdPosTracking && bHaveVisionTracking;
}

void FOSVRHMD::GetPositionalTrackingCameraProperties(FVector& OutOrigin, FQuat& OutOrientation,
													 float& OutHFOV, float& OutVFOV, float& OutCameraDistance, float& OutNearPlane, float& OutFarPlane) const
{
	// @TODO
}

bool FOSVRHMD::IsInLowPersistenceMode() const
{
	// @TODO
	return true;
}

void FOSVRHMD::EnableLowPersistenceMode(bool Enable)
{
	// @TODO
}

float FOSVRHMD::GetInterpupillaryDistance() const
{
	return HMDDescription.GetInterpupillaryDistance();
}

void FOSVRHMD::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
	// @TODO
}

void FOSVRHMD::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
	FVector2D FOVs = HMDDescription.GetFov(OSVRHMDDescription::LEFT_EYE);

	OutHFOVInDegrees = FOVs.X;
	OutVFOVInDegrees = FOVs.Y;
}

void FOSVRHMD::GetCurrentOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition)
{
	checkf(IsInGameThread(), TEXT("Orientation and position failed IsInGameThread test"));

	CurrentOrientation = LastHmdOrientation = CurHmdOrientation;
	CurrentPosition = CurHmdPosition;
}

void FOSVRHMD::ApplyHmdRotation(APlayerController* PC, FRotator& ViewRotation)
{
	ViewRotation.Normalize();

	LastHmdOrientation = CurHmdOrientation;

	const FRotator DeltaRot = ViewRotation - PC->GetControlRotation();
	DeltaControlRotation = (DeltaControlRotation + DeltaRot).GetNormalized();

	// Pitch from other sources is never good, because there is an absolute up and down that must be respected to avoid motion sickness.
	// Same with roll.
	//DeltaControlRotation.Pitch = 0;
	//DeltaControlRotation.Roll = 0;
	DeltaControlOrientation = DeltaControlRotation.Quaternion();

	ViewRotation = FRotator(DeltaControlOrientation * CurHmdOrientation);
}

void FOSVRHMD::UpdatePlayerCameraRotation(APlayerCameraManager* Camera, struct FMinimalViewInfo& POV)
{
	LastHmdOrientation = CurHmdOrientation;

	DeltaControlRotation = POV.Rotation;
	DeltaControlOrientation = DeltaControlRotation.Quaternion();

	// Apply HMD orientation to camera rotation.
	POV.Rotation = FRotator(POV.Rotation.Quaternion() * CurHmdOrientation);
}

bool FOSVRHMD::IsChromaAbCorrectionEnabled() const
{
	// @TODO
	return false;
}

TSharedPtr< class ISceneViewExtension, ESPMode::ThreadSafe > FOSVRHMD::GetViewExtension()
{
	TSharedPtr< FOSVRHMD, ESPMode::ThreadSafe > ptr(AsShared());
	return StaticCastSharedPtr< ISceneViewExtension >(ptr);
}

bool FOSVRHMD::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FParse::Command(&Cmd, TEXT("STEREO")))
	{
		if (FParse::Command(&Cmd, TEXT("ON")))
		{
			if (!IsHMDEnabled())
			{
				Ar.Logf(TEXT("HMD is disabled. Use 'hmd enable' to re-enable it."));
			}
			EnableStereo(true);
			return true;
		}
		else if (FParse::Command(&Cmd, TEXT("OFF")))
		{
			EnableStereo(false);
			return true;
		}
	}
	else if (FParse::Command(&Cmd, TEXT("HMD")))
	{
		if (FParse::Command(&Cmd, TEXT("ENABLE")))
		{
			EnableHMD(true);
			return true;
		}
		else if (FParse::Command(&Cmd, TEXT("DISABLE")))
		{
			EnableHMD(false);
			return true;
		}
	}
	else if (FParse::Command(&Cmd, TEXT("UNCAPFPS")))
	{
		GEngine->bSmoothFrameRate = false;
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("HEADTRACKING")))
	{
		FString val;
		if (FParse::Value(Cmd, TEXT("SOURCE="), val))
		{
			EnablePositionalTracking(false);
			OSVRInterfaceName = val;
			EnablePositionalTracking(true);
		}
		if (FParse::Command(&Cmd, TEXT("ENABLE")))
		{
			EnablePositionalTracking(true);
			return true;
		}
		else if (FParse::Command(&Cmd, TEXT("DISABLE")))
		{
			EnablePositionalTracking(false);
			return true;
		}
	}

	return false;
}

void FOSVRHMD::OnScreenModeChange(EWindowMode::Type WindowMode)
{
	EnableStereo(WindowMode != EWindowMode::Windowed);
}

bool FOSVRHMD::IsPositionalTrackingEnabled() const
{
	return bHmdPosTracking;
}

bool FOSVRHMD::EnablePositionalTracking(bool enable)
{
	if (enable && !IsPositionalTrackingEnabled())
	{
		OSVR_ReturnCode ReturnCode = osvrClientGetInterface(osvrClientContext, TCHAR_TO_ANSI(*OSVRInterfaceName), &OSVRClientInterface);

		if (ReturnCode == OSVR_RETURN_SUCCESS)
		{
			ReturnCode = osvrRegisterPoseCallback(OSVRClientInterface, &OSVRPoseCallback, this);

			bHmdPosTracking = ReturnCode == OSVR_RETURN_SUCCESS;
		}
	}
	else
	{
		if (OSVRClientInterface != nullptr)
		{
			OSVR_ReturnCode ReturnCode = osvrClientFreeInterface(osvrClientContext, OSVRClientInterface);
			check(ReturnCode == OSVR_RETURN_SUCCESS);

			//@TODO: unregister pose callback but this call is missing in the OSVR API !

			OSVRClientInterface = nullptr;
		}

		bHmdPosTracking = false;
	}

	return IsPositionalTrackingEnabled();
}

//---------------------------------------------------
// IStereoRendering Implementation
//---------------------------------------------------

bool FOSVRHMD::IsStereoEnabled() const
{
	return bStereoEnabled && bHmdEnabled;
}

bool FOSVRHMD::EnableStereo(bool stereo)
{
	bStereoEnabled = (IsHMDEnabled()) ? stereo : false;
	return bStereoEnabled;
}

void FOSVRHMD::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{
	SizeX = SizeX / 2;
	if (StereoPass == eSSP_RIGHT_EYE)
	{
		X += SizeX;
	}
}

void FOSVRHMD::CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, const FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation)
{
	if (StereoPassType != eSSP_FULL)
	{
		float EyeOffset = (GetInterpupillaryDistance() * WorldToMeters) / 2.0f;
		const float PassOffset = (StereoPassType == eSSP_LEFT_EYE) ? EyeOffset : -EyeOffset;
		ViewLocation += ViewRotation.Quaternion().RotateVector(FVector(0, PassOffset, 0));

		const FVector vHMDPosition = DeltaControlOrientation.RotateVector(CurHmdPosition);
		ViewLocation += vHMDPosition;
		LastHmdPosition = CurHmdPosition;
	}
}

void FOSVRHMD::ResetOrientationAndPosition(float yaw)
{
	ResetOrientation(yaw);
	ResetPosition();
}

void FOSVRHMD::ResetOrientation(float yaw)
{
	ResetOrientation(true, yaw);
}

void FOSVRHMD::ResetOrientation(bool adjustOrientation, float yaw)
{
	FQuat CurrentRotation(FQuat::Identity);

    OSVR_TimeValue Time;
	OSVR_PoseState Pose;
	OSVR_ReturnCode ReturnCode = osvrGetPoseState(OSVRClientInterface, &Time, &Pose);
	if (ReturnCode != OSVR_RETURN_SUCCESS)
		return;

	CurrentRotation = OSVR2FQuat(Pose.rotation);

	if (adjustOrientation)
	{
		FRotator ViewRotation;
		ViewRotation = FRotator(CurrentRotation);
		ViewRotation.Pitch = 0;
		ViewRotation.Roll = 0;

		if (yaw != 0.f)
		{
			// apply optional yaw offset
			ViewRotation.Yaw -= yaw;
			ViewRotation.Normalize();
		}

		BaseOrientation = ViewRotation.Quaternion();
	}
	else
	{
		BaseOrientation = CurrentRotation;
	}
}

void FOSVRHMD::ResetPosition()
{
	FVector CurrentPosition(FVector::ZeroVector);
	OSVR_TimeValue Time;
	OSVR_PoseState Pose;
	OSVR_ReturnCode ReturnCode = osvrGetPoseState(OSVRClientInterface, &Time, &Pose);
	if (ReturnCode != OSVR_RETURN_SUCCESS)
		return;

	CurrentPosition = OSVR2FVector(Pose.translation);

	// Reset position
	BasePosition = CurrentPosition * WorldToMetersScale;
}

void FOSVRHMD::SetCurrentHmdOrientationAndPositionAsBase()
{
	ResetPosition();
	ResetOrientation(false, 0);
}

FMatrix FOSVRHMD::GetStereoProjectionMatrix(enum EStereoscopicPass StereoPassType, const float FOV) const
{
	return HMDDescription.GetProjectionMatrix(StereoPassType == eSSP_LEFT_EYE
												  ? OSVRHMDDescription::LEFT_EYE
												  : OSVRHMDDescription::RIGHT_EYE);
}

void FOSVRHMD::InitCanvasFromView(FSceneView* InView, UCanvas* Canvas)
{
	// @TODO
}

/*void FOSVRHMD::PushViewportCanvas(EStereoscopicPass StereoPass, FCanvas* InCanvas, UCanvas* InCanvasObject, FViewport* InViewport) const
{
	FMatrix m;
	m.SetIdentity();
	InCanvas->PushAbsoluteTransform(m);
}

void FOSVRHMD::PushViewCanvas(EStereoscopicPass StereoPass, FCanvas* InCanvas, UCanvas* InCanvasObject, FSceneView* InView) const
{
	FMatrix m;
	m.SetIdentity();
	InCanvas->PushAbsoluteTransform(m);
}*/

//---------------------------------------------------
// ISceneViewExtension Implementation
//---------------------------------------------------

void FOSVRHMD::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	InViewFamily.EngineShowFlags.MotionBlur = 0;
	InViewFamily.EngineShowFlags.HMDDistortion = false;
	InViewFamily.EngineShowFlags.ScreenPercentage = 1.0f;
	InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();
}

void FOSVRHMD::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	InView.BaseHmdOrientation = FQuat(FRotator(0.0f, 0.0f, 0.0f));
	InView.BaseHmdLocation = FVector(0.f);
	WorldToMetersScale = InView.WorldToMetersScale;
	InViewFamily.bUseSeparateRenderTarget = false;
}

bool FOSVRHMD::IsHeadTrackingAllowed() const
{
	return GEngine->IsStereoscopic3D();
}

static void OSVRPoseCallback(void* Userdata, const OSVR_TimeValue* /*Timestamp*/, const OSVR_PoseReport* Report)
{
	auto This = reinterpret_cast< FOSVRHMD* >(Userdata);
	if (This && Report)
	{
		This->CurHmdPosition = This->BaseOrientation.Inverse().RotateVector((OSVR2FVector(Report->pose.translation) * This->WorldToMetersScale) - This->BasePosition);
		This->CurHmdOrientation = This->BaseOrientation.Inverse() * OSVR2FQuat(Report->pose.rotation);

		This->bHaveVisionTracking = true;
	}
}

FOSVRHMD::FOSVRHMD()
	: LastHmdOrientation(FQuat::Identity),
	  CurHmdOrientation(FQuat::Identity),
	  DeltaControlRotation(FRotator::ZeroRotator),
	  DeltaControlOrientation(FQuat::Identity),
	  CurHmdPosition(FVector::ZeroVector),
	  BaseOrientation(FQuat::Identity),
	  BasePosition(FVector::ZeroVector),
	  WorldToMetersScale(100.0f),
	  bHmdPosTracking(false),
	  bHaveVisionTracking(false),
	  bStereoEnabled(true),
	  bHmdEnabled(true),
	  OSVRClientInterface(nullptr),
	  OSVRInterfaceName("/me/head")
{
	HMDDescription.Init(osvrClientContext);
	OSVRInterfaceName = HMDDescription.GetPositionalTrackerInterface(OSVRHMDDescription::LEFT_EYE);

	EnablePositionalTracking(true);

	// enable vsync
	IConsoleVariable* CVSyncVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"));
	if (CVSyncVar)
		CVSyncVar->Set(true);

	// Uncap fps to enable FPS higher than 62
	GEngine->bSmoothFrameRate = false;
}

FOSVRHMD::~FOSVRHMD()
{
	EnablePositionalTracking(false);
}

bool FOSVRHMD::IsInitialized() const
{
	// @TODO
	return true;
}
