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

#include "IOSVR.h"
#include "OSVRHMDDescription.h"
#include "HeadMountedDisplay.h"
#include "IHeadMountedDisplay.h"
#include "SceneViewExtension.h"
#include "SceneView.h"
#include "ShowFlags.h"

#include <osvr/ClientKit/DisplayC.h>
#include <osvr/ClientKit/InterfaceC.h>
#include "OSVRCustomPresent.h"

DECLARE_LOG_CATEGORY_EXTERN(OSVRHMDLog, Log, All);

/**
* OSVR Head Mounted Display
*/
class FOSVRHMD : public IHeadMountedDisplay, public ISceneViewExtension, public TSharedFromThis< FOSVRHMD, ESPMode::ThreadSafe >
{
public:

#if OSVR_UNREAL_4_12
    virtual void OnBeginPlay(FWorldContext& InWorldContext) override;
    virtual void OnEndPlay(FWorldContext& InWorldContext) override;
#else
    virtual void OnBeginPlay() override;
    virtual void OnEndPlay() override;
#endif
    virtual bool IsHMDConnected() override;
    virtual bool IsHMDEnabled() const override;
    virtual void EnableHMD(bool bEnable = true) override;
    virtual EHMDDeviceType::Type GetHMDDeviceType() const override;
    virtual FName GetDeviceName() const override;
    virtual bool GetHMDMonitorInfo(MonitorInfo&) override;

    virtual void GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const override;

    virtual bool DoesSupportPositionalTracking() const override;
    virtual bool HasValidTrackingPosition() override;
    virtual void GetPositionalTrackingCameraProperties(FVector& OutOrigin, FQuat& OutOrientation, float& OutHFOV, float& OutVFOV, float& OutCameraDistance, float& OutNearPlane, float& OutFarPlane) const override;

    virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
    virtual float GetInterpupillaryDistance() const override;

    virtual void GetCurrentOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition) override;

    /**
    * Rebase the input position and orientation to that of the HMD's base
    */
    virtual void RebaseObjectOrientationAndPosition(FVector& Position, FQuat& Orientation) const override;

    virtual TSharedPtr< class ISceneViewExtension, ESPMode::ThreadSafe > GetViewExtension() override;
    virtual void ApplyHmdRotation(APlayerController* PC, FRotator& ViewRotation) override;

#if OSVR_UNREAL_4_11
    virtual bool UpdatePlayerCamera(FQuat& CurrentOrientation, FVector& CurrentPosition) override;
#else
    virtual void UpdatePlayerCameraRotation(class APlayerCameraManager* Camera, struct FMinimalViewInfo& POV) override;
#endif

    virtual bool IsChromaAbCorrectionEnabled() const override;

    virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
#if !OSVR_UNREAL_4_12
    virtual void OnScreenModeChange(EWindowMode::Type WindowMode) override;
#endif

    void SetTrackingOrigin(EHMDTrackingOrigin::Type InOrigin) override;
    EHMDTrackingOrigin::Type GetTrackingOrigin() override;

    virtual bool IsPositionalTrackingEnabled() const override;
    virtual bool EnablePositionalTracking(bool bEnable) override;

    virtual bool IsHeadTrackingAllowed() const override;

    virtual bool IsInLowPersistenceMode() const override;
    virtual void EnableLowPersistenceMode(bool bEnable = true) override;
    virtual bool OnStartGameFrame(FWorldContext& WorldContext) override;

    // seen in simplehmd
    virtual void SetClippingPlanes(float NCP, float FCP) override;

    virtual void SetBaseRotation(const FRotator& BaseRot) override;
    virtual FRotator GetBaseRotation() const override;

    virtual void SetBaseOrientation(const FQuat& BaseOrient) override;
    virtual FQuat GetBaseOrientation() const override;

    virtual void DrawDistortionMesh_RenderThread(struct FRenderingCompositePassContext& Context, const FIntPoint& TextureSize) override;

    /** IStereoRendering interface */
    virtual bool IsStereoEnabled() const override;
    virtual bool EnableStereo(bool bStereo = true) override;
    virtual void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
    virtual void CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, const FRotator& ViewRotation,
        const float MetersToWorld, FVector& ViewLocation) override;
    virtual FMatrix GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType, const float FOV) const override;
    virtual void InitCanvasFromView(FSceneView* InView, UCanvas* Canvas) override;
    virtual void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIParamRef BackBuffer, FTexture2DRHIParamRef SrcTexture) const override;
    virtual void GetEyeRenderParams_RenderThread(const struct FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
    virtual void CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) override;
    virtual bool NeedReAllocateViewportRenderTarget(const FViewport &viewport) override;
    virtual void UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, class SViewport*) override;
    virtual bool AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples = 1) override;

    virtual bool ShouldUseSeparateRenderTarget() const override
    {
        check(IsInGameThread());
        return IsStereoEnabled();
    }

    /** ISceneViewExtension interface */
    virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
    virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
    virtual FRHICustomPresent* GetCustomPresent() override
    {
        return mCustomPresent;
    }

    virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
    {
    }
    virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
    virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;

    /** Resets orientation by setting roll and pitch to 0,
    assuming that current yaw is forward direction and assuming
    current position as 0 point. */
    virtual void ResetOrientation(float yaw) override;
    void ResetOrientation(bool bAdjustOrientation, float yaw);
    virtual void ResetPosition() override;
    virtual void ResetOrientationAndPosition(float yaw = 0.f) override;

    inline float GetWorldToMetersScale()
    {
        return WorldToMetersScale;
    }

public:
    /** Constructor */
    FOSVRHMD(TSharedPtr<class OSVREntryPoint, ESPMode::ThreadSafe> entryPoint);

    /** Destructor */
    virtual ~FOSVRHMD();

    /** @return	True if the HMD was initialized OK */
    bool IsInitialized() const;

    OSVR_API FVector GetTrackingOriginOffset();

private:
    void UpdateHeadPose(bool renderThread, FQuat& lastHmdOrientation, FVector& lastHmdPosition, FQuat& hmdOrientation, FVector& hmdPosition);
    void StartCustomPresent();
    void StopCustomPresent();
    void GetRenderTargetSize_GameThread(float windowWidth, float windowHeight, float &width, float &height) const;
    float GetScreenScale() const;
    bool GetHMDSupportsPositionalTracking();

    TSharedPtr<class OSVREntryPoint, ESPMode::ThreadSafe> mOSVREntryPoint;
    IRendererModule* RendererModule = nullptr;

    /** Player's orientation tracking */
    FRotator DeltaControlRotation = FRotator::ZeroRotator; // same as DeltaControlOrientation but as rotator
    FQuat DeltaControlOrientation = FQuat::Identity; // same as DeltaControlRotation but as quat

    // head tracking on game thread
    mutable FQuat CurHmdOrientation = FQuat::Identity;
    mutable FVector CurHmdPosition = FVector::ZeroVector;
    mutable FQuat LastHmdOrientation = FQuat::Identity;
    mutable FVector LastHmdPosition = FVector::ZeroVector;

    // head tracking on render thread
    mutable FQuat CurHmdOrientationRT = FQuat::Identity;
    mutable FVector CurHmdPositionRT = FVector::ZeroVector;
    mutable FQuat LastHmdOrientationRT = FQuat::Identity;
    mutable FVector LastHmdPositionRT = FVector::ZeroVector;

    /** HMD base values, specify forward orientation and zero pos offset */
    FQuat BaseOrientation = FQuat::Identity; // base orientation
    FVector BasePosition = FVector::ZeroVector;

    /** World units (UU) to Meters scale.  Read from the level, and used to transform positional tracking data */
    float WorldToMetersScale = 100.0f; // @todo: isn't this meters to world units scale?
    float StandingHeightInMeters = 1.64592f; // @todo: can we get this information from OSVR-Core somehow?

    bool bHmdPosTracking = false;
    bool bHaveVisionTracking = false;

    bool bNewStereoEnabled = false;
    bool bStereoEnabled = false;
    bool bHmdEnabled = false;
    bool bHmdConnected = false;
    bool bHmdOverridesApplied = false;
    bool bWaitedForClientStatus = false;
    bool bPlaying = false;
    bool bHmdHadPositionalState = false;

    EHMDTrackingOrigin::Type TrackingOrigin = EHMDTrackingOrigin::Floor;

    OSVR_ClientInterface mHmdInterface;
    OSVRHMDDescription HMDDescription;
    OSVR_DisplayConfig DisplayConfig = nullptr;
    TRefCountPtr<FOSVRCustomPresent> mCustomPresent;
};
