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
	FVector2D DisplayOrigin[2];
	FVector2D Fov[2];
	FVector Location[2];
	FString PositionalTrackerInterface[2];

	DescriptionData();

	bool InitFromJSON(const char* JSON);
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
		DisplayOrigin[i].Set(i == 0 ? 0 : 960, 0);
		Fov[i].Set(90, 101.25f);
		Location[i].Set(i == 0 ? -0.0325 : 0.0325, 0, 0);
		PositionalTrackerInterface[i] = "/me/head";
	}
}

bool DescriptionData::InitFromJSON(const char* JSON)
{
	TSharedPtr< FJsonObject > JsonObject;
	auto JsonReader = TJsonReaderFactory< CHAR >::Create(JSON);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
		JsonObject.IsValid())
	{
		auto hmdJson = JsonObject->GetObjectField("hmd");
		auto resolutionsJson = hmdJson->GetArrayField("resolutions");
		auto fieldOfViewJson = hmdJson->GetObjectField("field_of_view");
		for (int i = 0; i < 2; ++i)
		{
			//set resolution
			DisplaySize[i].X = resolutionsJson[0]->AsObject()->GetNumberField("width") * 0.5f;
			DisplaySize[i].Y = resolutionsJson[0]->AsObject()->GetNumberField("height");
			DisplayOrigin[i].Set(i == 0 ? 0 : DisplaySize[i].X, 0);
			//set field of view
			Fov[i].X = fieldOfViewJson->GetNumberField("monocular_horizontal");
			Fov[i].Y = fieldOfViewJson->GetNumberField("monocular_vertical");
			//set default IPD for now. It's not coming from /display.
			Location[i].Set(i == 0 ? -0.0325 : 0.0325, 0, 0);
			PositionalTrackerInterface[i] = "/me/head";
		}
	}
	return true;
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

bool OSVRHMDDescription::Init(OSVR_ClientContext OSVRClientContext)
{
	Valid = false;

	static const char* Path = "/display";

	size_t Length;
	if (osvrClientGetStringParameterLength(OSVRClientContext, Path, &Length) == OSVR_RETURN_FAILURE)
		return false;

	FMemMark MemStackMark(FMemStack::Get());

	char* Buffer = new (FMemStack::Get()) char[Length];

	if (osvrClientGetStringParameter(OSVRClientContext, Path, Buffer, Length) == OSVR_RETURN_FAILURE)
		return false;

	Valid = GetData(Data).InitFromJSON(Buffer);

	return Valid;
}

FVector2D OSVRHMDDescription::GetDisplaySize(EEye Eye) const
{
	return GetData(Data).DisplaySize[Eye];
}

FVector2D OSVRHMDDescription::GetDisplayOrigin(EEye Eye) const
{
	return GetData(Data).DisplayOrigin[Eye];
}

FVector2D OSVRHMDDescription::GetFov(EEye Eye) const
{
	return GetData(Data).Fov[Eye];
}

FVector OSVRHMDDescription::GetLocation(EEye Eye) const
{
	return GetData(Data).Location[Eye];
}

FString OSVRHMDDescription::GetPositionalTrackerInterface(EEye Eye) const
{
	return GetData(Data).PositionalTrackerInterface[Eye];
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

void OSVRHMDDescription::GetMonitorInfo(IHeadMountedDisplay::MonitorInfo& MonitorDesc) const
{
	MonitorDesc.MonitorName = "OSVR-Display"; //@TODO
	MonitorDesc.MonitorId = 0;				  //@TODO
	MonitorDesc.DesktopX = GetDisplayOrigin(OSVRHMDDescription::LEFT_EYE).X;
	MonitorDesc.DesktopY = GetDisplayOrigin(OSVRHMDDescription::LEFT_EYE).Y;
	MonitorDesc.ResolutionX = GetResolution().X;
	MonitorDesc.ResolutionY = GetResolution().Y;
}

float OSVRHMDDescription::GetInterpupillaryDistance() const
{
	FVector EyeDistance = GetLocation(LEFT_EYE) - GetLocation(RIGHT_EYE);
	return FMath::Abs(EyeDistance.X);
}

FVector2D OSVRHMDDescription::GetResolution() const
{
	FVector2D LeftDisplay = GetDisplaySize(LEFT_EYE);
	FVector2D RightDisplay = GetDisplaySize(RIGHT_EYE);

	FVector2D Resolution;
	Resolution.X = LeftDisplay.X + RightDisplay.X;
	Resolution.Y = FMath::Max(LeftDisplay.Y, RightDisplay.Y);

	return Resolution;
}
