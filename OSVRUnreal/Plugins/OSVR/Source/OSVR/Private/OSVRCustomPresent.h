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
#include "OSVRTypes.h"
#include <osvr/RenderKit/RenderManagerC.h>

DECLARE_LOG_CATEGORY_EXTERN(FOSVRCustomPresentLog, Log, All);

//template<class TGraphicsDevice>
class FOSVRCustomPresent : public FRHICustomPresent
{
public:
  
    FOSVRCustomPresent(OSVR_ClientContext clientContext) :
        FRHICustomPresent(nullptr)
    {
        // If we are passed in a client context to use, we don't own it, so
        // we won't shut it down when we're done with it. Otherwise we will.
        if (clientContext)
        {
            mClientContext = clientContext;
            bOwnClientContext = false;
        }
        else
        {
            bOwnClientContext = true;
            mClientContext = osvrClientInit("com.osvr.unreal.plugin.FOSVRCustomPresent");
        }
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

    virtual void StartRendering()
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
        
        OSVR_ReturnCode rc;
        rc = osvrClientUpdate(mClientContext);
        check(rc == OSVR_RETURN_SUCCESS);

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

    virtual bool IsDisplayOpen()
    {
        return bDisplayOpen;
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

    virtual OSVR_Pose3 GetHeadPoseFromCachedRenderInfoCollection(bool renderThread, bool updateCache)
    {
        FScopeLock lock(&mOSVRMutex);
        OSVR_RenderInfoCollection& renderInfoCollection = renderThread ? mCachedRenderThreadRenderInfoCollection : mCachedGameThreadRenderInfoCollection;
        if (updateCache)
        {
            UpdateCachedRenderInfoCollection(renderInfoCollection);
        }
        return GetHeadPoseFromCachedRenderInfoCollectionImpl(renderInfoCollection);
    }

    virtual void GetProjectionMatrix(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip)
    {
        check(IsInitialized());
        check(IsDisplayOpen());
        FScopeLock lock(&mOSVRMutex);

        OSVR_ReturnCode rc;
        rc = osvrRenderManagerGetDefaultRenderParams(&mRenderParams);
        check(rc == OSVR_RETURN_SUCCESS);

        mRenderParams.nearClipDistanceMeters = static_cast<double>(nearClip);
        mRenderParams.farClipDistanceMeters = static_cast<double>(farClip);

        // this method gets called with alternating eyes starting with the left. We get the render info when
        // the left eye (index 0) is requested (releasing the old one, if any),
        // and re-use the same collection when the right eye (index 0) is requested
        if (eye == 0 || !mCachedProjectionRenderInfoCollection) {
            UpdateCachedRenderInfoCollection(mCachedProjectionRenderInfoCollection);
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
    virtual bool CalculateRenderTargetSize(uint32& InOutSizeX, uint32& InOutSizeY, float screenScale)
    {
        FScopeLock lock(&mOSVRMutex);
        return CalculateRenderTargetSizeImpl(InOutSizeX, InOutSizeY, screenScale);
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
    OSVR_ClientContext mClientContext = nullptr;
    OSVR_RenderManager mRenderManager = nullptr;

    // This is used by GetProjectionMatrix only
    OSVR_RenderInfoCollection mCachedProjectionRenderInfoCollection = nullptr;

    // This is used strictly on the render thread, and is what gets passed back
    // RenderManager Present calls.
    OSVR_RenderInfoCollection mCachedRenderThreadRenderInfoCollection = nullptr;

    // This is used by the game thread, for game thread-only render info caches.
    OSVR_RenderInfoCollection mCachedGameThreadRenderInfoCollection = nullptr;

    virtual bool CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY, float screenScale) = 0;
    virtual void GetProjectionMatrixImpl(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip) = 0;
    virtual bool InitializeImpl() = 0;
    virtual bool LazyOpenDisplayImpl() = 0;
    virtual bool LazySetSrcTextureImpl(FTexture2DRHIParamRef srcTexture) = 0;

    virtual OSVR_Pose3 GetHeadPoseFromCachedRenderInfoCollectionImpl(OSVR_RenderInfoCollection renderInfoCollection)
    {
        check(IsInitialized());
        check(IsDisplayOpen());
        check(renderInfoCollection);

        OSVR_ReturnCode rc;
        OSVR_RenderInfoCount numRenderInfo;
        OSVR_Pose3 ret;
        rc = osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo);
        if (rc != OSVR_RETURN_SUCCESS)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("OSVRCustomPresent::GetHeadPoseFromCachedRenderInfoCollectionImpl: osvrRenderManagerGetNumRenderInfoInCollection call failed."));
            return ret;
        }

        if (numRenderInfo != 2)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("OSVRCustomPresent::GetHeadPoseFromCachedRenderInfoCollectionImpl: expected exactly 2 render info from RenderManager, got %d"),
                numRenderInfo);
            return ret;
        }

        OSVR_Pose3 renderInfo[2];
        for (OSVR_RenderInfoCount i = 0; i < numRenderInfo; i++)
        {
            renderInfo[i] = GetHeadPoseFromCachedRenderInfoCollectionImpl(renderInfoCollection, i);
        }

        OSVR_Pose3 leftEye = renderInfo[0];
        OSVR_Pose3 rightEye = renderInfo[1];

        FQuat leftOrientation = OSVR2FQuat(leftEye.rotation);
        FQuat rightOrientation = OSVR2FQuat(rightEye.rotation);
        FQuat middleOrientation = FQuat::Slerp(leftOrientation, rightOrientation, 1.0f);
        OSVR_Quaternion middleOrientationOSVR = FQuat2OSVR(middleOrientation);

        ret.rotation.data[0] = middleOrientationOSVR.data[0];
        ret.rotation.data[1] = middleOrientationOSVR.data[1];
        ret.rotation.data[2] = middleOrientationOSVR.data[2];
        ret.rotation.data[3] = middleOrientationOSVR.data[3];
        ret.translation.data[0] = (leftEye.translation.data[0] + rightEye.translation.data[0]) / 2.0f;
        ret.translation.data[1] = (leftEye.translation.data[1] + rightEye.translation.data[1]) / 2.0f;
        ret.translation.data[2] = (leftEye.translation.data[2] + rightEye.translation.data[2]) / 2.0f;
        return ret;
    }

    virtual OSVR_Pose3 GetHeadPoseFromCachedRenderInfoCollectionImpl(OSVR_RenderInfoCollection renderInfoCollection, OSVR_RenderInfoCount index) = 0;

    virtual void UpdateCachedRenderInfoCollection(OSVR_RenderInfoCollection &renderInfoCollection)
    {
        OSVR_ReturnCode rc;
        rc = osvrClientUpdate(mClientContext);
        check(rc == OSVR_RETURN_SUCCESS);

        if (renderInfoCollection) {
            rc = osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection);
            check(rc == OSVR_RETURN_SUCCESS);
        }
        rc = osvrRenderManagerGetRenderInfoCollection(mRenderManager, mRenderParams, &renderInfoCollection);
        check(rc == OSVR_RETURN_SUCCESS);
    }

    template<class TGraphicsDevice>
    TGraphicsDevice* GetGraphicsDevice()
    {
        check(IsInRenderingThread());
        auto ret = RHIGetNativeDevice();
        return reinterpret_cast<TGraphicsDevice*>(ret);
    }

    virtual void FinishRendering() = 0;

    // abstract methods, implement in DirectX/OpenGL specific subclasses
    virtual FString GetGraphicsLibraryName() = 0;
    virtual bool ShouldFlipY() = 0;
    virtual void UpdateRenderBuffers() = 0;
};

