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
#include "OSVRHMDDescription.h"

#include "Json.h"

struct DescriptionData
{
	FVector2D DisplaySize[2];
	FVector2D Fov[2];

	DescriptionData();
};

static DescriptionData& GetData(void* This)
{
	return *reinterpret_cast< DescriptionData* >(This);
}

DescriptionData::DescriptionData()
{
	// Set defaults...
	for (int i = 0; i < 2; ++i)
	{
		DisplaySize[i].Set(960, 1080);
		Fov[i].Set(90, 101.25f);
	}
}

OSVRHMDDescription::OSVRHMDDescription()
	: Valid(false),
	  Data(new DescriptionData())
{
}

OSVRHMDDescription::~OSVRHMDDescription()
{
	delete Data;
}

bool OSVRHMDDescription::OSVRViewerFitsUnrealModel(OSVR_DisplayConfig displayConfig) {
    // if the display config hasn't started up, we can't tell yet
    if (osvrClientCheckDisplayStartup(displayConfig) == OSVR_RETURN_FAILURE) {
        return false;
    }

    OSVR_ReturnCode returnCode;
    
    // must be only one display input
    OSVR_DisplayInputCount numDisplayInputs;
    returnCode = osvrClientGetNumDisplayInputs(displayConfig, &numDisplayInputs);
    if (returnCode == OSVR_RETURN_FAILURE || numDisplayInputs != 1) {
        return false;
    }

    // must be only one viewer
    OSVR_ViewerCount numViewers;
    returnCode = osvrClientGetNumViewers(displayConfig, &numViewers);
    if (returnCode == OSVR_RETURN_FAILURE || numViewers != 1) {
        return false;
    }

    // the one viewer must have two eyes
    OSVR_EyeCount numEyes;
    returnCode = osvrClientGetNumEyesForViewer(displayConfig, 0, &numEyes);
    if (returnCode == OSVR_RETURN_FAILURE || numEyes != 2) {
        return false;
    }

    // each eye must have only one surface

    // left eye
    OSVR_ViewerCount numLeftEyeSurfaces, numRightEyeSurfaces;
    returnCode = osvrClientGetNumSurfacesForViewerEye(displayConfig, 0, 0, &numLeftEyeSurfaces);
    if (returnCode == OSVR_RETURN_FAILURE || numLeftEyeSurfaces != 1) {
        return false;
    }

    // right eye
    returnCode = osvrClientGetNumSurfacesForViewerEye(displayConfig, 0, 1, &numRightEyeSurfaces);
    if (returnCode == OSVR_RETURN_FAILURE || numRightEyeSurfaces != 1) {
        return false;
    }

    // I think we're good.
    return true;
}

void OSVRHMDDescription::InitIPD(OSVR_DisplayConfig displayConfig) {
    OSVR_Pose3 leftEye, rightEye;
    OSVR_ReturnCode returnCode;

    returnCode = osvrClientGetViewerEyePose(displayConfig, 0, 0, &leftEye);
    check(returnCode == OSVR_RETURN_SUCCESS);

    returnCode = osvrClientGetViewerEyePose(displayConfig, 0, 1, &rightEye);
    check(returnCode == OSVR_RETURN_SUCCESS);

    double dx = leftEye.translation.data[0] - rightEye.translation.data[0];
    double dy = leftEye.translation.data[1] - rightEye.translation.data[1];
    double dz = leftEye.translation.data[2] - rightEye.translation.data[2];

    m_ipd = std::sqrt(dx * dx + dy * dy + dz * dz);
}

void OSVRHMDDescription::InitDisplaySize(OSVR_DisplayConfig displayConfig) {
    OSVR_ReturnCode returnCode;

    // left eye surface (only one surface per eye supported)
    OSVR_ViewportDimension leftViewportLeft, leftViewportBottom, leftViewportWidth, leftViewportHeight;
    returnCode = osvrClientGetRelativeViewportForViewerEyeSurface(displayConfig, 0, 0, 0,
        &leftViewportLeft, &leftViewportBottom, &leftViewportWidth, &leftViewportHeight);
    check(returnCode == OSVR_RETURN_SUCCESS);

    // right eye surface (only one surface per eye supported)
    OSVR_ViewportDimension rightViewportLeft, rightViewportBottom, rightViewportWidth, rightViewportHeight;
    returnCode = osvrClientGetRelativeViewportForViewerEyeSurface(displayConfig, 0, 1, 0,
        &rightViewportLeft, &rightViewportBottom, &rightViewportWidth, &rightViewportHeight);
    check(returnCode == OSVR_RETURN_SUCCESS);

    auto data = GetData(Data);
    data.DisplaySize[0] = FVector2D(leftViewportWidth, leftViewportHeight);
    data.DisplaySize[1] = FVector2D(rightViewportWidth, rightViewportHeight);
}

void OSVRHMDDescription::InitFOV(OSVR_DisplayConfig displayConfig) {
    OSVR_ReturnCode returnCode;
    for (OSVR_EyeCount eye = 0; eye < 2; eye++) {
        double left, right, top, bottom;
        returnCode = osvrClientGetViewerEyeSurfaceProjectionClippingPlanes(displayConfig, 0, 0, eye, &left, &right, &bottom, &top);
        check(returnCode == OSVR_RETURN_SUCCESS);

        double horizontalFOV = FMath::RadiansToDegrees(atan(std::abs(left)) + atan(std::abs(right)));
        double verticalFOV = FMath::RadiansToDegrees(atan(std::abs(top)) + atan(std::abs(bottom)));
        auto data = GetData(Data);
        data.Fov[eye] = FVector2D(horizontalFOV, verticalFOV);
    }
}

bool OSVRHMDDescription::Init(OSVR_ClientContext OSVRClientContext, OSVR_DisplayConfig displayConfig)
{
	Valid = false;

    auto data = GetData(Data);
    
    // if the OSVR viewer doesn't fit nicely with the Unreal HMD model, don't
    // bother trying to fill everything else out.
    if (!OSVRViewerFitsUnrealModel(displayConfig)) {
        return false;
    }

    InitIPD(displayConfig);
    InitDisplaySize(displayConfig);
    InitFOV(displayConfig);
    Valid = true;
	return Valid;
}

FVector2D OSVRHMDDescription::GetDisplaySize(EEye Eye) const
{
	return GetData(Data).DisplaySize[Eye];
}

FVector2D OSVRHMDDescription::GetFov(OSVR_EyeCount Eye) const
{
	return GetData(Data).Fov[Eye];
}
FVector2D OSVRHMDDescription::GetFov(EEye Eye) const
{
    return GetData(Data).Fov[Eye];
}

FMatrix OSVRHMDDescription::GetProjectionMatrix(EEye Eye) const
{
	// @TODO: a proper stereo projection matrix should be calculated

	const float ProjectionCenterOffset = 0.151976421f;
	const float PassProjectionOffset = (Eye == LEFT_EYE) ? ProjectionCenterOffset : -ProjectionCenterOffset;

#if 1
	const float HalfFov = FMath::DegreesToRadians(GetFov(Eye).X) / 2.f;
	const float InWidth = GetDisplaySize(Eye).X;
	const float InHeight = GetDisplaySize(Eye).Y;
	const float XS = 1.0f / tan(HalfFov);
	const float YS = InWidth / tan(HalfFov) / InHeight;
#else
	const float HalfFov = 2.19686294f / 2.f;
	const float InWidth = 640.f;
	const float InHeight = 480.f;
	const float XS = 1.0f / tan(HalfFov);
	const float YS = InWidth / tan(HalfFov) / InHeight;
#endif

	const float InNearZ = GNearClippingPlane;
	return FMatrix(
			   FPlane(XS, 0.0f, 0.0f, 0.0f),
			   FPlane(0.0f, YS, 0.0f, 0.0f),
			   FPlane(0.0f, 0.0f, 0.0f, 1.0f),
			   FPlane(0.0f, 0.0f, InNearZ, 0.0f))

		   *
		   FTranslationMatrix(FVector(PassProjectionOffset, 0, 0));
}

float OSVRHMDDescription::GetInterpupillaryDistance() const
{
    return m_ipd;
}
