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

#include "WaveVRLipExpUtils.generated.h"

UENUM(BlueprintType, Category = "WaveVR|Lip")
enum class EWaveVRLipExp : uint8
{
	Jaw_Right				= 0,//WVR_LipExpression.WVR_LipExpression_Jaw_Right,
	Jaw_Left				= 1,//WVR_LipExpression.WVR_LipExpression_Jaw_Left,
	Jaw_Forward				= 2,//WVR_LipExpression.WVR_LipExpression_Jaw_Forward,
	Jaw_Open				= 3,//WVR_LipExpression.WVR_LipExpression_Jaw_Open,
	Mouth_Ape_Shape			= 4,//WVR_LipExpression.WVR_LipExpression_Mouth_Ape_Shape,
	Mouth_Upper_Right		= 5,//WVR_LipExpression.WVR_LipExpression_Mouth_Upper_Right,      // 5
	Mouth_Upper_Left		= 6,//WVR_LipExpression.WVR_LipExpression_Mouth_Upper_Left,
	Mouth_Lower_Right		= 7,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Right,
	Mouth_Lower_Left		= 8,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Left,
	Mouth_Upper_Overturn	= 9,//WVR_LipExpression.WVR_LipExpression_Mouth_Upper_Overturn,
	Mouth_Lower_Overturn	= 10,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Overturn,   // 10
	Mouth_Pout				= 11,//WVR_LipExpression.WVR_LipExpression_Mouth_Pout,
	Mouth_Smile_Right		= 12,//WVR_LipExpression.WVR_LipExpression_Mouth_Smile_Right,
	Mouth_Smile_Left		= 13,//WVR_LipExpression.WVR_LipExpression_Mouth_Smile_Left,
	Mouth_Sad_Right			= 14,//WVR_LipExpression.WVR_LipExpression_Mouth_Sad_Right,
	Mouth_Sad_Left			= 15,//WVR_LipExpression.WVR_LipExpression_Mouth_Sad_Left,         // 15
	Cheek_Puff_Right		= 16,//WVR_LipExpression.WVR_LipExpression_Cheek_Puff_Right,
	Cheek_Puff_Left			= 17,//WVR_LipExpression.WVR_LipExpression_Cheek_Puff_Left,
	Cheek_Suck				= 18,//WVR_LipExpression.WVR_LipExpression_Cheek_Suck,
	Mouth_Upper_UpRight		= 19,//WVR_LipExpression.WVR_LipExpression_Mouth_Upper_Upright,
	Mouth_Upper_UpLeft		= 20,//WVR_LipExpression.WVR_LipExpression_Mouth_Upper_Upleft,     // 20
	Mouth_Lower_DownRight	= 21,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Downright,
	Mouth_Lower_DownLeft	= 22,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Downleft,
	Mouth_Upper_Inside		= 23,//WVR_LipExpression.WVR_LipExpression_Mouth_Upper_Inside,
	Mouth_Lower_Inside		= 24,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Inside,
	Mouth_Lower_Overlay		= 25,//WVR_LipExpression.WVR_LipExpression_Mouth_Lower_Overlay,    // 25
	Tongue_Longstep1		= 26,//WVR_LipExpression.WVR_LipExpression_Tongue_Longstep1,
	Tongue_Left				= 27,//WVR_LipExpression.WVR_LipExpression_Tongue_Left,
	Tongue_Right			= 28,//WVR_LipExpression.WVR_LipExpression_Tongue_Right,
	Tongue_Up				= 29,//WVR_LipExpression.WVR_LipExpression_Tongue_Up,
	Tongue_Down				= 30,//WVR_LipExpression.WVR_LipExpression_Tongue_Down,            // 30
	Tongue_Roll				= 31,//WVR_LipExpression.WVR_LipExpression_Tongue_Roll,
	Tongue_Longstep2		= 32,//WVR_LipExpression.WVR_LipExpression_Tongue_Longstep2,
	Tongue_UpRight_Morph	= 33,//WVR_LipExpression.WVR_LipExpression_Tongue_Upright_Morph,
	Tongue_UpLeft_Morph		= 34,//WVR_LipExpression.WVR_LipExpression_Tongue_Upleft_Morph,
	Tongue_DownRight_Morph	= 35,//WVR_LipExpression.WVR_LipExpression_Tongue_Downright_Morph, // 35
	Tongue_DownLeft_Morph	= 36,//WVR_LipExpression.WVR_LipExpression_Tongue_Downleft_Morph,
	Max,//WVR_LipExpression.WVR_LipExpression_Max,
};

UENUM(BlueprintType, Category = "WaveVR|Lip")
enum class EWaveVRLipExpStatus : uint8
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
