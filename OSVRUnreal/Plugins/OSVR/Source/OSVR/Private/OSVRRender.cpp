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
//#include "RendererPrivate.h"
//#include "ScenePrivate.h"
//#include "PostProcess/PostProcessHMD.h"

void FOSVRHMD::DrawDistortionMesh_RenderThread(FRenderingCompositePassContext& Context, const FSceneView& View, const FIntPoint& TextureSize)
{
	// @TODO
}

void FOSVRHMD::GetEyeRenderParams_RenderThread(EStereoscopicPass StereoPass, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
	if (StereoPass == eSSP_LEFT_EYE)
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

void FOSVRHMD::GetTimewarpMatrices_RenderThread(EStereoscopicPass StereoPass, FMatrix& EyeRotationStart, FMatrix& EyeRotationEnd) const
{
	// @TODO
}

void FOSVRHMD::PreRenderViewFamily_RenderThread(FSceneViewFamily& ViewFamily)
{
	// @TODO
}

void FOSVRHMD::PreRenderView_RenderThread(FSceneView& View)
{
	// @TODO
}

void FOSVRHMD::FinishRenderingFrame_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	// @TODO
}

void FOSVRHMD::UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& InViewport, class SViewport*)
{
	// @TODO
}
