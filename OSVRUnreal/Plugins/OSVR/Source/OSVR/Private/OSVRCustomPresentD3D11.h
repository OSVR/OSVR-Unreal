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

#include "CoreMinimal.h"
#include "Misc/ScopeLock.h"

#if PLATFORM_WINDOWS

#include "IOSVR.h"
#include "OSVRCustomPresent.h"

#include "AllowWindowsPlatformTypes.h"
#include <osvr/RenderKit/RenderManagerD3D11C.h>
#include "HideWindowsPlatformTypes.h"
#include "Runtime/Windows/D3D11RHI/Private/D3D11RHIPrivate.h"

class FD3D11Texture2DSet : public FD3D11Texture2D
{
public:
    FD3D11Texture2DSet(
        class FD3D11DynamicRHI* inD3DRHI,
        const TArray<TRefCountPtr<ID3D11RenderTargetView> >& inRenderTargetViews,
        uint32 inSizeX,
        uint32 inSizeY,
        uint32 inNumMips,
        uint32 inNumSamples,
        EPixelFormat inFormat,
        uint32 inFlags
    ) : FD3D11Texture2D(
            inD3DRHI,
            nullptr,
            nullptr,
            false,
            1,
            inRenderTargetViews,
            NULL,
            inSizeX,
            inSizeY,
            0,
            inNumMips,
            inNumSamples,
            inFormat,
            false,
            inFlags,
            false,
            FClearValueBinding::None
        )
    {
    }

    void SwitchToNextElement()
    {
        currentIndex = (currentIndex + 1) % textures.Num();
        InitWithCurrentElement();
    }

    void AddTexture(ID3D11Texture2D* inTexture, ID3D11ShaderResourceView* inSRV, IDXGIKeyedMutex* keyedMutex, TArray<TRefCountPtr<ID3D11RenderTargetView> >* inRTVs = nullptr)
    {
        TextureElement element;
        inTexture->AddRef();
        element.texture = inTexture;
        element.srv = inSRV;
        if (inRTVs)
        {
            element.rtvs.Empty(inRTVs->Num());
            for (int32 i = 0; i < inRTVs->Num(); ++i)
            {
                element.rtvs.Add((*inRTVs)[i]);
            }
        }
        element.keyedMutex = keyedMutex;
        textures.Push(element);
    }

    void ReleaseResources()
    {
        for (int32 i = 0; i < textures.Num(); i++)
        {
            textures[i].texture->Release();
        }
        textures.Empty(0);
    }

    void RenderStart()
    {
        if (currentIndex < textures.Num()) {
            textures[currentIndex].keyedMutex->AcquireSync(0, 500);
        }
    }

    void RenderStop()
    {
        if (currentIndex < textures.Num()) {
            textures[currentIndex].keyedMutex->ReleaseSync(0);
        }
    }

    uint32 GetNumTextures()
    {
        return textures.Num();
    }

    void GetRenderTargetTexture(
        ID3D11Texture2D** colorBufferOut,
        ID3D11RenderTargetView** colorBufferViewOut)
    {
        GetRenderTargetTexture(currentIndex, colorBufferOut, colorBufferViewOut);
    }

    void GetRenderTargetTexture(
        int32 index, 
        ID3D11Texture2D** colorBufferOut,
        ID3D11RenderTargetView** colorBufferViewOut)
    {
        check(index < textures.Num());
        check(textures[index].rtvs.Num() == 1); // only one level supported now
        *colorBufferOut = textures[index].texture;
        *colorBufferViewOut = textures[index].rtvs[0];
    }

    static TRefCountPtr<FD3D11Texture2DSet> D3D11CreateTexture2DSet(
        uint32 width,
        uint32 height,
        uint32 numMipLevels,
        uint32 numSamples,
        EPixelFormat inFormat
    )
    {
        check(IsInRenderingThread());
        check(numMipLevels == 1); // only one mip level supported.
        uint32 flags = TexCreate_RenderTargetable | TexCreate_ShaderResource;
        auto D3D11RHI = static_cast<FD3D11DynamicRHI*>(GDynamicRHI);
        TArray<TRefCountPtr<ID3D11RenderTargetView> > textureSetRenderTargetViews;
        TRefCountPtr<FD3D11Texture2DSet> newTextureSet = new FD3D11Texture2DSet(
            D3D11RHI,
            textureSetRenderTargetViews,
            width,
            height,
            numMipLevels,
            numSamples,
            inFormat,
            flags
        );

        int texCount = 2;

        const bool bSRGB = (flags & TexCreate_SRGB) != 0;

        const DXGI_FORMAT platformResourceFormat = (DXGI_FORMAT)GPixelFormats[inFormat].PlatformFormat;
        const DXGI_FORMAT platformShaderResourceFormat = FindShaderResourceDXGIFormat(platformResourceFormat, bSRGB);
        const DXGI_FORMAT platformRenderTargetFormat = FindShaderResourceDXGIFormat(platformResourceFormat, bSRGB);

        D3D11_RTV_DIMENSION renderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        if (numSamples > 1)
        {
            renderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }

        auto graphicsDevice = reinterpret_cast<ID3D11Device*>(RHIGetNativeDevice());
        for (int32 i = 0; i < texCount; ++i)
        {
            TRefCountPtr<ID3D11Texture2D> pD3DTexture;

            D3D11_TEXTURE2D_DESC textureDesc;
            memset(&textureDesc, 0, sizeof(textureDesc));
            textureDesc.Width = width;
            textureDesc.Height = height;
            textureDesc.MipLevels = numMipLevels;
            textureDesc.ArraySize = 1;
            //textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            //textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.Format = platformResourceFormat;
            textureDesc.SampleDesc.Count = numSamples;
            textureDesc.SampleDesc.Quality = numSamples > 0 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            // We need it to be both a render target and a shader resource
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

            VERIFYD3D11RESULT_EX(graphicsDevice->CreateTexture2D(
                &textureDesc, NULL, pD3DTexture.GetInitReference()), D3D11RHI->GetDevice());

            IDXGIKeyedMutex* myMutex = nullptr;
            VERIFYD3D11RESULT_EX(pD3DTexture->QueryInterface(
                __uuidof(IDXGIKeyedMutex), (LPVOID*)&myMutex), D3D11RHI->GetDevice());

            TArray<TRefCountPtr<ID3D11RenderTargetView> > renderTargetViews;
            if (flags & TexCreate_RenderTargetable)
            {
                // Create a render target view for each mip
                for (uint32 mipIndex = 0; mipIndex < numMipLevels; mipIndex++)
                {
                    check(!(flags & TexCreate_TargetArraySlicesIndependently)); // not supported
                    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
                    FMemory::Memzero(&rtvDesc, sizeof(rtvDesc));
                    rtvDesc.Format = platformRenderTargetFormat;
                    rtvDesc.ViewDimension = renderTargetViewDimension;
                    rtvDesc.Texture2D.MipSlice = mipIndex;

                    TRefCountPtr<ID3D11RenderTargetView> renderTargetView;
                    VERIFYD3D11RESULT_EX(D3D11RHI->GetDevice()->CreateRenderTargetView(pD3DTexture, &rtvDesc, renderTargetView.GetInitReference()), D3D11RHI->GetDevice());
                    renderTargetViews.Add(renderTargetView);
                }
            }

            TRefCountPtr<ID3D11ShaderResourceView> shaderResourceView;

            // Create a shader resource view for the texture.
            if (flags & TexCreate_ShaderResource)
            {
                D3D_SRV_DIMENSION shaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.Format = platformShaderResourceFormat;
                srvDesc.ViewDimension = shaderResourceViewDimension;
                srvDesc.Texture2D.MostDetailedMip = numMipLevels - 1;//0;
                srvDesc.Texture2D.MipLevels = numMipLevels;

                VERIFYD3D11RESULT_EX(D3D11RHI->GetDevice()->CreateShaderResourceView(pD3DTexture, &srvDesc, shaderResourceView.GetInitReference()), D3D11RHI->GetDevice());

                check(IsValidRef(shaderResourceView));
            }

            newTextureSet->AddTexture(pD3DTexture, shaderResourceView, myMutex, &renderTargetViews);
        }

        if (flags & TexCreate_RenderTargetable)
        {
            newTextureSet->SetCurrentGPUAccess(EResourceTransitionAccess::EWritable);
        }

        newTextureSet->InitWithCurrentElement();
        return newTextureSet;
    }


protected:
    void InitWithCurrentElement()
    {
        Resource = textures[currentIndex].texture;
        ShaderResourceView = textures[currentIndex].srv;

        RenderTargetViews.Empty(textures[currentIndex].rtvs.Num());
        for (int32 i = 0; i < textures[currentIndex].rtvs.Num(); ++i)
        {
            RenderTargetViews.Add(textures[currentIndex].rtvs[i]);
        }
    }

    struct TextureElement
    {
        TRefCountPtr<ID3D11Texture2D> texture;
        TRefCountPtr<ID3D11ShaderResourceView> srv;
        TArray<TRefCountPtr<ID3D11RenderTargetView> > rtvs;
        IDXGIKeyedMutex* keyedMutex; // we don't own this pointer, so not ref counted
    };
    TArray<TextureElement> textures;

    int currentIndex = 0;
};

class FDirect3D11CustomPresent : public FOSVRCustomPresent
{
public:
    FDirect3D11CustomPresent(OSVR_ClientContext clientContext) :
        FOSVRCustomPresent(clientContext)
    {
    }

    virtual bool AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples = 1) override
    {
        check(IsInRenderingThread());

        FScopeLock lock(&mOSVRMutex);
        if (IsInitialized())
        {
            if (mRenderTargetTextureSet.IsValid())
            {
                mRenderTargetTextureSet->ReleaseResources();
            }

            mRenderTargetTextureSet = FD3D11Texture2DSet::D3D11CreateTexture2DSet(
                sizeX, sizeY, numMips, numSamples, PF_B8G8R8A8);

            if (!mRenderTargetTextureSet.IsValid())
            {
                return false;
            }

            outTargetableTexture = mRenderTargetTextureSet->GetTexture2D();
            outShaderResourceTexture = mRenderTargetTextureSet->GetTexture2D();
            bRenderBuffersNeedToUpdate = true;
            UpdateRenderBuffers();
            return true;
        }
        return false;
    }

protected:
    TRefCountPtr<FD3D11Texture2DSet> mRenderTargetTextureSet;

    TArray<OSVR_RenderBufferD3D11> mRenderBuffers;
    TArray<OSVR_RenderInfoD3D11> mRenderInfos;
    OSVR_RenderManagerD3D11 mRenderManagerD3D11 = nullptr;

    virtual void GetProjectionMatrixImpl(OSVR_RenderInfoCount eye, float &left, float &right, float &bottom, float &top, float nearClip, float farClip) override
    {
        check(IsInitialized());
        check(IsDisplayOpen());

        OSVR_ReturnCode rc;
        OSVR_RenderInfoD3D11 renderInfo;
        rc = osvrRenderManagerGetRenderInfoFromCollectionD3D11(mCachedProjectionRenderInfoCollection, eye, &renderInfo);
        if (rc != OSVR_RETURN_SUCCESS)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::GetProjectionMatrixImpl: osvrRenderManagerGetRenderInfoFromCollectionD3D11 failed with index %d"),
                eye);
            return;
        }

        // previously we divided these by renderInfo.projection.nearClip but we need
        // to pass these unmodified through to the OSVR_Projection_to_D3D call (and OpenGL
        // equivalent)
        left = static_cast<float>(renderInfo.projection.left);
        right = static_cast<float>(renderInfo.projection.right);
        top = static_cast<float>(renderInfo.projection.top);
        bottom = static_cast<float>(renderInfo.projection.bottom);
    }

    virtual OSVR_Pose3 GetHeadPoseFromCachedRenderInfoCollectionImpl(OSVR_RenderInfoCollection renderInfoCollection, OSVR_RenderInfoCount index) override
    {
        check(IsInitialized());
        check(IsDisplayOpen());
        check(renderInfoCollection);

        OSVR_ReturnCode rc;
        OSVR_Pose3 ret = { 0 };
        OSVR_RenderInfoD3D11 renderInfo = { 0 };

        rc = osvrRenderManagerGetRenderInfoFromCollectionD3D11(renderInfoCollection, index, &renderInfo);
        if (rc != OSVR_RETURN_SUCCESS)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::GetHeadPoseFromCachedRenderInfoCollectionImpl: osvrRenderManagerGetRenderInfoFromCollectionD3D11 failed with index %d"),
                index);
            return ret;
        }
        ret = renderInfo.pose;
        return ret;
    }

    virtual bool CalculateRenderTargetSizeImpl(uint32& InOutSizeX, uint32& InOutSizeY, float screenScale) override
    {
        check(IsInRenderingThread());
        check(IsInitialized());

        if (InitializeImpl())
        {
            // Should we create a RenderParams?
            OSVR_ReturnCode rc;

            rc = osvrRenderManagerGetDefaultRenderParams(&mRenderParams);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("FDirect3D11CustomPresent::CalculateRenderTargetSizeImpl: osvrRenderManagerGetDefaultRenderParams call failed."));
                return false;
            }

            OSVR_RenderInfoCount numRenderInfo;
            OSVR_RenderInfoCollection renderInfoCollection = { 0 };
            rc = osvrRenderManagerGetRenderInfoCollection(mRenderManager, mRenderParams, &renderInfoCollection);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("FDirect3D11CustomPresent::CalculateRenderTargetSizeImpl: osvrRenderManagerGetRenderInfoCollection call failed."));
                return false;
            }

            rc = osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("FDirect3D11CustomPresent::CalculateRenderTargetSizeImpl: osvrRenderManagerGetNumRenderInfoInCollection call failed."));
                return false;
            }

            if (numRenderInfo != 2)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, 
                    TEXT("FDirect3D11CustomPresent::CalculateRenderTargetSizeImpl: expecting 2 render infos from osvrRenderManagerGetNumRenderInfoInCollection. Got %d."), 
                    numRenderInfo);
                return false;
            }

            mRenderInfos.Empty();
            for (size_t i = 0; i < numRenderInfo; i++)
            {
                OSVR_RenderInfoD3D11 renderInfo;
                rc = osvrRenderManagerGetRenderInfoFromCollectionD3D11(renderInfoCollection, i, &renderInfo);
                if (rc != OSVR_RETURN_SUCCESS)
                {
                    UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("FDirect3D11CustomPresent::CalculateRenderTargetSizeImpl: osvrRenderManagerGetRenderInfoFromCollectionD3D11 call failed."));
                    return false;
                }

                mRenderInfos.Add(renderInfo);
            }

            // check some assumptions. Should all be the same height.
            if (mRenderInfos[0].viewport.height != mRenderInfos[1].viewport.height)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning, TEXT("FDirect3D11CustomPresent::CalculateRenderTargetSizeImpl: viewports from both OSVR_RenderInfoD3D11's should be the same height."));
                return false;
            }

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
        check(IsInRenderingThread());

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

            rc = osvrCreateRenderManagerD3D11(mClientContext, TCHAR_TO_ANSI(*graphicsLibraryName), graphicsLibrary, &mRenderManager, &mRenderManagerD3D11);
            if (rc == OSVR_RETURN_FAILURE || !mRenderManager || !mRenderManagerD3D11)
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

            // The display is not opened here, but in a separate call (LazyOpenDisplay)
            // because we may be in the game thread here

            bInitialized = true;
        }
        return true;
    }

    virtual bool LazyOpenDisplayImpl() override
    {
        check(IsInRenderingThread());
        check(IsInitialized());

        // we can assume we're initialized and running on the rendering thread
        // and we haven't already opened the display here (done in parent class)
        OSVR_OpenResultsD3D11 results;
        OSVR_ReturnCode rc = osvrRenderManagerOpenDisplayD3D11(mRenderManagerD3D11, &results);
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
        // The Direct3D11 implementation can allocate the texture right away,
        // so we don't need to defer setting this.
        return true;
    }

    virtual void FinishRendering() override
    {
        check(IsInitialized());
        check(IsInRenderingThread());

        if (!mCachedRenderThreadRenderInfoCollection)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::FinishRendering() - mCachedRenderThreadRenderInfoCollection is null. Should be non-null at this point."));
            return;
        }

        if (!mRenderTargetTextureSet.IsValid())
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::FinishRendering() - mRenderTargetTextureSet is not initialized. Skipping this frame."));
            return;
        }

        mRenderTargetTextureSet->RenderStop();

        // all of the render manager samples keep the flipY at the default false,
        // for both OpenGL and DirectX. Is this even needed anymore?
        OSVR_ReturnCode rc = OSVR_RETURN_SUCCESS;
        OSVR_RenderManagerPresentState presentState;
        rc = osvrRenderManagerStartPresentRenderBuffers(&presentState);
        if (rc != OSVR_RETURN_SUCCESS)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::FinishRendering() - osvrRenderManagerStartPresentRenderBuffers call failed."));
        }
        check(mRenderBuffers.Num() == mRenderInfos.Num() && mRenderBuffers.Num() == mViewportDescriptions.Num());
        for (int32 i = 0; i < mRenderBuffers.Num(); i++)
        {
            OSVR_RenderInfoD3D11 renderInfo = { 0 };
            rc = osvrRenderManagerGetRenderInfoFromCollectionD3D11(mCachedRenderThreadRenderInfoCollection, i, &renderInfo);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning,
                    TEXT("FDirect3D11CustomPresent::FinishRendering() - osvrRenderManagerGetRenderInfoFromCollectionD3D11 call failed on i = %d."), i);
                renderInfo = mRenderInfos[i];
            }
            ID3D11Texture2D* texture;
            ID3D11RenderTargetView* rtv;
            mRenderTargetTextureSet->GetRenderTargetTexture(&texture, &rtv);
            OSVR_RenderBufferD3D11 buffer = { 0 };
            buffer.colorBuffer = texture;
            buffer.colorBufferView = rtv;

            rc = osvrRenderManagerPresentRenderBufferD3D11(presentState, buffer, renderInfo, mViewportDescriptions[i]);
            if (rc != OSVR_RETURN_SUCCESS)
            {
                UE_LOG(FOSVRCustomPresentLog, Warning,
                    TEXT("FDirect3D11CustomPresent::FinishRendering() - osvrRenderManagerPresentRenderBufferD3D11 call failed on i = %d."), i);
                break;
            }
        }

        rc = osvrRenderManagerFinishPresentRenderBuffers(mRenderManager, presentState, mRenderParams, ShouldFlipY() ? OSVR_TRUE : OSVR_FALSE);
        if (rc != OSVR_RETURN_SUCCESS)
        {
            UE_LOG(FOSVRCustomPresentLog, Warning,
                TEXT("FDirect3D11CustomPresent::FinishRendering() - osvrRenderManagerFinishPresentRenderBuffers call failed."));
        }

        mRenderTargetTextureSet->SwitchToNextElement();
    }

    virtual void UpdateRenderBuffers() override
    {
        HRESULT hr;
        check(IsInRenderingThread());
        check(IsInitialized());
        
        if (bRenderBuffersNeedToUpdate && mRenderTargetTextureSet.IsValid())
        {
            // @todo why are we checking bDisplayOpen if LazyOpenDisplayImpl is lazy?
            if (!bDisplayOpen)
            {
                bDisplayOpen = LazyOpenDisplayImpl();
            }

            mRenderBuffers.Empty();

            // Adding two RenderBuffers, but they both point to the same D3D11 texture target
            uint32 numBuffers = mRenderTargetTextureSet->GetNumTextures();
            check(numBuffers == 2);
            for (uint32 i = 0; i < numBuffers; i++)
            {
                OSVR_RenderBufferD3D11 buffer;
                ID3D11Texture2D* colorBuffer;
                ID3D11RenderTargetView* colorBufferView;
                mRenderTargetTextureSet->GetRenderTargetTexture(i, &colorBuffer, &colorBufferView);

                buffer.colorBuffer = colorBuffer;
                //buffer.colorBufferView = renderTargetView;
                buffer.colorBufferView = colorBufferView;
                //buffer.depthStencilBuffer = ???;
                //buffer.depthStencilView = ???;
                mRenderBuffers.Add(buffer);
            }

            // We need to register these new buffers.

            {
                OSVR_RenderManagerRegisterBufferState state;
                hr = osvrRenderManagerStartRegisterRenderBuffers(&state);
                check(hr == OSVR_RETURN_SUCCESS);

                for (int32 i = 0; i < mRenderBuffers.Num(); i++)
                {
                    hr = osvrRenderManagerRegisterRenderBufferD3D11(state, mRenderBuffers[i]);
                    check(hr == OSVR_RETURN_SUCCESS);
                }

                hr = osvrRenderManagerFinishRegisterRenderBuffers(mRenderManager, state, true);
                check(hr == OSVR_RETURN_SUCCESS);
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

    virtual OSVR_GraphicsLibraryD3D11 CreateGraphicsLibrary()
    {
        check(IsInRenderingThread());

        OSVR_GraphicsLibraryD3D11 ret;
        // Put the device and context into a structure to let RenderManager
        // know to use this one rather than creating its own.
        ret.device = GetGraphicsDevice<ID3D11Device>();
        ID3D11DeviceContext *ctx = NULL;
        ret.device->GetImmediateContext(&ctx);
        check(ctx);
        ret.context = ctx;

        return ret;
    }

    virtual FString GetGraphicsLibraryName() override
    {
        return FString("Direct3D11");
    }

    virtual bool ShouldFlipY() override
    {
        return false;
    }

    virtual void StartRendering() override
    {
        if (mRenderTargetTextureSet.IsValid())
        {
            mRenderTargetTextureSet->RenderStart();
        }
    }
};

#endif // #if PLATFORM_WINDOWS
