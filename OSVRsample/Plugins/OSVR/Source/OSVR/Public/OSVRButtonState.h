
#pragma once

#include "OSVRButtonState.generated.h"

UENUM(BlueprintType)
namespace EButtonState
{
	enum Type
	{
		PRESSED 		UMETA(DisplayName = "Button is pressed"),
		NOT_PRESSED 	UMETA(DisplayName = "Button is not pressed"),
		NUM				UMETA(Hidden),
	};
}
