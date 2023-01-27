// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#pragma once

#include <chrono>
using namespace std::chrono;

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WaveVRBlueprintTypes.h"
#include "WaveVRController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWaveVRController, Log, All);

class Device
{
public:
	Device(EWVR_DeviceType hand);

	EWVR_DeviceType Hand;

	bool AllowGetAxisInAFrame(EWVR_TouchId id);
	FVector2D GetAxis(EWVR_TouchId id);

private:
	uint64 axisUpdateFrameCount[TouchButtonCount];
	FVector2D curAxis[(uint8)kAllButtonCount];
};

UCLASS(Blueprintable)
class WAVEVR_API UWaveVRController : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UWaveVRController();

	static FVector2D GetRightControllerAxis(EWVR_TouchId button_id);
	static FVector2D GetLeftControllerAxis(EWVR_TouchId button_id);
	static FVector2D GetControllerAxis(EWVR_DeviceType device, EWVR_TouchId button_id);

private:
	static Device HMD;
	static Device RightController;
	static Device LeftController;
};
