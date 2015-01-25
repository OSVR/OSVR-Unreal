
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

void FOSVRHMD::PreRenderViewFamily_RenderThread(FSceneViewFamily& ViewFamily, uint32 InFrameNumber)
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
