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

#include "WaveVREyeExpUtils.generated.h"

UENUM(BlueprintType, Category = "WaveVR|Eye|Expression")
enum class EWaveVREyeExp : uint8
{
	LEFT_BLINK		= 0,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_BLINK
	LEFT_WIDE		= 1,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_WIDE
	RIGHT_BLINK		= 2,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_BLINK
	RIGHT_WIDE		= 3,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_WIDE
	LEFT_SQUEEZE	= 4,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_SQUEEZE
	RIGHT_SQUEEZE	= 5,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_SQUEEZE
	LEFT_DOWN		= 6,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_DOWN
	RIGHT_DOWN		= 7,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_DOWN
	LEFT_OUT		= 8,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_OUT
	RIGHT_IN		= 9,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_IN
	LEFT_IN			= 10,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_IN
	RIGHT_OUT		= 11,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_OUT
	LEFT_UP			= 12,// WVR_EyeExpression::WVR_EYEEXPRESSION_LEFT_UP
	RIGHT_UP		= 13,// WVR_EyeExpression::WVR_EYEEXPRESSION_RIGHT_UP
	MAX,// WVR_EyeExpression::WVR_EYEEXPRESSION_MAX
};

UENUM(BlueprintType, Category = "WaveVR|Eye|Expression")
enum class EWaveVREyeExpStatus : uint8
{
	// Initial, can call Start API in this state.
	NOT_START,
	START_FAILURE,

	// Processing, should NOT call API in this state.
	STARTING,
	STOPING,

	// Running, can call Stop API in this state.
	AVAILABLE,

	// Do nothing.
	NO_SUPPORT
};
