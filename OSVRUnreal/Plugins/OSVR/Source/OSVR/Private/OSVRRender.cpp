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

void FOSVRHMD::DrawDistortionMesh_RenderThread(FRenderingCompositePassContext& Context, const FIntPoint& TextureSize)
{
    // shouldn't be called with a custom present
    check(0);
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

void FOSVRHMD::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
    check(IsInRenderingThread());
    mCustomPresent->Initialize();
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

    mCustomPresent->CalculateRenderTargetSize(Viewport, InOutSizeX, InOutSizeY);
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
    }

    mCustomPresent->UpdateViewport(InViewport, viewportRHI);
}
