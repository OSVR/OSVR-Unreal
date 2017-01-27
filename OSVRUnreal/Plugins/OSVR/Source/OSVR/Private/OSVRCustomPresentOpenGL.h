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
#include <osvr/util/PlatformConfig.h>

#if OSVR_ANDROID
// @todo this may not work - if not, what headers is unreal expecting?
#define OSVR_RM_USE_OPENGLES20 1
#else
// you can't include <GL/gl.h> and <GL/glcorearb.h> in the same source file,
// and unreal is going to include <GL/glcorearb.h>
#define OSVR_RM_SKIP_GL_INCLUDE 1
#define OSVR_RM_SKIP_GLEXT_INCLUDE 1
#include <GL/glcorearb.h>
#endif

#include <osvr/RenderKit/RenderManagerOpenGLC.h>

#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"

typedef struct FUnrealBackBufferContainer
{
    GLint displayFrameBuffer;
    int width;
    int height;
};


//==========================================================================
// Toolkit object to handle our window creation needs.  We pass it down to
// the RenderManager to override its window and context creation behavior,
// since we are using the Unreal window/context creation.
class FUnrealOSVRRenderManagerOpenGLToolkit {
private:
    OSVR_OpenGLToolkitFunctions toolkit;
    FUnrealBackBufferContainer* mBackBufferContainer;

    // eliminate force to bool performance warning.
    static inline OSVR_CBool ConvertBool(bool bValue)
    {
        return bValue ? OSVR_TRUE : OSVR_FALSE;
    }

    static inline bool ConvertBool(OSVR_CBool bValue)
    {
        return bValue == OSVR_TRUE ? true : false;
    }

    static void createImpl(void* data)
    {
    }

    static void destroyImpl(void* data)
    {
        delete ((FUnrealOSVRRenderManagerOpenGLToolkit*)data);
    }

    static OSVR_CBool addOpenGLContextImpl(void* data, const OSVR_OpenGLContextParams* p)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->addOpenGLContext(p));
    }

    static OSVR_CBool removeOpenGLContextsImpl(void* data)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->removeOpenGLContexts());
    }

    static OSVR_CBool makeCurrentImpl(void* data, size_t display)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->makeCurrent(display));
    }

    static OSVR_CBool swapBuffersImpl(void* data, size_t display)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->swapBuffers(display));
    }

    static OSVR_CBool setVerticalSyncImpl(void* data, OSVR_CBool verticalSync)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->setVerticalSync(ConvertBool(verticalSync)));
    }

    static OSVR_CBool handleEventsImpl(void* data)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->handleEvents());
    }

    static OSVR_CBool getDisplayFrameBufferImpl(void* data, size_t display, GLint* displayFrameBufferOut)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->getDisplayFrameBuffer(display, displayFrameBufferOut));
    }

    static OSVR_CBool getDisplaySizeOverrideImpl(void* data, size_t display, int* width, int* height)
    {
        return ConvertBool(((FUnrealOSVRRenderManagerOpenGLToolkit*)data)->getDisplaySizeOverride(display, width, height));
    }

public:
    FUnrealOSVRRenderManagerOpenGLToolkit(FUnrealBackBufferContainer* backBufferContainer)
        : mBackBufferContainer(backBufferContainer)
    {
        memset(&toolkit, 0, sizeof(toolkit));
        toolkit.size = sizeof(toolkit);
        toolkit.data = this;

        toolkit.create = createImpl;
        toolkit.destroy = destroyImpl;
        toolkit.addOpenGLContext = addOpenGLContextImpl;
        toolkit.removeOpenGLContexts = removeOpenGLContextsImpl;
        toolkit.makeCurrent = makeCurrentImpl;
        toolkit.swapBuffers = swapBuffersImpl;
        toolkit.setVerticalSync = setVerticalSyncImpl;
        toolkit.handleEvents = handleEventsImpl;
        toolkit.getDisplayFrameBuffer = getDisplayFrameBufferImpl;
        toolkit.getDisplaySizeOverride = getDisplaySizeOverrideImpl;
    }

    ~FUnrealOSVRRenderManagerOpenGLToolkit()
    {
    }

    const OSVR_OpenGLToolkitFunctions* getToolkit() const
    {
        return &toolkit;
    }

    bool addOpenGLContext(const OSVR_OpenGLContextParams* p)
    {
        // @todo change resolution and move the window here?
        // We may need to just record the params and respond to them later,
        // since this call has to be done on the game thread, but it's called on the
        // window thread.

        //FSystemResolution::RequestResolutionChange(p->width, p->height, EWindowMode::Windowed);
        return true;
    }

    bool removeOpenGLContexts()
    {
        return true;
    }

    bool makeCurrent(size_t display)
    {
        // we are always current, since unreal creates our context
        // beforehand
        return true;
    }

    bool swapBuffers(size_t display)
    {
        // unreal does this for us
        return true;
    }

    bool setVerticalSync(bool verticalSync)
    {
        // @todo ???
        return true;
    }

    bool handleEvents()
    {
        // @todo ???
        return true;
    }

    bool getDisplayFrameBuffer(size_t display, GLint* displayFrameBufferOut)
    {
        (*displayFrameBufferOut) = mBackBufferContainer->displayFrameBuffer;
        return true;
    }

    bool getDisplaySizeOverride(size_t display, int* width, int* height)
    {
        (*width) = mBackBufferContainer->width;
        (*height) = mBackBufferContainer->height;
        return true;
    }
};

class FOpenGLCustomPresent : public FOSVRCustomPresent
{
public:
    FOpenGLCustomPresent(OSVR_ClientContext clientContext) :
        FOSVRCustomPresent(clientContext)
    {
    }

    virtual ~FOpenGLCustomPresent()
    {
        if (RenderTargetTexture > 0)
        {
            glDeleteTextures(1, &RenderTargetTexture);
        }

        if (mBackBufferContainer)
        {
            delete mBackBufferContainer;
        }
    }

protected:

    TArray<OSVR_RenderBufferOpenGL> mRenderBuffers;
    TArray<OSVR_RenderInfoOpenGL> mRenderInfos;
    OSVR_RenderManagerOpenGL mRenderManagerOpenGL = nullptr;
    GLuint RenderTargetTexture = 0;
    FUnrealBackBufferContainer* mBackBufferContainer = nullptr;

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
        // For OpenGL, we allow RenderManager to create our render textures for us and then
        // we pass that texture id in on the first RenderTexture_RenderThread call.

        // signal UpdateRenderBuffers to register the new render buffer when we
        // next get it passed in via LazySetSrcTexture
        bRenderBuffersNeedToUpdate = true;
        return false;
    }

    virtual void GetProjectionMatrixImpl(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip) override
    {
        OSVR_ReturnCode rc;
        OSVR_RenderInfoOpenGL renderInfo;
        rc = osvrRenderManagerGetRenderInfoFromCollectionOpenGL(mCachedProjectionRenderInfoCollection, eye, &renderInfo);
        check(rc == OSVR_RETURN_SUCCESS);

        // previously we divided these by renderInfo.projection.nearClip but we need
        // to pass these unmodified through to the OSVR_Projection_to_D3D call (and OpenGL
        // equivalent)
        left = static_cast<float>(renderInfo.projection.left);
        right = static_cast<float>(renderInfo.projection.right);
        top = static_cast<float>(renderInfo.projection.top);
        bottom = static_cast<float>(renderInfo.projection.bottom);
    }

    virtual bool CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY, float screenScale) override
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

            OSVR_RenderInfoCount numRenderInfo = { 0 };
            rc = osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo);
            check(rc == OSVR_RETURN_SUCCESS);

            mRenderInfos.Empty();
            for (size_t i = 0; i < numRenderInfo; i++)
            {
                OSVR_RenderInfoOpenGL renderInfo = { 0 };
                rc = osvrRenderManagerGetRenderInfoFromCollectionOpenGL(renderInfoCollection, i, &renderInfo);
                check(rc == OSVR_RETURN_SUCCESS);

                mRenderInfos.Add(renderInfo);
            }

            rc = osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection);
            check(rc == OSVR_RETURN_SUCCESS);

            // check some assumptions. Should all be the same height.
            check(mRenderInfos.Num() == 2);
            check(mRenderInfos[0].viewport.height == mRenderInfos[1].viewport.height);

            mRenderInfos[0].viewport.width = int(float(mRenderInfos[0].viewport.width) * screenScale);
            mRenderInfos[0].viewport.height = int(float(mRenderInfos[0].viewport.height) * screenScale);
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
            
            // We don't open the display here, because this might be called from the game thread
            // LazyOpenDisplay needs to be called on the rendering thread to open the display
            bInitialized = true;
        }
        return true;
    }

    virtual bool LazyOpenDisplayImpl() override
    {
        // we can assume we're initialized and running on the rendering thread
        // and we haven't already opened the display here (done in parent class)
        OSVR_OpenResultsOpenGL results;
        OSVR_ReturnCode rc = osvrRenderManagerOpenDisplayOpenGL(mRenderManagerOpenGL, &results);
        if (rc == OSVR_RETURN_FAILURE || results.status == OSVR_OPEN_STATUS_FAILURE)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("osvrRenderManagerOpenDisplayD3D11 call failed, or the result status was OSVR_OPEN_STATUS_FAILURE. Potential causes could be that the display is already open in direct mode with another app, or the display does not support direct mode"));
            return false;
        }
        return true;
    }
    
    virtual bool LazySetSrcTextureImpl(FTexture2DRHIParamRef srcTexture) override
    {
        auto textureOpenGL = static_cast<FOpenGLTexture2D*>(&(*srcTexture));
        RenderTargetTexture = textureOpenGL->Resource;
        UpdateRenderBuffers();
        return true;
    }

    virtual OSVR_Pose3 GetHeadPoseFromCachedRenderInfoCollectionImpl(OSVR_RenderInfoCollection renderInfoCollection, OSVR_RenderInfoCount index) override
    {
        check(IsInitialized());
        check(IsDisplayOpen());
        check(renderInfoCollection);

        OSVR_ReturnCode rc;
        OSVR_Pose3 ret = { 0 };
        OSVR_RenderInfoOpenGL renderInfo = { 0 };

        rc = osvrRenderManagerGetRenderInfoFromCollectionOpenGL(renderInfoCollection, index, &renderInfo);
        if (rc != OSVR_RETURN_SUCCESS)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::GetHeadPoseFromCachedRenderInfoCollectionImpl: osvrRenderManagerGetRenderInfoFromCollectionOpenGL failed with index %d"),
                index);
            return ret;
        }
        ret = renderInfo.pose;
        return ret;
    }

    virtual void RenderTexture_RenderThread(FRHICommandListImmediate& rhiCmdList, FTexture2DRHIParamRef backBuffer, FTexture2DRHIParamRef srcTexture) override
    {
        check(IsInRenderingThread());
        LazySetSrcTexture(srcTexture);
    }

    virtual void FinishRendering() override
    {
        check(IsInitialized());

        // @todo: this needs to be more robust.
        // Can we find this by inspection of the view somehow?
        // After the first call, the framebuffer ends up set to
        // 0 before entering this function, but 0 is not the
        // framebuffer for the window.
        if (mBackBufferContainer->displayFrameBuffer < 0)
        {
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mBackBufferContainer->displayFrameBuffer);
        }

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
            auto renderBuffer = mRenderBuffers[i];
            auto renderInfo = mRenderInfos[i];
            auto viewportDescription = mViewportDescriptions[i];
            rc = osvrRenderManagerPresentRenderBufferOpenGL(presentState, renderBuffer, renderInfo, viewportDescription);
            check(rc == OSVR_RETURN_SUCCESS);
        }

        // @todo: figure out why this call is failing (release DLLs make debugging
        // difficult here - can't determine which failure branch is being taken).
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

    virtual FUnrealBackBufferContainer* getBackBufferContainer()
    {
        if (!mBackBufferContainer)
        {
            mBackBufferContainer = new FUnrealBackBufferContainer();
            mBackBufferContainer->displayFrameBuffer = -1;
            mBackBufferContainer->width = 1280;
            mBackBufferContainer->height = 720;
        }
        return mBackBufferContainer;
    }

    virtual OSVR_GraphicsLibraryOpenGL CreateGraphicsLibrary()
    {
        OSVR_GraphicsLibraryOpenGL ret = { 0 };
        // @todo figure out why this toolkit doesn't get passed back when
        // we get the render info from the collection API.
        auto toolkit = new FUnrealOSVRRenderManagerOpenGLToolkit(getBackBufferContainer());
        ret.toolkit = toolkit->getToolkit();
        return ret;
    }
};


