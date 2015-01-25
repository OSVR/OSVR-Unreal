
#include "OSVRPrivatePCH.h"
#include "OSVRHMDDescription.h"

#include "Json.h"

struct DescriptionData
{
	FVector2D 	DisplaySize[2];
	FVector2D 	DisplayOrigin[2];
	FVector2D 	Fov[2];
	FVector 	Location[2];
	FString 	PositionalTrackerInterface[2];

	DescriptionData();

	bool		InitFromJSON(const char* JSON);
};

static DescriptionData& GetData(void* This) { return *reinterpret_cast<DescriptionData*>(This); }

DescriptionData::DescriptionData()
{
	// Set defaults...
	for (int i = 0; i < 2; ++i)
	{
		DisplaySize[i].Set(960, 1080);
		DisplayOrigin[i].Set(i == 0 ? 0 : 960, 0);
		Fov[i].Set(90, 101.25f);
		Location[i].Set(i == 0 ? -0.0325 : 0.0325, 0, 0);
		PositionalTrackerInterface[i] = "me/head";
	}
}

bool DescriptionData::InitFromJSON(const char* JSON)
{
	TSharedPtr<FJsonObject> JsonObject;
	auto JsonReader = TJsonReaderFactory<CHAR>::Create(JSON);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
		JsonObject.IsValid())
	{
		auto Viewports = JsonObject->GetObjectField(TEXT("viewports"));

		TSharedPtr<FJsonObject> Eyes[2];
		Eyes[0] = Viewports->GetObjectField("left-eye");
		Eyes[1] = Viewports->GetObjectField("right-eye");

		if (!Eyes[0].IsValid() || !Eyes[1].IsValid())
			return false;

		//@TODO: add more checks ...

		for (int i = 0; i < 2; ++i)
		{
			auto& Eye = Eyes[i];
			auto DisplayJson = Eye->GetObjectField("display");
			{
				auto DisplaySizeJson = DisplayJson->GetArrayField("size");

				DisplaySize[i].X = DisplaySizeJson[0]->AsNumber();
				DisplaySize[i].Y = DisplaySizeJson[1]->AsNumber();

				auto DisplayOriginJson = DisplayJson->GetArrayField("origin");

				DisplayOrigin[i].X = DisplayOriginJson[0]->AsNumber();
				DisplayOrigin[i].Y = DisplayOriginJson[1]->AsNumber();
			}

			auto FovJson = Eye->GetObjectField("fov");
			{
				Fov[i].X = FovJson->GetNumberField("horizontal");
				Fov[i].Y = FovJson->GetNumberField("vertical");
			}

			auto LocationJson = Eye->GetObjectField("location");
			{
				auto TranslateJson = LocationJson->GetArrayField("translate");

				Location[i].X = TranslateJson[0]->AsNumber();
				Location[i].Y = TranslateJson[1]->AsNumber();
				Location[i].Z = TranslateJson[2]->AsNumber();

				auto ChildJson = LocationJson->GetObjectField("child");
				{
					PositionalTrackerInterface[i] = ChildJson->GetStringField("tracker");
				}
			}
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

#if OSVR_ENABLED
	size_t Length;
	if (osvrClientGetStringParameterLength(OSVRClientContext, Path, &Length) == OSVR_RETURN_FAILURE)
		return false;

	FMemMark MemStackMark(FMemStack::Get());

	char* Buffer = new(FMemStack::Get()) char[Length];

	if (osvrClientGetStringParameter(OSVRClientContext, Path, Buffer, Length) == OSVR_RETURN_FAILURE)
		return false;

	Valid = GetData(Data).InitFromJSON(Buffer);
#endif // OSVR_ENABLED

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

		* FTranslationMatrix(FVector(PassProjectionOffset, 0, 0));
}

void OSVRHMDDescription::GetMonitorInfo(IHeadMountedDisplay::MonitorInfo& MonitorDesc) const
{
	MonitorDesc.MonitorName  = "OSVR-Display";	//@TODO
	MonitorDesc.MonitorId	 = 0;				//@TODO
	MonitorDesc.DesktopX	 = GetDisplayOrigin(OSVRHMDDescription::LEFT_EYE).X;
	MonitorDesc.DesktopY	 = GetDisplayOrigin(OSVRHMDDescription::LEFT_EYE).Y;
	MonitorDesc.ResolutionX	 = GetResolution().X;
	MonitorDesc.ResolutionY	 = GetResolution().Y;
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
