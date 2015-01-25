
#pragma once

#include "IHeadMountedDisplay.h"

class OSVRHMDDescription
{
public:

	OSVRHMDDescription();
	~OSVRHMDDescription();

	bool		Init(OSVR_ClientContext OSVRClientContext);
	bool		IsValid() const { return Valid; }

	enum EEye
	{
		LEFT_EYE = 0,
		RIGHT_EYE
	};
	
	FVector2D 	GetDisplaySize(EEye Eye) const;
	FVector2D 	GetDisplayOrigin(EEye Eye) const;
	FVector2D 	GetFov(EEye Eye) const;
	FVector 	GetLocation(EEye Eye) const;
	FString 	GetPositionalTrackerInterface(EEye Eye) const;
	
	FMatrix 	GetProjectionMatrix(EEye Eye) const;

	void		GetMonitorInfo(IHeadMountedDisplay::MonitorInfo& MonitorDesc) const;
	
	// Helper function
	// IPD    = ABS(GetLocation(LEFT_EYE).X - GetLocation(RIGHT_EYE).X);
	float 		GetInterpupillaryDistance() const;
	
	// Helper function
	// Width  = GetDisplaySize(LEFT_EYE).X + GetDisplaySize(RIGHT_EYE).X;
	// Height = MAX(GetDisplaySize(LEFT_EYE).Y, GetDisplaySize(RIGHT_EYE).Y);
	FVector2D 	GetResolution() const;

private:

	OSVRHMDDescription(OSVRHMDDescription&);
	OSVRHMDDescription& operator=(OSVRHMDDescription&);

	bool Valid;
	void* Data;
};
