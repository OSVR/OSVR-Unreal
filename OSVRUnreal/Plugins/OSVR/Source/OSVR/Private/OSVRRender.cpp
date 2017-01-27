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

// Must put path from Engine/Source to these includes since we are an out-of-tree module.
#include "Runtime/Renderer/Private/RendererPrivate.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"
#include "Runtime/Renderer/Private/PostProcess/PostProcessHMD.h"
#include "Runtime/Engine/Public/ScreenRendering.h"

void FOSVRHMD::DrawDistortionMesh_RenderThread(FRenderingCompositePassContext& Context, const FIntPoint& TextureSize)
{
    // shouldn't be called with a custom present
    check(0);
}

// Based off of the SteamVR Unreal Plugin implementation.
void FOSVRHMD::RenderTexture_RenderThread(FRHICommandListImmediate& rhiCmdList, FTexture2DRHIParamRef backBuffer, FTexture2DRHIParamRef srcTexture) const
{
    check(IsInRenderingThread());
    if (mCustomPresent && mCustomPresent->IsInitialized())
    {
        // the current custom present implementation may be allowing Unreal to
        // allocate its render textures. If so, this is the first time we get
        // access to the texture that was created.
        mCustomPresent->RenderTexture_RenderThread(rhiCmdList, backBuffer, srcTexture);
    }

    // @todo should we add an FCustomPresent function
    // to ask if we should draw the preview? The OpenGL
    // custom present can draw distortion directly into
    // the preview window for instance, making this
    // redundant.
    const uint32 viewportWidth = backBuffer->GetSizeX();
    const uint32 viewportHeight = backBuffer->GetSizeY();
    const uint32 textureWidth = srcTexture->GetSizeX();
    const uint32 textureHeight = srcTexture->GetSizeY();

    SetRenderTarget(rhiCmdList, backBuffer, FTextureRHIRef());
    rhiCmdList.SetViewport(0, 0, 0, viewportWidth, viewportHeight, 1.0f);

    rhiCmdList.SetBlendState(TStaticBlendState<>::GetRHI());
    rhiCmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
    rhiCmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());

    const auto featureLevel = GMaxRHIFeatureLevel;
    auto shaderMap = GetGlobalShaderMap(featureLevel);

    TShaderMapRef<FScreenVS> vertexShader(shaderMap);
    TShaderMapRef<FScreenPS> pixelShader(shaderMap);

    static FGlobalBoundShaderState boundShaderState;
    SetGlobalBoundShaderState(rhiCmdList, featureLevel, boundShaderState, RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *vertexShader, *pixelShader);

    pixelShader->SetParameters(rhiCmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), srcTexture);

#if !OSVR_UNREAL_4_14
    rhiCmdList.Clear(true, FLinearColor::Black, true, 0, true, 0, FIntRect());
#endif

    // @todo: do we need to ask mCustomPresent whether we should draw the preview or not?
    RendererModule->DrawRectangle(
        rhiCmdList,
        0, 0, // X, Y
        viewportWidth, viewportHeight, // SizeX, SizeY
        0.0f, 0.0f, // U, V
        1.0f, 1.0f, // SizeU, SizeV
        FIntPoint(viewportWidth, viewportHeight), // TargetSize
        FIntPoint(1, 1), // TextureSize
        *vertexShader,
        EDRF_Default);
}

void FOSVRHMD::GetEyeRenderParams_RenderThread(const struct FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
	if (Context.View.StereoPass == eSSP_LEFT_EYE)
	{
		EyeToSrcUVOffsetValue.X = 0.0f;
		EyeToSrcUVOffsetValue.Y = 0.0f;

		EyeToSrcUVScaleValue.X = 0.5f;
		EyeToSrcUVScaleValue.Y = 1.0f;
	}
	else
	{
		EyeToSrcUVOffsetValue.X = 0.5f;
		EyeToSrcUVOffsetValue.Y = 0.0f;

		EyeToSrcUVScaleValue.X = 0.5f;
		EyeToSrcUVScaleValue.Y = 1.0f;
	}
}

void FOSVRHMD::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
    check(IsInRenderingThread());
    if (mCustomPresent)
    {
        // @todo make Initialize lazy and remove this if block
        if (!mCustomPresent->IsInitialized())
        {
            mCustomPresent->Initialize();
        }

        mCustomPresent->StartRendering();

        FQuat lastHmdOrientation, hmdOrientation;
        FVector lastHmdPosition, hmdPosition;
        UpdateHeadPose(true, lastHmdOrientation, lastHmdPosition, hmdOrientation, hmdPosition);
        const FTransform oldRelativeTransform(lastHmdOrientation, lastHmdPosition);
        const FTransform newRelativeTransform(hmdOrientation, hmdPosition);

        ApplyLateUpdate(ViewFamily.Scene, oldRelativeTransform, newRelativeTransform);
    }
}

void FOSVRHMD::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& View)
{
    check(IsInRenderingThread());
    const FQuat deltaOriention = View.BaseHmdOrientation.Inverse() * CurHmdOrientationRT;
    View.ViewRotation = FRotator(View.ViewRotation.Quaternion() * deltaOriention);
    View.UpdateViewMatrix();
}

void FOSVRHMD::CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY)
{
    check(IsInGameThread());

    if (!IsStereoEnabled())
    {
        return;
    }

    float screenScale = GetScreenScale();
    
    if (mCustomPresent 
        && !mCustomPresent->IsInitialized() 
        && IsInRenderingThread() 
        && !mCustomPresent->Initialize())
    {
        mCustomPresent = nullptr;
    }

    if (mCustomPresent)
    {
        // this only happens once, the first time this is called. I don't know why.
        if (IsInRenderingThread())
        {
            mCustomPresent->LazyOpenDisplay();
            mCustomPresent->CalculateRenderTargetSize(InOutSizeX, InOutSizeY, screenScale);
        }
        else
        {
            auto leftEye = HMDDescription.GetDisplaySize(OSVRHMDDescription::LEFT_EYE);
            auto rightEye = HMDDescription.GetDisplaySize(OSVRHMDDescription::RIGHT_EYE);
            auto width = leftEye.X + rightEye.X;
            auto height = leftEye.Y;

            GetRenderTargetSize_GameThread(width, height, width, height);

            InOutSizeX = width;
            InOutSizeY = height;
            InOutSizeX = int(float(InOutSizeX) * screenScale);
            InOutSizeY = int(float(InOutSizeY) * screenScale);
        }
    }
    else
    {
        auto leftEye = HMDDescription.GetDisplaySize(OSVRHMDDescription::LEFT_EYE);
        auto rightEye = HMDDescription.GetDisplaySize(OSVRHMDDescription::RIGHT_EYE);
        InOutSizeX = leftEye.X + rightEye.X;
        InOutSizeY = leftEye.Y;
        InOutSizeX = int(float(InOutSizeX) * screenScale);
        InOutSizeY = int(float(InOutSizeY) * screenScale);
    }
}

bool FOSVRHMD::NeedReAllocateViewportRenderTarget(const FViewport &viewport)
{
    check(IsInGameThread());
    if (IsStereoEnabled())
    {
        const uint32 inSizeX = viewport.GetSizeXY().X;
        const uint32 inSizeY = viewport.GetSizeXY().Y;
        FIntPoint renderTargetSize;
        renderTargetSize.X = viewport.GetRenderTargetTexture()->GetSizeX();
        renderTargetSize.Y = viewport.GetRenderTargetTexture()->GetSizeY();

        uint32 newSizeX = inSizeX, newSizeY = inSizeY;
        CalculateRenderTargetSize(viewport, newSizeX, newSizeY);
        if (newSizeX != renderTargetSize.X || newSizeY != renderTargetSize.Y)
        {
            return true;
        }
    }
    return false;
}

void FOSVRHMD::UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& InViewport, class SViewport*)
{
    check(IsInGameThread());

    auto viewportRHI = InViewport.GetViewportRHI().GetReference();
    if (!mCustomPresent || (GIsEditor && !bPlaying) || (!IsStereoEnabled() && !bUseSeparateRenderTarget))
    {
        if (viewportRHI)
        {
            viewportRHI->SetCustomPresent(nullptr);
        }
        return;
    }

    if (mCustomPresent && mCustomPresent->IsInitialized())
    {
        if (!mCustomPresent->UpdateViewport(InViewport, viewportRHI))
        {
            delete mCustomPresent;
            mCustomPresent = nullptr;
        }
    }
}

bool FOSVRHMD::AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples)
{
    check(index == 0);
    check(IsInRenderingThread());
    if (mCustomPresent && (mCustomPresent->IsInitialized() || mCustomPresent->Initialize()))
    {
        return mCustomPresent->AllocateRenderTargetTexture(index, sizeX, sizeY, format, numMips, flags, targetableTextureFlags, outTargetableTexture, outShaderResourceTexture, numSamples);
    }
    return false;
}
