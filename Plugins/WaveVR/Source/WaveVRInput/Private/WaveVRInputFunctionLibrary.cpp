// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRInputFunctionLibrary.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "WaveVRInput.h"
#include "WaveVREventCommon.h"

FWaveVRInput* GetWaveVRInput()
{
	TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
	for (auto MotionController : MotionControllers)
	{
		if (MotionController != nullptr && MotionController->GetMotionControllerDeviceTypeName() == FWaveVRInput::DeviceTypeName)
		{
			return static_cast<FWaveVRInput*>(MotionController);
		}
	}

	return nullptr;
}

static EControllerHand EWVRDeviceTypeToEControllerHand(EWVR_DeviceType device)
{
	bool left_handed = UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode();
	switch (device)
	{
	case EWVR_DeviceType::DeviceType_HMD:
		return EControllerHand::ExternalCamera;
		break;
	case EWVR_DeviceType::DeviceType_Controller_Right:
		return left_handed ? EControllerHand::Left : EControllerHand::Right;
		break;
	case EWVR_DeviceType::DeviceType_Controller_Left:
		return left_handed ? EControllerHand::Right : EControllerHand::Left;
		break;
	default:
		break;
	}

	return EControllerHand::AnyHand;
}

static EWVR_DeviceType GetLeftHandedDevice(EWVR_DeviceType device)
{
	bool left_handed = UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode();

	EWVR_DeviceType adaptive_device = device;
	if (left_handed)
	{
		switch (device)
		{
		case EWVR_DeviceType::DeviceType_Controller_Right:
			adaptive_device = EWVR_DeviceType::DeviceType_Controller_Left;
			break;
		case EWVR_DeviceType::DeviceType_Controller_Left:
			adaptive_device = EWVR_DeviceType::DeviceType_Controller_Right;
			break;
		default:
			break;
		}
	}
	return adaptive_device;
}

bool UWaveVRInputFunctionLibrary::IsWaveVRInputInitialized()
{
	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		return WaveVRInput->IsInputInitialized();
	}

	return false;
}

FRotator UWaveVRInputFunctionLibrary::GetWaveVRInputDeviceOrientation(EWVR_DeviceType device)
{
	EControllerHand _hand = EWVRDeviceTypeToEControllerHand(device);
	if (_hand == EControllerHand::AnyHand)
		return FRotator::ZeroRotator;

	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		FRotator orientation;
		FVector position;

		bool bIsValid = WaveVRInput->GetControllerOrientationAndPosition(0, _hand, orientation, position, 0);
		return orientation; // no matter pose is invalid or not, return pose.
	}

	return FRotator::ZeroRotator;
}

FVector UWaveVRInputFunctionLibrary::GetWaveVRInputDevicePosition(EWVR_DeviceType device)
{
	EControllerHand _hand = EWVRDeviceTypeToEControllerHand(device);
	if (_hand == EControllerHand::AnyHand)
		return FVector::ZeroVector;

	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		FRotator orientation;
		FVector position;
		bool bIsValid = WaveVRInput->GetControllerOrientationAndPosition(0, _hand, orientation, position, 0);
		return position; // no matter pose is invalid or not, return pose.
	}

	return FVector::ZeroVector;
}

bool UWaveVRInputFunctionLibrary::GetWaveVRInputDevicePositionAndOrientation(EWVR_DeviceType device, FVector& OutPosition, FRotator& OutOrientation)
{
	EControllerHand _hand = EWVRDeviceTypeToEControllerHand(device);
	if (_hand == EControllerHand::AnyHand)
		return false;

	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		FRotator orientation;
		FVector position;
		if (WaveVRInput->GetControllerOrientationAndPosition(0, _hand, orientation, position, 0))
		{
			OutPosition = position;
			OutOrientation = orientation;
			return true;
		}
	}

	return false;
}

ETrackingStatus UWaveVRInputFunctionLibrary::GetWaveVRInputDeviceTrackingStatus(EWVR_DeviceType device)
{
	EControllerHand _hand = EWVRDeviceTypeToEControllerHand(device);
	if (_hand == EControllerHand::AnyHand)
		return ETrackingStatus::NotTracked;

	ETrackingStatus status = ETrackingStatus::NotTracked;
	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		status = WaveVRInput->GetControllerTrackingStatus((int32)_hand, _hand);
	}

	return status;
}

FVector UWaveVRInputFunctionLibrary::GetWaveVRInputDeviceVelocity(EWVR_DeviceType device)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	FVector velocity = UWaveVRBlueprintFunctionLibrary::GetDeviceVelocity(adaptive_device);

	return velocity;
}

FVector UWaveVRInputFunctionLibrary::GetWaveVRInputDeviceAngularVelocity(EWVR_DeviceType device)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	FVector angularv = UWaveVRBlueprintFunctionLibrary::GetDeviceAngularVelocity(adaptive_device);

	return angularv;
}

void UWaveVRInputFunctionLibrary::UseSimulationPose(SimulatePosition simulation)
{
	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		WaveVRInput->UseSimulationPose(simulation);
	}
}

void UWaveVRInputFunctionLibrary::EnableInputSimulator(UObject * WorldContextObject)
{
	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		WaveVRInput->EnableInputSimulator(WorldContextObject);
	}
}

void UWaveVRInputFunctionLibrary::FollowHeadPosition(bool follow)
{
	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		WaveVRInput->FollowHeadPosition(follow);
	}
}

void UWaveVRInputFunctionLibrary::UpdateUnitySimulationSettingsFromJson(
	FVector HEADTOELBOW_OFFSET,
	FVector ELBOWTOWRIST_OFFSET,
	FVector WRISTTOCONTROLLER_OFFSET,
	FVector ELBOW_PITCH_OFFSET,
	float ELBOW_PITCH_ANGLE_MIN,
	float ELBOW_PITCH_ANGLE_MAX)
{
	FWaveVRInput* WaveVRInput = GetWaveVRInput();
	if (WaveVRInput != nullptr)
	{
		WaveVRInput->UpdateUnitySimulationSettingsFromJson(
			HEADTOELBOW_OFFSET,
			ELBOWTOWRIST_OFFSET,
			WRISTTOCONTROLLER_OFFSET,
			ELBOW_PITCH_OFFSET,
			ELBOW_PITCH_ANGLE_MIN,
			ELBOW_PITCH_ANGLE_MAX);
	}
}


bool UWaveVRInputFunctionLibrary::IsInputAvailable(EWVR_DeviceType device)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	return UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(adaptive_device);
}

bool UWaveVRInputFunctionLibrary::IsInputPoseValid(EWVR_DeviceType device)
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World()->IsPlayInEditor())
		{
			return true;
		}
	}
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	FVector _position = FVector::ZeroVector;
	FRotator _rotation = FRotator::ZeroRotator;

	return UWaveVRBlueprintFunctionLibrary::GetDevicePose(_position, _rotation, adaptive_device);
}

#pragma region
bool UWaveVRInputFunctionLibrary::IsInputButtonPressed(EWVR_DeviceType device, EWVR_InputId button_id)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	return UWaveVRBlueprintFunctionLibrary::IsButtonPressed(adaptive_device, button_id);
}

bool UWaveVRInputFunctionLibrary::IsInputButtonTouched(EWVR_DeviceType device, EWVR_TouchId button_id)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	return UWaveVRBlueprintFunctionLibrary::IsButtonTouched(adaptive_device, button_id);
}
#pragma endregion

EWVR_InputId UWaveVRInputFunctionLibrary::GetInputMappingPair(EWVR_DeviceType type, EWVR_InputId button)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(type);

	return UWaveVRBlueprintFunctionLibrary::GetInputMappingPair(adaptive_device, button);
}

bool UWaveVRInputFunctionLibrary::IsButtonAvailable(EWVR_DeviceType type, EWVR_InputId button)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(type);

	return UWaveVRBlueprintFunctionLibrary::IsButtonAvailable(adaptive_device, button);
}

FVector2D UWaveVRInputFunctionLibrary::GetInputButtonAxis(EWVR_DeviceType device, EWVR_TouchId button_id)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	return UWaveVRController::GetControllerAxis(adaptive_device, button_id);
}

float UWaveVRInputFunctionLibrary::GetDeviceBatteryPercentage(EWVR_DeviceType device)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	return UWaveVRBlueprintFunctionLibrary::getDeviceBatteryPercentage(adaptive_device);
}

void UWaveVRInputFunctionLibrary::TriggerHapticPulse(EWVR_DeviceType device, int32 duration_ms)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(device);
	duration_ms = duration_ms > 0 ? duration_ms : 1000;
	int32 duration_millisec = duration_ms / 1000;

	UWaveVRBlueprintFunctionLibrary::TriggerVibration(device, duration_millisec, EWVR_Intensity::Normal);
}
