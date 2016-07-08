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
#include "OSVRCustomPresent.h"

#include <osvr/RenderKit/RenderManagerC.h>
#include <osvr/RenderKit/RenderManagerOpenGLC.h>

#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"

class FOpenGLCustomPresent : public FOSVRCustomPresent
{
public:
    FOpenGLCustomPresent(OSVR_ClientContext clientContext, float screenScale) :
        FOSVRCustomPresent(clientContext, screenScale)
    {
    }

    virtual ~FOpenGLCustomPresent()
    {
        if (RenderTargetTexture > 0)
        {
            glDeleteTextures(1, &RenderTargetTexture);
        }
    }

protected:

    TArray<OSVR_RenderBufferOpenGL> mRenderBuffers;
    TArray<OSVR_RenderInfoOpenGL> mRenderInfos;
    OSVR_RenderManagerOpenGL mRenderManagerOpenGL = nullptr;
    GLuint RenderTargetTexture = 0;

    virtual FString GetGraphicsLibraryName() override
    {
        return FString("OpenGL");
    }

    virtual bool ShouldFlipY() override
    {
        return false;
    }

    virtual bool AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples = 1) override
    {
        FScopeLock lock(&mOSVRMutex);
        if (IsInitialized())
        {
            // create the color buffer
            if (RenderTargetTexture > 0)
            {
                glDeleteTextures(1, &RenderTargetTexture);
            }
            RenderTargetTexture = 0;
            osvrRenderManagerCreateColorBufferOpenGL(sizeX, sizeY, &RenderTargetTexture);

            //SetRenderTargetTexture(D3DTexture);

            auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
            GLenum target = numSamples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
            GLenum attachment = GL_NONE;
            bool bAllocatedStorage = false;
            numMips = 1;
            uint8* textureRange = nullptr;

            auto targetableTexture = new FOpenGLTexture2D(
                GLRHI, RenderTargetTexture, target, attachment, sizeX, sizeY, 0, numMips, numSamples, 1, EPixelFormat(format), false,
                bAllocatedStorage, flags, textureRange, FClearValueBinding::Black);

            outTargetableTexture = targetableTexture->GetTexture2D();
            outShaderResourceTexture = targetableTexture->GetTexture2D();
            mRenderTexture = targetableTexture;
            bRenderBuffersNeedToUpdate = true;
            UpdateRenderBuffers();
            return true;
        }
        return false;
    }

    virtual void GetProjectionMatrixImpl(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip) override
    {
        OSVR_ReturnCode rc;
        OSVR_RenderInfoOpenGL renderInfo;
        rc = osvrRenderManagerGetRenderInfoFromCollectionOpenGL(mCachedRenderInfoCollection, eye, &renderInfo);
        check(rc == OSVR_RETURN_SUCCESS);

        // previously we divided these by renderInfo.projection.nearClip but we need
        // to pass these unmodified through to the OSVR_Projection_to_D3D call (and OpenGL
        // equivalent)
        left = static_cast<float>(renderInfo.projection.left);
        right = static_cast<float>(renderInfo.projection.right);
        top = static_cast<float>(renderInfo.projection.top);
        bottom = static_cast<float>(renderInfo.projection.bottom);
    }

    virtual bool CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY) override
    {
        if (InitializeImpl())
        {
            // Should we create a RenderParams?
            OSVR_ReturnCode rc;

            rc = osvrRenderManagerGetDefaultRenderParams(&mRenderParams);
            check(rc == OSVR_RETURN_SUCCESS);

            OSVR_RenderInfoCollection renderInfoCollection = { 0 };
            rc = osvrRenderManagerGetRenderInfoCollection(mRenderManager, mRenderParams, &renderInfoCollection);
            check(rc == OSVR_RETURN_SUCCESS);

            OSVR_RenderInfoCount numRenderInfo;
            rc = osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo);
            check(rc == OSVR_RETURN_SUCCESS);

            mRenderInfos.Empty();
            for (size_t i = 0; i < numRenderInfo; i++)
            {
                OSVR_RenderInfoOpenGL renderInfo;
                rc = osvrRenderManagerGetRenderInfoFromCollectionOpenGL(renderInfoCollection, i, &renderInfo);
                check(rc == OSVR_RETURN_SUCCESS);

                mRenderInfos.Add(renderInfo);
            }

            rc = osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection);
            check(rc == OSVR_RETURN_SUCCESS);

            // check some assumptions. Should all be the same height.
            check(mRenderInfos.Num() == 2);
            check(mRenderInfos[0].viewport.height == mRenderInfos[1].viewport.height);

            mRenderInfos[0].viewport.width = int(float(mRenderInfos[0].viewport.width) * mScreenScale);
            mRenderInfos[0].viewport.height = int(float(mRenderInfos[0].viewport.height) * mScreenScale);
            mRenderInfos[1].viewport.width = mRenderInfos[0].viewport.width;
            mRenderInfos[1].viewport.height = mRenderInfos[0].viewport.height;
            mRenderInfos[1].viewport.left = mRenderInfos[0].viewport.width;

            InOutSizeX = mRenderInfos[0].viewport.width + mRenderInfos[1].viewport.width;
            InOutSizeY = mRenderInfos[0].viewport.height;
            check(InOutSizeX != 0 && InOutSizeY != 0);
            return true;
        }
        return false;
    }

    virtual bool InitializeImpl() override
    {
        if (!IsInitialized())
        {
            auto graphicsLibrary = CreateGraphicsLibrary();
            auto graphicsLibraryName = GetGraphicsLibraryName();
            OSVR_ReturnCode rc;

            if (!mClientContext)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("Can't initialize FOSVRCustomPresent without a valid client context"));
                return false;
            }

            rc = osvrCreateRenderManagerOpenGL(
                mClientContext, TCHAR_TO_ANSI(*graphicsLibraryName),
                graphicsLibrary, &mRenderManager, &mRenderManagerOpenGL);

            if (rc == OSVR_RETURN_FAILURE || !mRenderManager || !mRenderManagerOpenGL)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("osvrCreateRenderManagerD3D11 call failed, or returned numm renderManager/renderManagerD3D11 instances"));
                return false;
            }

            rc = osvrRenderManagerGetDoingOkay(mRenderManager);
            if (rc == OSVR_RETURN_FAILURE)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("osvrRenderManagerGetDoingOkay call failed. Perhaps there was an error during initialization?"));
                return false;
            }

            OSVR_OpenResultsOpenGL results;
            rc = osvrRenderManagerOpenDisplayOpenGL(mRenderManagerOpenGL, &results);
            if (rc == OSVR_RETURN_FAILURE || results.status == OSVR_OPEN_STATUS_FAILURE)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning,
                    TEXT("osvrRenderManagerOpenDisplayD3D11 call failed, or the result status was OSVR_OPEN_STATUS_FAILURE. Potential causes could be that the display is already open in direct mode with another app, or the display does not support direct mode"));
                return false;
            }

            // @todo: create the textures?

            bInitialized = true;
        }
        return true;
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
        check(mRenderBuffers.Num() == mRenderInfos.Num() && mRenderBuffers.Num() == mViewportDescriptions.Num());
        for (size_t i = 0; i < mRenderBuffers.Num(); i++)
        {
            rc = osvrRenderManagerPresentRenderBufferOpenGL(presentState, mRenderBuffers[i], mRenderInfos[i], mViewportDescriptions[i]);
            check(rc == OSVR_RETURN_SUCCESS);
        }
        rc = osvrRenderManagerFinishPresentRenderBuffers(mRenderManager, presentState, mRenderParams, ShouldFlipY() ? OSVR_TRUE : OSVR_FALSE);
        check(rc == OSVR_RETURN_SUCCESS);
    }

    virtual void UpdateRenderBuffers() override
    {
        OSVR_ReturnCode rc;

        check(IsInitialized());
        if (bRenderBuffersNeedToUpdate)
        {
            //check(RenderTargetTexture);
            mRenderBuffers.Empty();

            // Adding two RenderBuffers, but they both point to the same D3D11 texture target
            for (int i = 0; i < 2; i++)
            {
                OSVR_RenderBufferOpenGL buffer = { 0 };
                buffer.colorBufferName = RenderTargetTexture;
                mRenderBuffers.Add(buffer);
            }

            // We need to register these new buffers.
            // @todo RegisterRenderBuffers doesn't do anything other than set a flag and crash
            // if you pass it a non-empty vector here. Passing it a dummy array for now.

            {
                OSVR_RenderManagerRegisterBufferState state;
                rc = osvrRenderManagerStartRegisterRenderBuffers(&state);
                check(rc == OSVR_RETURN_SUCCESS);

                for (size_t i = 0; i < mRenderBuffers.Num(); i++)
                {
                    rc = osvrRenderManagerRegisterRenderBufferOpenGL(state, mRenderBuffers[i]);
                    check(rc == OSVR_RETURN_SUCCESS);
                }

                rc = osvrRenderManagerFinishRegisterRenderBuffers(mRenderManager, state, false);
                check(rc == OSVR_RETURN_SUCCESS);
            }

            // Now specify the viewports for each.
            mViewportDescriptions.Empty();

            OSVR_ViewportDescription leftEye, rightEye;

            leftEye.left = 0;
            leftEye.lower = 0;
            leftEye.width = 0.5;
            leftEye.height = 1.0;
            mViewportDescriptions.Add(leftEye);

            rightEye.left = 0.5;
            rightEye.lower = 0;
            rightEye.width = 0.5;
            rightEye.height = 1.0;
            mViewportDescriptions.Add(rightEye);

            bRenderBuffersNeedToUpdate = false;
        }
    }

    virtual OSVR_GraphicsLibraryOpenGL CreateGraphicsLibrary()
    {
        OSVR_GraphicsLibraryOpenGL ret = { 0 };
        // @todo: anything needed here?
        return ret;
    }
};


