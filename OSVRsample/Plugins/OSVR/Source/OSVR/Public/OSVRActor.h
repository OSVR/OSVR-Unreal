
#pragma once

#include "GameFramework/Actor.h"
#include "OSVRButtonState.h"
#include "OSVRActor.generated.h"

class UOSVRInputComponent;

//USTRUCT()
//struct FButtonData
//{
//	GENERATED_USTRUCT_BODY()
//
//	UPROPERTY()
//	FName Name;
//};

UCLASS()
class OSVR_API AOSVRActor : public AActor
{
	GENERATED_BODY()

public:
	AOSVRActor(const class FObjectInitializer& OI);

	UPROPERTY(VisibleAnywhere, Category = "OSVR")
	class UOSVRInputComponent* osvrInput;

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnPoseChanged(FName Interface, const FTransform & Pose);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnPositionChanged(FName Interface, FVector Position);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnOrientationChanged(FName Interface, FRotator Rotation);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnAnalogValueChanged(FName Interface, float Value);

	UFUNCTION(BlueprintImplementableEvent, Category = "OSVR")
	void OnButtonStateChanged(FName Interface, EButtonState::Type State);
};
