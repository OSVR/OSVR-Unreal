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

void FOSVRHMD::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIParamRef BackBuffer, FTexture2DRHIParamRef SrcTexture) const
//{
//    check(IsInRenderingThread());
//
//    if (DstRect.IsEmpty())
//    {
//        DstRect = FIntRect(0, 0, DstTexture->GetSizeX(), DstTexture->GetSizeY());
//    }
//    const uint32 ViewportWidth = DstRect.Width();
//    const uint32 ViewportHeight = DstRect.Height();
//    const FIntPoint TargetSize(ViewportWidth, ViewportHeight);
//
//    const float SrcTextureWidth = SrcTexture->GetSizeX();
//    const float SrcTextureHeight = SrcTexture->GetSizeY();
//    float U = 0.f, V = 0.f, USize = 1.f, VSize = 1.f;
//    if (!SrcRect.IsEmpty())
//    {
//        U = SrcRect.Min.X / SrcTextureWidth;
//        V = SrcRect.Min.Y / SrcTextureHeight;
//        USize = SrcRect.Width() / SrcTextureWidth;
//        VSize = SrcRect.Height() / SrcTextureHeight;
//    }
//
//    SetRenderTarget(RHICmdList, DstTexture, FTextureRHIRef());
//    RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);
//
//    RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
//    RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
//    RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
//
//    const auto FeatureLevel = GMaxRHIFeatureLevel;
//    auto ShaderMap = GetGlobalShaderMap(FeatureLevel);
//
//    TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
//    TShaderMapRef<FScreenPS> PixelShader(ShaderMap);
//
//    static FGlobalBoundShaderState BoundShaderState;
//    SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState, RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);
//
//    PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);
//
//    RendererModule->DrawRectangle(
//        RHICmdList,
//        0, 0,
//        ViewportWidth, ViewportHeight,
//        U, V,
//        USize, VSize,
//        TargetSize,
//        FIntPoint(1, 1),
//        *VertexShader,
//        EDRF_Default);
//}
{
    check(IsInRenderingThread());

    const uint32 ViewportWidth = BackBuffer->GetSizeX();
    const uint32 ViewportHeight = BackBuffer->GetSizeY();

    SetRenderTarget(RHICmdList, BackBuffer, FTextureRHIRef());
    RHICmdList.SetViewport(0, 0, 0, ViewportWidth, ViewportHeight, 1.0f);

    RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
    RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
    RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());

    const auto FeatureLevel = GMaxRHIFeatureLevel;
    auto ShaderMap = GetGlobalShaderMap(FeatureLevel);

    TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
    TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

    static FGlobalBoundShaderState BoundShaderState;
    SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState, RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);

    PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);
    //if (WindowMirrorMode == 1)
    //{
    //    // need to clear when rendering only one eye since the borders won't be touched by the DrawRect below
    //    RHICmdList.Clear(true, FLinearColor::Black, false, 0, false, 0, FIntRect());

    //    RendererModule->DrawRectangle(
    //        RHICmdList,
    //        ViewportWidth / 4, 0,
    //        ViewportWidth / 2, ViewportHeight,
    //        0.1f, 0.2f,
    //        0.3f, 0.6f,
    //        FIntPoint(ViewportWidth, ViewportHeight),
    //        FIntPoint(1, 1),
    //        *VertexShader,
    //        EDRF_Default);
    //}
    //else if (WindowMirrorMode == 2)
    //{
        RendererModule->DrawRectangle(
            RHICmdList,
            0, 0, // X, Y
            ViewportWidth, ViewportHeight, // SizeX, SizeY
            0.0f, 0.0f, // U, V
            1.0f, 1.0f, // SizeU, SizeV
            FIntPoint(ViewportWidth, ViewportHeight), // TargetSize
            FIntPoint(1, 1), // TextureSize
            *VertexShader,
            EDRF_Default);
    //}
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

void FOSVRHMD::GetTimewarpMatrices_RenderThread(const struct FRenderingCompositePassContext& Context, FMatrix& EyeRotationStart, FMatrix& EyeRotationEnd) const
{
	// intentionally left blank
}

// @todo: Why is this function never called?
void FOSVRHMD::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
    check(IsInRenderingThread());
    if (mCustomPresent) {
        mCustomPresent->Initialize();
    }
    // steamVR updates the current pose here, should we?
}

void FOSVRHMD::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& View)
{
    // @todo

}

void FOSVRHMD::CalculateRenderTargetSize(const FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY)
{
    check(IsInGameThread());

    if (!IsStereoEnabled()) {
        return;
    }

    if (mCustomPresent) {
        mCustomPresent->CalculateRenderTargetSize(InOutSizeX, InOutSizeY);
    }
}


bool FOSVRHMD::NeedReAllocateViewportRenderTarget(const FViewport &viewport) {
    check(IsInGameThread());
    if (IsStereoEnabled()) {
        const uint32 inSizeX = viewport.GetSizeXY().X;
        const uint32 inSizeY = viewport.GetSizeXY().Y;
        FIntPoint renderTargetSize;
        renderTargetSize.X = viewport.GetRenderTargetTexture()->GetSizeX();
        renderTargetSize.Y = viewport.GetRenderTargetTexture()->GetSizeY();

        uint32 newSizeX = inSizeX, newSizeY = inSizeY;
        CalculateRenderTargetSize(viewport, newSizeX, newSizeY);
        if (newSizeX != renderTargetSize.X || newSizeY != renderTargetSize.Y) {
            return true;
        }
    }
    return false;
}

void FOSVRHMD::UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& InViewport, class SViewport*)
{
    check(IsInGameThread());

    auto viewportRHI = InViewport.GetViewportRHI().GetReference();
    if (!IsStereoEnabled()) {
        if (!bUseSeparateRenderTarget) {
            viewportRHI->SetCustomPresent(nullptr);
        }
        return;
    }

    if (mCustomPresent) {
        mCustomPresent->UpdateViewport(InViewport, viewportRHI);
    }
}

bool FOSVRHMD::AllocateRenderTargetTexture(uint32 index, uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, FTexture2DRHIRef& outTargetableTexture, FTexture2DRHIRef& outShaderResourceTexture, uint32 numSamples)
{
    check(index == 0);
    if (mCustomPresent) {
        return mCustomPresent->AllocateRenderTargetTexture(index, sizeX, sizeY, format, numMips, flags, targetableTextureFlags, outTargetableTexture, outShaderResourceTexture, numSamples);
    }
    return false;
}
