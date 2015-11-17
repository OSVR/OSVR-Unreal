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

#include <cmath>

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

        double horizontalFOV = FMath::RadiansToDegrees(std::atan(std::abs(left)) + std::atan(std::abs(right)));
        double verticalFOV = FMath::RadiansToDegrees(std::atan(std::abs(top)) + std::atan(std::abs(bottom)));
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

// implemented to match the steamvr projection calculation but with OSVR calculated clipping planes.
FMatrix OSVRHMDDescription::GetProjectionMatrix(EEye Eye, OSVR_DisplayConfig displayConfig) const
{
    OSVR_EyeCount eye = (Eye == LEFT_EYE ? 0 : 1);
    double left, right, bottom, top;
    OSVR_ReturnCode rc;
    rc = osvrClientGetViewerEyeSurfaceProjectionClippingPlanes(displayConfig, 0, eye, 0, &left, &right, &bottom, &top);
    check(rc == OSVR_RETURN_SUCCESS);

    // not sure if these are needed coming from OSVR. SteamVR does this, but no documentation as to why
    bottom *= -1.0f;
    top *= -1.0f;
    right *= -1.0f;
    left *= -1.0f;

    // sanity check: what is going on with this projection matrix?
    // no reference to far clipping plane. This looks nothing like glFrustum.
    // matches their occulus rift calculation in the parts that they correct for unreal though
    // ([3][3] = 0, [2][3] = 1, [2][2] = 0, [3][3] = GNearClippingPlane)
    float zNear = GNearClippingPlane;
    float sumRightLeft = static_cast<float>(right + left);
    float sumTopBottom = static_cast<float>(top + bottom);
    float inverseRightLeft = 1.0f / static_cast<float>(right - left);
    float inverseTopBottom = 1.0f / static_cast<float>(top - bottom);
    FPlane row1(2.0f * inverseRightLeft, 0.0f, 0.0f, 0.0f);
    FPlane row2(0.0f, 2.0f * inverseTopBottom, 0.0f, 0.0f);
    FPlane row3(sumRightLeft * inverseRightLeft, sumTopBottom * inverseTopBottom, 0.0f, 1.0f);
    FPlane row4(0.0f, 0.0f, zNear, 0.0f);
    FMatrix ret = FMatrix(row1, row2, row3, row4);
    return ret;
}

float OSVRHMDDescription::GetInterpupillaryDistance() const
{
    return m_ipd;
}
