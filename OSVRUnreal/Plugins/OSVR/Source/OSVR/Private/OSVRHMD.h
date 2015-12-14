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
#include <osvr/RenderKit/RenderManagerD3D11C.h>
#include "HideWindowsPlatformTypes.h"
#include "Runtime/Windows/D3D11RHI/Private/D3D11RHIPrivate.h"
#else
#include <osvr/RenderKit/RenderManagerOpenGLC.h>
#endif

#include <memory>
#include <iostream>
#include <set>
#include <vector>

//class ID3D11Device;
//class ID3D11DeviceContext;

template<class TGraphicsDevice>
class FOSVRCustomPresent : public FRHICustomPresent
{
public:
    FTexture2DRHIRef mRenderTexture;

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
        FScopeLock lock(&mOSVRMutex);
        InitializeImpl();
        FinishRendering();
        return true;
    }

    // implement this in the sub-class
    virtual void Initialize() {
        FScopeLock lock(&mOSVRMutex);
        InitializeImpl();
    }

    virtual bool IsInitialized() {
        return mInitialized;
    }

    virtual void UpdateViewport(const FViewport& InViewport, class FRHIViewport* InViewportRHI) = 0;

    // RenderManager normalizes displays a bit. We create the render target assuming horizontal side-by-side.
    // RenderManager then rotates that render texture if needed for vertical side-by-side displays.
    virtual void CalculateRenderTargetSize(uint32& InOutSizeX, uint32& InOutSizeY) {
        FScopeLock lock(&mOSVRMutex);
        CalculateRenderTargetSizeImpl(InOutSizeX, InOutSizeY);
    }

    virtual bool AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples = 1) = 0;

protected:
    FCriticalSection mOSVRMutex;
    std::vector<OSVR_ViewportDescription> mViewportDescriptions;
    OSVR_RenderParams mRenderParams;
    
    bool mRenderBuffersNeedToUpdate = true;
    bool mInitialized = false;
    OSVR_ClientContext mClientContext = nullptr;
    OSVR_RenderManager mRenderManager = nullptr;

    virtual void CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY) = 0;

    virtual void InitializeImpl() = 0;

    virtual TGraphicsDevice* GetGraphicsDevice() {
        auto ret = RHIGetNativeDevice();
        return reinterpret_cast<TGraphicsDevice*>(ret);
    }

    virtual void FinishRendering() = 0;

    // abstract methods, implement in DirectX/OpenGL specific subclasses
    virtual std::string GetGraphicsLibraryName() = 0;
    virtual bool ShouldFlipY() = 0;
    virtual void UpdateRenderBuffers() = 0;
};

#if PLATFORM_WINDOWS

class FCurrentCustomPresent : public FOSVRCustomPresent<ID3D11Device>
{
public:
    FCurrentCustomPresent(OSVR_ClientContext clientContext) :
        FOSVRCustomPresent(clientContext)
    {}

    virtual void UpdateViewport(const FViewport& InViewport, class FRHIViewport* InViewportRHI) override {
        FScopeLock lock(&mOSVRMutex);

        check(IsInGameThread());
        check(InViewportRHI);
        //const FTexture2DRHIRef& rt = InViewport.GetRenderTargetTexture();
        //check(IsValidRef(rt));
        //SetRenderTargetTexture((ID3D11Texture2D*)rt->GetNativeResource()); // @todo: do we need to do this?
        auto oldCustomPresent = InViewportRHI->GetCustomPresent();
        if (oldCustomPresent != this) {
            InViewportRHI->SetCustomPresent(this);
        }
        // UpdateViewport is called before we're initialized, so we have to
        // defer updates to the render buffers until we're in the render thread.
        //mRenderBuffersNeedToUpdate = true;
    }

    virtual bool AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples = 1) override {
        FScopeLock lock(&mOSVRMutex);
        auto d3d11RHI = static_cast<FD3D11DynamicRHI*>(GDynamicRHI);
        auto graphicsDevice = GetGraphicsDevice();
        HRESULT hr;
        D3D11_TEXTURE2D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof(textureDesc));
        textureDesc.Width = sizeX;
        textureDesc.Height = sizeY;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        //textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        // We need it to be both a render target and a shader resource
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        ID3D11Texture2D *D3DTexture = nullptr;
        hr = graphicsDevice->CreateTexture2D(
            &textureDesc, NULL, &D3DTexture);
        check(!FAILED(hr));

        SetRenderTargetTexture(D3DTexture);

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        memset(&renderTargetViewDesc, 0, sizeof(renderTargetViewDesc));
        // This must match what was created in the texture to be rendered
        //renderTargetViewDesc.Format = renderTextureDesc.Format;
        renderTargetViewDesc.Format = textureDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;

        // Create the render target view.
        ID3D11RenderTargetView *renderTargetView; //< Pointer to our render target view
        hr = graphicsDevice->CreateRenderTargetView(
            RenderTargetTexture, &renderTargetViewDesc, &renderTargetView);
        check(!FAILED(hr));

        RenderTargetView = renderTargetView;

        ID3D11ShaderResourceView* shaderResourceView = nullptr;
        bool createdRTVsPerSlice = false;
        int32 rtvArraySize = 1;
        TArray<TRefCountPtr<ID3D11RenderTargetView>> renderTargetViews;
        TRefCountPtr<ID3D11DepthStencilView>* depthStencilViews = nullptr;
        uint32 sizeZ = 0;
        EPixelFormat epFormat = EPixelFormat(format);
        bool cubemap = false;
        bool pooled = false;
        // override flags
        flags = TexCreate_RenderTargetable | TexCreate_ShaderResource;

        renderTargetViews.Add(renderTargetView);
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        memset(&shaderResourceViewDesc, 0, sizeof(shaderResourceViewDesc));
        shaderResourceViewDesc.Format = textureDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = textureDesc.MipLevels - 1;

        hr = graphicsDevice->CreateShaderResourceView(
            RenderTargetTexture, &shaderResourceViewDesc, &shaderResourceView);
        check(!FAILED(hr));

        auto targetableTexture = new FD3D11Texture2D(
            d3d11RHI, D3DTexture, shaderResourceView, createdRTVsPerSlice,
            rtvArraySize, renderTargetViews, depthStencilViews,
            textureDesc.Width, textureDesc.Height, sizeZ, numMips, numSamples, epFormat,
            cubemap, flags, pooled, FClearValueBinding::Black);

        outTargetableTexture = targetableTexture->GetTexture2D();
        outShaderResourceTexture = targetableTexture->GetTexture2D();
        mRenderTexture = targetableTexture;
        mRenderBuffersNeedToUpdate = true;
        UpdateRenderBuffers();
        return true;
    }

protected:
    ID3D11Texture2D* RenderTargetTexture = NULL;
    ID3D11RenderTargetView * RenderTargetView = NULL;

    std::vector<OSVR_RenderBufferD3D11> mRenderBuffers;
    std::vector<OSVR_RenderInfoD3D11> mRenderInfos;
    OSVR_RenderManagerD3D11 mRenderManagerD3D11 = nullptr;

    virtual void CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY) override {
        InitializeImpl();
        // Should we create a RenderParams?
        OSVR_ReturnCode rc;

        rc = osvrRenderManagerGetDefaultRenderParams(&mRenderParams);
        check(rc == OSVR_RETURN_SUCCESS);

        OSVR_RenderInfoCount numRenderInfo;
        rc = osvrRenderManagerGetNumRenderInfo(mRenderManager, mRenderParams, &numRenderInfo);
        check(rc == OSVR_RETURN_SUCCESS);

        mRenderInfos.clear();
        for (size_t i = 0; i < numRenderInfo; i++) {
            OSVR_RenderInfoD3D11 renderInfo;
            rc = osvrRenderManagerGetRenderInfoD3D11(mRenderManagerD3D11, i, mRenderParams, &renderInfo);
            check(rc == OSVR_RETURN_SUCCESS);

            mRenderInfos.push_back(renderInfo);
        }

        // check some assumptions. Should all be the same height.
        check(mRenderInfos.size() == 2);
        check(mRenderInfos[0].viewport.height == mRenderInfos[1].viewport.height);
        InOutSizeX = mRenderInfos[0].viewport.width + mRenderInfos[1].viewport.width;
        InOutSizeY = mRenderInfos[0].viewport.height;
        check(InOutSizeX != 0 && InOutSizeY != 0);
    }

    virtual void InitializeImpl() override {
        if (!IsInitialized()) {
            auto graphicsLibrary = CreateGraphicsLibrary();
            auto graphicsLibraryName = GetGraphicsLibraryName();
            OSVR_ReturnCode rc;

            check(mClientContext);

            rc = osvrCreateRenderManagerD3D11(mClientContext, graphicsLibraryName.c_str(), graphicsLibrary, &mRenderManager, &mRenderManagerD3D11);
            check(rc == OSVR_RETURN_SUCCESS && mRenderManager && mRenderManagerD3D11);

            rc = osvrRenderManagerGetDoingOkay(mRenderManager);
            check(rc == OSVR_RETURN_SUCCESS);

            OSVR_OpenResultsD3D11 results;
            rc = osvrRenderManagerOpenDisplayD3D11(mRenderManagerD3D11, &results);
            check(results.status != OSVR_OPEN_STATUS_FAILURE);

            // @todo: create the textures?

            mInitialized = true;
        }
    }

    virtual void FinishRendering() override
    {
        check(IsInitialized());
        UpdateRenderBuffers();
        // all of the render manager samples keep the flipY at the default false,
        // for both OpenGL and DirectX. Is this even needed anymore?
        OSVR_ReturnCode rc;
        OSVR_RenderManagerPresentState presentState;
        rc = osvrRenderManagerStartPresentRenderBuffers(&presentState);
        check(rc == OSVR_RETURN_SUCCESS);
        check(mRenderBuffers.size() == mRenderInfos.size() && mRenderBuffers.size() == mViewportDescriptions.size());
        for (size_t i = 0; i < mRenderBuffers.size(); i++) {
            rc = osvrRenderManagerPresentRenderBufferD3D11(presentState, mRenderBuffers[i], mRenderInfos[i], mViewportDescriptions[i]);
            check(rc == OSVR_RETURN_SUCCESS);
        }
        rc = osvrRenderManagerFinishPresentRenderBuffers(mRenderManager, presentState, mRenderParams, ShouldFlipY() ? OSVR_TRUE : OSVR_FALSE);
        check(rc == OSVR_RETURN_SUCCESS);
    }

    void SetRenderTargetTexture(ID3D11Texture2D* renderTargetTexture) {
        if (RenderTargetTexture != nullptr && RenderTargetTexture != renderTargetTexture)
        {
            // @todo: testing if this is causing problems later on.
            //RenderTargetTexture->Release();
        }
        RenderTargetTexture = renderTargetTexture;
        RenderTargetTexture->AddRef();
    }

    virtual void UpdateRenderBuffers() override {
        HRESULT hr;
        
        check(IsInitialized());
        if (mRenderBuffersNeedToUpdate) {
            uint32 width;
            uint32 height;
            // @todo: can't call this here, we're in the wrong thread.
            CalculateRenderTargetSizeImpl(width, height);

            //check(mRenderTexture);
            //SetRenderTargetTexture(reinterpret_cast<ID3D11Texture2D*>(mRenderTexture->GetNativeResource()));
            check(RenderTargetTexture);

            D3D11_TEXTURE2D_DESC renderTextureDesc;
            RenderTargetTexture->GetDesc(&renderTextureDesc);

            auto graphicsDevice = GetGraphicsDevice();

            //D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
            //memset(&renderTargetViewDesc, 0, sizeof(renderTargetViewDesc));
            //// This must match what was created in the texture to be rendered
            ////renderTargetViewDesc.Format = renderTextureDesc.Format;
            //renderTargetViewDesc.Format = renderTextureDesc.Format;
            //renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            //renderTargetViewDesc.Texture2D.MipSlice = 0;

            //// Create the render target view.
            //ID3D11RenderTargetView *renderTargetView; //< Pointer to our render target view
            //hr = graphicsDevice->CreateRenderTargetView(
            //    RenderTargetTexture, &renderTargetViewDesc, &renderTargetView);
            //check(!FAILED(hr));

            
            mRenderBuffers.clear();

            // Adding two RenderBuffers, but they both point to the same D3D11 texture target
            for (int i = 0; i < 2; i++) {
                OSVR_RenderBufferD3D11 buffer;
                buffer.colorBuffer = RenderTargetTexture;
                //buffer.colorBufferView = renderTargetView;
                buffer.colorBufferView = RenderTargetView;
                //buffer.depthStencilBuffer = ???;
                //buffer.depthStencilView = ???;
                mRenderBuffers.push_back(buffer);
            }

            // We need to register these new buffers.
            // @todo RegisterRenderBuffers doesn't do anything other than set a flag and crash
            // if you pass it a non-empty vector here. Passing it a dummy array for now.

            {
                OSVR_RenderManagerRegisterBufferState state;
                hr = osvrRenderManagerStartRegisterRenderBuffers(&state);
                check(hr == OSVR_RETURN_SUCCESS);

                for (size_t i = 0; i < mRenderBuffers.size(); i++) {
                    hr = osvrRenderManagerRegisterRenderBufferD3D11(state, mRenderBuffers[i]);
                    check(hr == OSVR_RETURN_SUCCESS);
                }

                hr = osvrRenderManagerFinishRegisterRenderBuffers(mRenderManager, state, false);
                check(hr == OSVR_RETURN_SUCCESS);
            }

            // Now specify the viewports for each.
            mViewportDescriptions.clear();

            OSVR_ViewportDescription leftEye, rightEye;

            leftEye.left = 0;
            leftEye.lower = 0;
            leftEye.width = 0.5;
            leftEye.height = 1.0;
            mViewportDescriptions.push_back(leftEye);

            rightEye.left = 0.5;
            rightEye.lower = 0;
            rightEye.width = 0.5;
            rightEye.height = 1.0;
            mViewportDescriptions.push_back(rightEye);

            mRenderBuffersNeedToUpdate = false;
        }
    }

    virtual OSVR_GraphicsLibraryD3D11 CreateGraphicsLibrary() {
        OSVR_GraphicsLibraryD3D11 ret;
        // Put the device and context into a structure to let RenderManager
        // know to use this one rather than creating its own.
        ret.device = GetGraphicsDevice();
        ID3D11DeviceContext *ctx = NULL;
        ret.device->GetImmediateContext(&ctx);
        check(ctx);
        ret.context = ctx;

        return ret;
    }

    virtual std::string GetGraphicsLibraryName() override {
        return "Direct3D11";
    }

    virtual bool ShouldFlipY() override {
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
  virtual void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIParamRef BackBuffer, FTexture2DRHIParamRef SrcTexture) const override;
  virtual void GetEyeRenderParams_RenderThread(const struct FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
  virtual void GetTimewarpMatrices_RenderThread(const struct FRenderingCompositePassContext& Context, FMatrix& EyeRotationStart, FMatrix& EyeRotationEnd) const override;
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
      if (GIsEditor) {
          return nullptr;
      }
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

  IRendererModule* RendererModule;

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
  bool bHmdConnected;
  bool bHmdOverridesApplied;
  bool bWaitedForClientStatus = false;
  
  OSVRHMDDescription HMDDescription;
  OSVR_DisplayConfig DisplayConfig;
  FCurrentCustomPresent* mCustomPresent = nullptr;
};
