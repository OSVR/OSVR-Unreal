
#include "OSVRPrivatePCH.h"
#include "OSVRActor.h"

#include "IOSVR.h"
#include "OSVREntryPoint.h"
#include "OSVRInterfaceCollection.h"
#include "OSVRInputComponent.h"



AOSVRActor::AOSVRActor(const FObjectInitializer& OI)
	: AActor(OI)
{
	osvrInput = OI.CreateDefaultSubobject<UOSVRInputComponent> (this, TEXT("osvrInput"));
	osvrInput->OnPoseChanged.AddDynamic(this, &AOSVRActor::OnPoseChanged);
	osvrInput->OnPositionChanged.AddDynamic(this, &AOSVRActor::OnPositionChanged);
	osvrInput->OnOrientationChanged.AddDynamic(this, &AOSVRActor::OnOrientationChanged);
	osvrInput->OnButtonStateChanged.AddDynamic(this, &AOSVRActor::OnButtonStateChanged);
	osvrInput->OnAnalogValueChanged.AddDynamic(this, &AOSVRActor::OnAnalogValueChanged);
}
