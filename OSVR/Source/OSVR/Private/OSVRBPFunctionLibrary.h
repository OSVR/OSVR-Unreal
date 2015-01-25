
#pragma once

#include "OSVRButtonState.h"
#include "OSVRBPFunctionLibrary.generated.h"

UCLASS()
class UOSVRBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static FTransform GetInterfacePose(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static FVector GetInterfacePosition(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static FRotator GetInterfaceRotation(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static EButtonState::Type GetInterfaceButtonState(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input")
	static float GetInterfaceAnalogValue(FName Name);

	UFUNCTION(BlueprintCallable, Category = "OSVR|Input|HeadMountedDisplay")
	static void SetCurrentHmdOrientationAndPositionAsBase();
};