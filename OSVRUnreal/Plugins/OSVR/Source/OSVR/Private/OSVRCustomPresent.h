//
// Copyright 2016 Sensics, Inc.
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
#include <osvr/RenderKit/RenderManagerC.h>
#include <vector>
#include <string>

DECLARE_LOG_CATEGORY_EXTERN(FOSVRCustomPresentLog, Log, All);

//template<class TGraphicsDevice>
class FOSVRCustomPresent : public FRHICustomPresent
{
public:
    FTexture2DRHIRef mRenderTexture;
  
    FOSVRCustomPresent(OSVR_ClientContext clientContext, float screenScale) :
        FRHICustomPresent(nullptr)
    {
        // If we are passed in a client context to use, we don't own it, so
        // we won't shut it down when we're done with it. Otherwise we will.
        // @todo - we're not currently using the passed-in clientContext, so
        // for now we always own it.
        //bOwnClientContext = (clientContext == nullptr);
        bOwnClientContext = true;
        mClientContext = osvrClientInit("com.osvr.unreal.plugin.FOSVRCustomPresent");
        mScreenScale = screenScale;
    }

    virtual ~FOSVRCustomPresent()
    {
        OSVR_ReturnCode rc;
        if (mRenderManager)
        {
            rc = osvrDestroyRenderManager(mRenderManager);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("[OSVR] Failed to destroy the render manager in ~FOSVRCustomPresent()."));
            }
        }

        // only shut down the client context if we own it (currently always)
        if (bOwnClientContext && mClientContext)
        {
            rc = osvrClientShutdown(mClientContext);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("[OSVR] Failed to shut down client context in ~FOSVRCustomPresent()."));
            }
        }
    }

    // virtual methods from FRHICustomPresent

    virtual void OnBackBufferResize() override
    {
    }

    virtual bool Present(int32 &inOutSyncInterval) override
    {
        check(IsInRenderingThread());
        FScopeLock lock(&mOSVRMutex);
        InitializeImpl();
        if (!bDisplayOpen)
        {
            bDisplayOpen = LazyOpenDisplayImpl();
            check(bDisplayOpen);
        }
        
        // @todo This is giving us a black screen.
        FinishRendering();
        return true;
    }

    virtual void RenderTexture_RenderThread(
        FRHICommandListImmediate& rhiCmdList,
        FTexture2DRHIParamRef backBuffer,
        FTexture2DRHIParamRef srcTexture)
    {
    }

    // Initializes RenderManager but does not open the displays
    // Can be called from the render thread or game thread.
    // @todo: this call should be lazy, then IsInitialized can be removed.
    virtual bool Initialize()
    {
        FScopeLock lock(&mOSVRMutex);
        return InitializeImpl();
    }

    virtual bool IsInitialized()
    {
        return bInitialized;
    }

    virtual bool LazySetSrcTexture(FTexture2DRHIParamRef srcTexture)
    {
        FScopeLock lock(&mOSVRMutex);
        return LazySetSrcTextureImpl(srcTexture);
    }

    virtual bool LazyOpenDisplay()
    {
        FScopeLock lock(&mOSVRMutex);
        if(IsInRenderingThread() && IsInitialized() && !bDisplayOpen)
        {
            bDisplayOpen = LazyOpenDisplayImpl();    
        }
        return bDisplayOpen;
    }
    
    virtual void GetProjectionMatrix(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip)
    {
        OSVR_ReturnCode rc;
        rc = osvrRenderManagerGetDefaultRenderParams(&mRenderParams);
        check(rc == OSVR_RETURN_SUCCESS);

        mRenderParams.nearClipDistanceMeters = static_cast<double>(nearClip);
        mRenderParams.farClipDistanceMeters = static_cast<double>(farClip);

        // this method gets called with alternating eyes starting with the left. We get the render info when
        // the left eye (index 0) is requested (releasing the old one, if any),
        // and re-use the same collection when the right eye (index 0) is requested
        if (eye == 0 || !mCachedRenderInfoCollection) {
            if (mCachedRenderInfoCollection) {
                rc = osvrRenderManagerReleaseRenderInfoCollection(mCachedRenderInfoCollection);
                check(rc == OSVR_RETURN_SUCCESS);
            }
            rc = osvrRenderManagerGetRenderInfoCollection(mRenderManager, mRenderParams, &mCachedRenderInfoCollection);
            check(rc == OSVR_RETURN_SUCCESS);
        }

        GetProjectionMatrixImpl(eye, left, right, bottom, top, nearClip, farClip);
    }

    virtual bool UpdateViewport(const FViewport& InViewport, class FRHIViewport* InViewportRHI)
    {
        FScopeLock lock(&mOSVRMutex);

        check(IsInGameThread());
        if (!IsInitialized())
        {
            UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("UpdateViewport called but custom present is not initialized - doing nothing"));
            return false;
        }
        else
        {
            check(InViewportRHI);
            auto oldCustomPresent = InViewportRHI->GetCustomPresent();
            if (oldCustomPresent != this)
            {
                InViewportRHI->SetCustomPresent(this);
            }
            // UpdateViewport is called before we're initialized, so we have to
            // defer updates to the render buffers until we're in the render thread.
            //bRenderBuffersNeedToUpdate = true;
            return true;
        }
    }

    // RenderManager normalizes displays a bit. We create the render target assuming horizontal side-by-side.
    // RenderManager then rotates that render texture if needed for vertical side-by-side displays.
    virtual bool CalculateRenderTargetSize(uint32& InOutSizeX, uint32& InOutSizeY)
    {
        FScopeLock lock(&mOSVRMutex);
        return CalculateRenderTargetSizeImpl(InOutSizeX, InOutSizeY);
    }

    virtual bool AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples = 1) = 0;

protected:
    FCriticalSection mOSVRMutex;
    TArray<OSVR_ViewportDescription> mViewportDescriptions;
    OSVR_RenderParams mRenderParams;

    bool bRenderBuffersNeedToUpdate = true;
    bool bInitialized = false;
    bool bDisplayOpen = false;
    bool bOwnClientContext = true;
    float mScreenScale = 1.0f;
    OSVR_ClientContext mClientContext = nullptr;
    OSVR_RenderManager mRenderManager = nullptr;
    OSVR_RenderInfoCollection mCachedRenderInfoCollection = nullptr;

    virtual bool CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY) = 0;
    virtual void GetProjectionMatrixImpl(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip) = 0;
    virtual bool InitializeImpl() = 0;
    virtual bool LazyOpenDisplayImpl() = 0;
    virtual bool LazySetSrcTextureImpl(FTexture2DRHIParamRef srcTexture) = 0;
    
    template<class TGraphicsDevice>
    TGraphicsDevice* GetGraphicsDevice()
    {
        auto ret = RHIGetNativeDevice();
        return reinterpret_cast<TGraphicsDevice*>(ret);
    }

    virtual void FinishRendering() = 0;

    // abstract methods, implement in DirectX/OpenGL specific subclasses
    virtual FString GetGraphicsLibraryName() = 0;
    virtual bool ShouldFlipY() = 0;
    virtual void UpdateRenderBuffers() = 0;
};

