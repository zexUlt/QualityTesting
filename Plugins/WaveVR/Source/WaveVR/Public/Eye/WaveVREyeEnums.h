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

#include "WaveVREyeEnums.generated.h"

UENUM(BlueprintType, Category = "WaveVR|Eye")
enum class EWaveVREyeTrackingStatus : uint8
{
	// Initial, can call Start API in this state.
	NOT_START,
	START_FAILURE,

	// Processing, should NOT call API in this state.
	STARTING,
	STOPPING,

	// Running, can call Stop API in this state.
	AVAILABLE,

	// Do nothing.
	UNSUPPORT
};
