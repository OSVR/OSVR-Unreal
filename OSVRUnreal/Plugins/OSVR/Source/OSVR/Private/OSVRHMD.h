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

/**
 * OSVR Head Mounted Display
 */
class FOSVRHMD : public IHeadMountedDisplay, public ISceneViewExtension, public TSharedFromThis< FOSVRHMD, ESPMode::ThreadSafe >
{
public:
	/** IHeadMountedDisplay interface */
	virtual bool IsHMDConnected() override;
	virtual bool IsHMDEnabled() const override;
	virtual void EnableHMD(bool allow = true) override;
	virtual EHMDDeviceType::Type GetHMDDeviceType() const override;
	virtual bool GetHMDMonitorInfo(MonitorInfo&) override;

	virtual void GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const override;

	virtual bool DoesSupportPositionalTracking() const override;
	virtual bool HasValidTrackingPosition() override;
	virtual void GetPositionalTrackingCameraProperties(FVector& OutOrigin, FQuat& OutOrientation, float& OutHFOV, float& OutVFOV, float& OutCameraDistance, float& OutNearPlane, float& OutFarPlane) const override;

	virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
	virtual float GetInterpupillaryDistance() const override;

	virtual void GetCurrentOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition) override;
	virtual TSharedPtr< class ISceneViewExtension, ESPMode::ThreadSafe > GetViewExtension() override;
	virtual void ApplyHmdRotation(APlayerController* PC, FRotator& ViewRotation) override;
	virtual void UpdatePlayerCameraRotation(class APlayerCameraManager* Camera, struct FMinimalViewInfo& POV) override;

	virtual bool IsChromaAbCorrectionEnabled() const override;

	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void OnScreenModeChange(EWindowMode::Type WindowMode) override;

	virtual bool IsPositionalTrackingEnabled() const override;
	virtual bool EnablePositionalTracking(bool enable) override;

	virtual bool IsHeadTrackingAllowed() const override;

	virtual bool IsInLowPersistenceMode() const override;
	virtual void EnableLowPersistenceMode(bool Enable = true) override;
    virtual bool OnStartGameFrame(FWorldContext& WorldContext) override;

#if 0
    // seen in simplehmd
	virtual void SetClippingPlanes(float NCP, float FCP) override;

	virtual void SetBaseRotation(const FRotator& BaseRot) override;
	virtual FRotator GetBaseRotation() const override;

	virtual void SetBaseOrientation(const FQuat& BaseOrient) override;
	virtual FQuat GetBaseOrientation() const override;
#endif

	virtual void DrawDistortionMesh_RenderThread(struct FRenderingCompositePassContext& Context, const FIntPoint& TextureSize) override;

	/** IStereoRendering interface */
	virtual bool IsStereoEnabled() const override;
	virtual bool EnableStereo(bool stereo = true) override;
	virtual void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
	virtual void CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, const FRotator& ViewRotation,
										   const float MetersToWorld, FVector& ViewLocation) override;
	virtual FMatrix GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType, const float FOV) const override;
	virtual void InitCanvasFromView(FSceneView* InView, UCanvas* Canvas) override;
	virtual void GetEyeRenderParams_RenderThread(const struct FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
	virtual void GetTimewarpMatrices_RenderThread(const struct FRenderingCompositePassContext& Context, FMatrix& EyeRotationStart, FMatrix& EyeRotationEnd) const override;

	virtual void UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, class SViewport*) override;

	virtual bool ShouldUseSeparateRenderTarget() const override
	{
		return false;
	}

	/** ISceneViewExtension interface */
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
	{
	}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;

	/** Resets orientation by setting roll and pitch to 0, 
	    assuming that current yaw is forward direction and assuming
		current position as 0 point. */
	virtual void ResetOrientation(float yaw) override;
	void ResetOrientation(bool adjustOrientation, float yaw);
	virtual void ResetPosition() override;
	virtual void ResetOrientationAndPosition(float yaw = 0.f) override;
	void SetCurrentHmdOrientationAndPositionAsBase();

public:
	/** Constructor */
	FOSVRHMD();

	/** Destructor */
	virtual ~FOSVRHMD();

	/** @return	True if the HMD was initialized OK */
	bool IsInitialized() const;

private:
	/** Player's orientation tracking */
	mutable FQuat CurHmdOrientation;

	FRotator DeltaControlRotation; // same as DeltaControlOrientation but as rotator
	FQuat DeltaControlOrientation; // same as DeltaControlRotation but as quat

	mutable FVector CurHmdPosition;

	mutable FQuat LastHmdOrientation; // contains last APPLIED ON GT HMD orientation
	FVector LastHmdPosition;		  // contains last APPLIED ON GT HMD position

	/** HMD base values, specify forward orientation and zero pos offset */
	FQuat BaseOrientation; // base orientation
	FVector BasePosition;

	/** World units (UU) to Meters scale.  Read from the level, and used to transform positional tracking data */
	float WorldToMetersScale;

	bool bHmdPosTracking;
	bool bHaveVisionTracking;

	bool bStereoEnabled;
	bool bHmdEnabled;
    bool bHmdOverridesApplied;
	friend static void OSVRPoseCallback(void* Userdata, const OSVR_TimeValue* /*Timestamp*/, const OSVR_PoseReport* Report);

	OSVRHMDDescription HMDDescription;

	OSVR_ClientInterface OSVRClientInterface;

	FString OSVRInterfaceName;
};
