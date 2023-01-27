// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRController.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "Platforms/Editor/WaveVRDirectPreview.h"
#include "PoseManagerImp.h"


DEFINE_LOG_CATEGORY(LogWaveVRController);

#pragma region Timer
bool Device::AllowGetAxisInAFrame(EWVR_TouchId id)
{
	for (unsigned int i = 0; i < TouchButtonCount; i++)
	{
		if (id == TouchButton[i])
		{
			if (axisUpdateFrameCount[i] != GFrameCounter)
			{
				axisUpdateFrameCount[i] = GFrameCounter;
				return true;
			}
		}
	}

	return false;
}
#pragma endregion

#pragma region Class Device
Device::Device(EWVR_DeviceType hand)
{
	Hand = hand;
	for (unsigned int i = 0; i < TouchButtonCount; i++)
	{
		axisUpdateFrameCount[i] = UINT64_MAX;
	}
	for (uint8 i = 0; i < (uint8)kAllButtonCount; i++)
	{
		curAxis[i] = FVector2D::ZeroVector;
	}
}

FVector2D Device::GetAxis(EWVR_TouchId id)
{
	WVR_Axis_t _axis = {0, 0};
	FVector2D _vec = FVector2D::ZeroVector;

	if (AllowGetAxisInAFrame(id))
	{
		if (WaveVRDirectPreview::IsDirectPreview() || !GIsEditor) {
			_axis = FWaveVRAPIWrapper::GetInstance()->GetInputAnalogAxis((WVR_DeviceType)Hand, (WVR_InputId)id);
			//LOGD(LogWaveVRController, "GetAxis(), id %d axis:{%f, %f}", (uint8)id, _axis.x, _axis.y);
		}
		curAxis[(uint8)id].X = _axis.x;
		curAxis[(uint8)id].Y = _axis.y;
	}

	_vec = curAxis[(int)id];

	return _vec;
}
#pragma endregion

#pragma region Class UWaveVRController
UWaveVRController::UWaveVRController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

Device UWaveVRController::HMD(EWVR_DeviceType::DeviceType_HMD);
Device UWaveVRController::RightController(EWVR_DeviceType::DeviceType_Controller_Right);
Device UWaveVRController::LeftController(EWVR_DeviceType::DeviceType_Controller_Left);

FVector2D UWaveVRController::GetRightControllerAxis(EWVR_TouchId button_id)
{
	FVector2D _vec = RightController.GetAxis(button_id);

	return _vec;
}

FVector2D UWaveVRController::GetLeftControllerAxis(EWVR_TouchId button_id)
{
	FVector2D _vec = LeftController.GetAxis(button_id);

	return _vec;
}

FVector2D UWaveVRController::GetControllerAxis(EWVR_DeviceType device, EWVR_TouchId button_id)
{
	FVector2D _vec = FVector2D::ZeroVector;

	switch (device)
	{
	case EWVR_DeviceType::DeviceType_Controller_Right:
		_vec = GetRightControllerAxis(button_id);
		break;
	case EWVR_DeviceType::DeviceType_Controller_Left:
		_vec = GetLeftControllerAxis(button_id);
		break;
	default:
		break;
	}

	return _vec;
}
#pragma endregion
