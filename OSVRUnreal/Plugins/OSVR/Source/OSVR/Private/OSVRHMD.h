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


#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#include <osvr/RenderKit/RenderManager.h>
#include <d3d11.h>
#include <osvr/RenderKit/GraphicsLibraryD3D11.h>
#include "HideWindowsPlatformTypes.h"
#else
#include <osvr/RenderKit/RenderManager.h>
#include <osvr/RenderKit/GraphicsLibraryOpenGL.h>
#endif

#include <memory>
#include <iostream>
#include <set>

//class ID3D11Device;
//class ID3D11DeviceContext;

template<class TGraphicsDevice>
class FOSVRCustomPresent : public FRHICustomPresent
{
public:
    FOSVRCustomPresent(OSVR_ClientContext clientContext) :
        FRHICustomPresent(nullptr)//,
        //mClientContext(clientContext)
    {
        mClientContext = osvrClientInit("com.osvr.unreal.plugin");
    }

    virtual ~FOSVRCustomPresent() {
        if (mClientContext) {
            osvrClientShutdown(mClientContext);
        }
    }

    // virtual methods from FRHICustomPresent

    virtual void OnBackBufferResize() override {}

    virtual bool Present(int32 &inOutSyncInterval) override {
        check(IsInRenderingThread());
        FinishRendering();
        return true;
    }

    // implement this in the sub-class
    virtual void Initialize() {
        if (!IsInitialized()) {
            auto graphicsLibrary = CreateGraphicsLibrary();
            auto graphicsLibraryName = GetGraphicsLibraryName();

            check(mClientContext);

            osvr::renderkit::RenderManager *renderManager = osvr::renderkit::createRenderManager(mClientContext, graphicsLibraryName, graphicsLibrary);
            //osvr::renderkit::RenderManager *renderManager = osvr::renderkit::createRenderManager(mClientContext, graphicsLibraryName);
            check(renderManager && renderManager->doingOkay());

            mRenderManager.reset(renderManager);
            check(mRenderManager)
            auto results = mRenderManager->OpenDisplay();
            check(results.status != osvr::renderkit::RenderManager::OpenStatus::FAILURE);

            // @todo: create the textures

            mInitialized = true;
        }
    }

    virtual bool IsInitialized() {
        return mInitialized;
    }

    virtual void UpdateViewport(const FViewport& InViewport, class FRHIViewport* InViewportRHI) = 0;

    // RenderManager normalizes displays a bit. We create the render target assuming horizontal side-by-side.
    // RenderManager then rotates that render texture if needed for vertical side-by-side displays.
    virtual void CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) {
        check(IsInGameThread());
        Initialize();
        // Should we create a RenderParams?
        auto renderInfo = mRenderManager->GetRenderInfo();

        // check some assumptions. Should all be the same height.
        check(renderInfo.size() == 2);
        check(renderInfo[0].viewport.height == renderInfo[1].viewport.height);
        InOutSizeX = renderInfo[0].viewport.width + renderInfo[1].viewport.width;
        InOutSizeY = renderInfo[0].viewport.height;
        check(InOutSizeX != 0 && InOutSizeY != 0);
    }

protected:
    virtual TGraphicsDevice* GetGraphicsDevice() {
        auto ret = RHIGetNativeDevice();
        return reinterpret_cast<TGraphicsDevice*>(ret);
    }

    virtual void FinishRendering()
    {
        Initialize();
        // all of the render manager samples keep the flipY at the default false,
        // for both OpenGL and DirectX. Is this even needed anymore?
        mRenderManager->PresentRenderBuffers(mRenderBuffers, mViewportDescriptions);// , ShouldFlipY());
    }

    // abstract methods, implement in DirectX/OpenGL specific subclasses
    virtual osvr::renderkit::GraphicsLibrary CreateGraphicsLibrary() = 0;
    virtual std::string GetGraphicsLibraryName() = 0;
    virtual bool ShouldFlipY() = 0;
    virtual bool AllocateRenderTargetTexture(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 InFlags, uint32 TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples) = 0;

    std::vector<osvr::renderkit::RenderBuffer> mRenderBuffers;
    std::vector<osvr::renderkit::OSVR_ViewportDescription> mViewportDescriptions;

    bool mInitialized = false;
    OSVR_ClientContext mClientContext = nullptr;
    std::shared_ptr<osvr::renderkit::RenderManager> mRenderManager = nullptr;
};

#if PLATFORM_WINDOWS

class FCurrentCustomPresent : public FOSVRCustomPresent<ID3D11Device>
{
public:
    FCurrentCustomPresent(OSVR_ClientContext clientContext) :
        FOSVRCustomPresent(clientContext)
    {}

    virtual void UpdateViewport(const FViewport& InViewport, class FRHIViewport* InViewportRHI) override {
        check(IsInGameThread());
        check(InViewportRHI);
        const FTexture2DRHIRef& rt = InViewport.GetRenderTargetTexture();
        check(IsValidRef(rt));
        Initialize();
        if (RenderTargetTexture != nullptr)
        {
            RenderTargetTexture->Release();
        }
        RenderTargetTexture = (ID3D11Texture2D*)rt->GetNativeResource();
        RenderTargetTexture->AddRef();
        InViewportRHI->SetCustomPresent(this);
        UpdateRenderBuffers(InViewport);
    }

protected:
    ID3D11Texture2D* RenderTargetTexture = NULL;

    virtual void UpdateRenderBuffers(const FViewport& InViewport) {
        check(RenderTargetTexture);
        Initialize();
        // get a set of unique RenderBufferD3D11* to delete
        std::set<osvr::renderkit::RenderBufferD3D11*> deletedBuffers;
        for (size_t i = 0; i < mRenderBuffers.size(); i++) {
            if (mRenderBuffers[i].D3D11) {
                deletedBuffers.insert(mRenderBuffers[i].D3D11);
            }
        }

        // then delete them
        for (auto i = deletedBuffers.begin(); i != deletedBuffers.end(); i++) {
            osvr::renderkit::RenderBufferD3D11* current = *i;
            delete current;
        }

        mRenderBuffers.clear();
        osvr::renderkit::RenderBuffer buffer;
        osvr::renderkit::RenderBufferD3D11 *bufferD3D11 = new osvr::renderkit::RenderBufferD3D11();
        bufferD3D11->colorBuffer = RenderTargetTexture;
        //bufferD3D11->colorBufferView = ???;
        //bufferD3D11->depthStencilBuffer = ???;
        //bufferD3D11->depthStencilView = ???;
        buffer.D3D11 = bufferD3D11;

        // Now add the buffer, twice. We are re-using the buffer for both eyes.
        mRenderBuffers.push_back(buffer);
        mRenderBuffers.push_back(buffer);

        // We need to register these new buffers.
        mRenderManager->RegisterRenderBuffers(mRenderBuffers);

        // Now specify the viewports for each.
        mViewportDescriptions.clear();
        int32 width = InViewport.GetSizeXY().X;
        int32 height = InViewport.GetSizeXY().Y;

        osvr::renderkit::OSVR_ViewportDescription leftEye, rightEye;

        leftEye.left = 0;
        leftEye.lower = 0;
        leftEye.width = width / 2;
        leftEye.height = height;
        mViewportDescriptions.push_back(leftEye);

        rightEye.left = leftEye.left + leftEye.width + 1;
        rightEye.lower = 0;
        rightEye.width = leftEye.width;
        rightEye.height = leftEye.height;
        mViewportDescriptions.push_back(rightEye);
    }

    virtual osvr::renderkit::GraphicsLibrary CreateGraphicsLibrary() override {
        osvr::renderkit::GraphicsLibrary ret;
        // Put the device and context into a structure to let RenderManager
        // know to use this one rather than creating its own.
        ret.D3D11 = new osvr::renderkit::GraphicsLibraryD3D11;
        ret.D3D11->device = GetGraphicsDevice();
        ID3D11DeviceContext *ctx = NULL;
        ret.D3D11->device->GetImmediateContext(&ctx);
        ret.D3D11->context = ctx;

        return ret;
    }

    virtual std::string GetGraphicsLibraryName() override {
        return "Direct3D11";
    }

    virtual bool ShouldFlipY() override {
        return false;
    }

    virtual bool AllocateRenderTargetTexture(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 InFlags, uint32 TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples) override {
        return false;
    }
};

#else
// @todo OpenGL implementation
class FCurrentCustomPresent : public FOSVRCustomPresent< ? >
{
protected:
    virtual osvr::renderkit::GraphicsLibrary CreateGraphicsLibrary() override {
        osvr::renderkit::GraphicsLibrary ret;
        // OpenGL path not implemented yet
        return ret;
    }

    virtual const std::string& GetGraphicsLibraryName() override {
        return "OpenGL";
    }
    
    virtual bool ShouldFlipY() override {
        return false;
    }

};
#endif


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
  virtual void CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) override;
  virtual bool NeedReAllocateViewportRenderTarget(const FViewport &viewport) override;
  virtual void UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, class SViewport*) override;

  virtual bool ShouldUseSeparateRenderTarget() const override
  {
      check(IsInGameThread());
      return IsStereoEnabled();
  }

  /** ISceneViewExtension interface */
  virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
  virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
  virtual FRHICustomPresent* GetCustomPresent() override { return mCustomPresent.get(); }
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
  void GetMonitorInfo(IHeadMountedDisplay::MonitorInfo& MonitorDesc) const;
  void UpdateHeadPose();

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

  OSVRHMDDescription HMDDescription;
  OSVR_DisplayConfig DisplayConfig;
  std::shared_ptr<FCurrentCustomPresent> mCustomPresent = nullptr;
};
