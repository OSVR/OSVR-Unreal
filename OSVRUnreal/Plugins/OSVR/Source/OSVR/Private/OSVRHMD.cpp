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
#include "SceneViewport.h"

#if WITH_EDITOR
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#endif

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#include <osvr/Util/ReturnCodesC.h>
#include <osvr/RenderKit/RenderManagerD3D11C.h>
#include "HideWindowsPlatformTypes.h"
#else
#include <osvr/RenderKit/RenderManagerOpenGLC.h>
#endif

#include <osvr/Util/MatrixConventionsC.h>
#include <cmath>
#include <vector>

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

// @todo: move this to OSVRHMDDescription
void FOSVRHMD::GetMonitorInfo(IHeadMountedDisplay::MonitorInfo& MonitorDesc) const {
    OSVR_ReturnCode returnCode;

    OSVR_DisplayDimension width, height;
    returnCode = osvrClientGetDisplayDimensions(DisplayConfig, 0, &width, &height);
    check(returnCode == OSVR_RETURN_SUCCESS);

    OSVR_ViewportDimension left, bottomIgnored, widthIgnored, heightIgnored;
    returnCode = osvrClientGetRelativeViewportForViewerEyeSurface(DisplayConfig, 0, 0, 0,
        &left, &bottomIgnored, &widthIgnored, &heightIgnored);
    check(returnCode == OSVR_RETURN_SUCCESS);

    MonitorDesc.MonitorName = "OSVR-Display"; //@TODO
    MonitorDesc.MonitorId = 0;				  //@TODO
    MonitorDesc.DesktopX = left;
    MonitorDesc.DesktopY = 0;
    MonitorDesc.ResolutionX = width;
    MonitorDesc.ResolutionY = height;
}

bool FOSVRHMD::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
    if (IsInitialized()
        && osvrClientCheckDisplayStartup(DisplayConfig) == OSVR_RETURN_SUCCESS)
    {
        GetMonitorInfo(MonitorDesc);
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

void FOSVRHMD::UpdateHeadPose() {
    OSVR_Pose3 pose;
    OSVR_ReturnCode returnCode;
    
    returnCode = osvrClientUpdate(osvrClientContext);
    check(returnCode == OSVR_RETURN_SUCCESS);

    returnCode = osvrClientGetViewerPose(DisplayConfig, 0, &pose);
    check(returnCode == OSVR_RETURN_SUCCESS);

    CurHmdPosition = BaseOrientation.Inverse().RotateVector((OSVR2FVector(pose.translation) * WorldToMetersScale) - BasePosition);
    CurHmdOrientation = BaseOrientation.Inverse() * OSVR2FQuat(pose.rotation);
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

bool FOSVRHMD::OnStartGameFrame(FWorldContext& WorldContext) {
    check(IsInGameThread());
    if (!bHmdOverridesApplied) {
        IConsoleManager::Get().FindConsoleVariable(TEXT("r.FinishCurrentFrame"))->Set(1);
        bHmdOverridesApplied = true;
    }
    return true;
}

float FOSVRHMD::GetInterpupillaryDistance() const
{
    return HMDDescription.GetInterpupillaryDistance();
}

void FOSVRHMD::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
    // intentionally left blank
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
    UpdateHeadPose();

    CurrentOrientation = LastHmdOrientation = CurHmdOrientation;
    CurrentPosition = CurHmdPosition;
}

void FOSVRHMD::ApplyHmdRotation(APlayerController* PC, FRotator& ViewRotation)
{
    ViewRotation.Normalize();

    UpdateHeadPose();

    LastHmdOrientation = CurHmdOrientation;

    const FRotator DeltaRot = ViewRotation - PC->GetControlRotation();
    DeltaControlRotation = (DeltaControlRotation + DeltaRot).GetNormalized();

    // Pitch from other sources is never good, because there is an absolute up and down that must be respected to avoid motion sickness.
    // Same with roll.
    DeltaControlRotation.Pitch = 0;
    DeltaControlRotation.Roll = 0;
    DeltaControlOrientation = DeltaControlRotation.Quaternion();

    ViewRotation = FRotator(DeltaControlOrientation * CurHmdOrientation);
}

void FOSVRHMD::UpdatePlayerCameraRotation(APlayerCameraManager* Camera, struct FMinimalViewInfo& POV)
{
    UpdateHeadPose();

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
            //OSVRInterfaceName = val;
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
        OSVR_ReturnCode ReturnCode = osvrClientGetDisplay(osvrClientContext, &DisplayConfig);
        if (ReturnCode == OSVR_RETURN_SUCCESS)
        {
            int numTries = 0;
            while (osvrClientCheckDisplayStartup(DisplayConfig) == OSVR_RETURN_FAILURE && numTries++ < 10000) {
                osvrClientUpdate(osvrClientContext);
            }
            if (osvrClientCheckDisplayStartup(DisplayConfig) == OSVR_RETURN_SUCCESS) {
                bHmdPosTracking = true;
            }
        }
    }
    else
    {
        if (DisplayConfig != nullptr)
        {
            OSVR_ReturnCode ReturnCode = osvrClientFreeDisplay(DisplayConfig);
            check(ReturnCode == OSVR_RETURN_SUCCESS);
            DisplayConfig = nullptr;
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

    FSystemResolution::RequestResolutionChange(1280, 720, EWindowMode::Windowed); // bStereo ? WindowedMirror : Windowed

    FSceneViewport* sceneViewport;
    if (!GIsEditor) {
        UGameEngine* gameEngine = Cast<UGameEngine>(GEngine);
        sceneViewport = gameEngine->SceneViewport.Get();
    }
#if WITH_EDITOR
    else {
        UEditorEngine* editorEngine = Cast<UEditorEngine>(GEngine);
        sceneViewport = (FSceneViewport*)(editorEngine->GetPIEViewport());
    }
#endif

    if (sceneViewport) {
        sceneViewport->SetViewportSize(1280, 720);
    }
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

    OSVR_PoseState Pose;
    OSVR_ReturnCode ReturnCode = osvrClientGetViewerPose(DisplayConfig, 0, &Pose);
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
    OSVR_PoseState Pose;
    OSVR_ReturnCode ReturnCode = osvrClientGetViewerPose(DisplayConfig, 0, &Pose);
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

namespace {
    static OSVR_MatrixConventions gMatrixFlags = OSVR_MATRIX_ROWMAJOR | OSVR_MATRIX_RHINPUT;

}
FMatrix FOSVRHMD::GetStereoProjectionMatrix(enum EStereoscopicPass StereoPassType, const float FOV) const
{
    FMatrix original = HMDDescription.GetProjectionMatrix(
        StereoPassType == eSSP_LEFT_EYE ? OSVRHMDDescription::LEFT_EYE : OSVRHMDDescription::RIGHT_EYE);

    // @todo we should be getting a matrix from core, but this doesn't appear to be working.
    //OSVR_EyeCount eye = 0;
    //if (StereoPassType == eSSP_LEFT_EYE) {
    //    eye = 0;
    //}
    //else if (StereoPassType == eSSP_RIGHT_EYE) {
    //    eye = 1;
    //}

    //float yUpMatrix[16];
    //if (osvrClientGetViewerEyeSurfaceProjectionMatrixf(DisplayConfig, 0, eye, 0, GNearClippingPlane, 10000.0f, gMatrixFlags, yUpMatrix) == OSVR_RETURN_FAILURE) {
    //    throw std::exception("FOSVRHMD::GetStereoProjectionMatrix: couldn't get the viewer eye matrix.");
    //}
    //FMatrix ret = OSVR2FMatrix(yUpMatrix);
    //FMatrix axisChange = FMatrix(
    //    FPlane(1, 0, 0, 0),
    //    FPlane(0, 0, 1, 0),
    //    FPlane(0, 1, 0, 0),
    //    FPlane(0, 0, 0, 1));

    //// @todo do a change of basis here? Seems to make it worse??
    //FMatrix preMultiply = axisChange * ret;
    //FMatrix postMultiply = ret * axisChange;
    return original;
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
    //InViewFamily.EngineShowFlags.ScreenPercentage = 1.0f;
    InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();
}

void FOSVRHMD::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
    InView.BaseHmdOrientation = FQuat(FRotator(0.0f, 0.0f, 0.0f));
    InView.BaseHmdLocation = FVector(0.f);
    WorldToMetersScale = InView.WorldToMetersScale;
    InViewFamily.bUseSeparateRenderTarget = true;
}

bool FOSVRHMD::IsHeadTrackingAllowed() const
{
    return GEngine->IsStereoscopic3D();
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
    bHmdOverridesApplied(false),
    DisplayConfig(nullptr)
{
    FSystemResolution::RequestResolutionChange(1280, 720, EWindowMode::Windowed); // bStereo ? WindowedMirror : Windowed

    EnablePositionalTracking(true);
    HMDDescription.Init(osvrClientContext, DisplayConfig);
#if PLATFORM_WINDOWS
    if (!GIsEditor && IsPCPlatform(GMaxRHIShaderPlatform) && !IsOpenGLPlatform(GMaxRHIShaderPlatform)) {
        mCustomPresent = std::make_shared<FCurrentCustomPresent>(osvrClientContext);
    }
#endif

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
