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

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WaveVRBlueprintFunctionLibrary.h"
#include "WaveVRController.h"
#include "WaveVRInputFunctionLibrary.generated.h"

/**
 * The blueprint function library supports left-handed mode for Controller input.
 */
UCLASS()
class UWaveVRInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if the WaveVRInput feature is available."))
	static bool IsWaveVRInputInitialized();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a device's orientation. Supports left-handed mode."))
	static FRotator GetWaveVRInputDeviceOrientation(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a device's position. Supports left-handed mode."))
	static FVector GetWaveVRInputDevicePosition(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve the position and orientation of a device. Supports left-handed mode."))
	static bool GetWaveVRInputDevicePositionAndOrientation(EWVR_DeviceType device, FVector& OutPosition, FRotator& OutOrientation);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if a device is tracked. Supports left-handed mode."))
	static ETrackingStatus GetWaveVRInputDeviceTrackingStatus(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a device's velocity. Supports left-handed mode."))
	static FVector GetWaveVRInputDeviceVelocity(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a device's angular velocity. Supports left-handed mode."))
	static FVector GetWaveVRInputDeviceAngularVelocity(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To simulate the device position when the environment is 3DoF (rotation only)."))
	static void UseSimulationPose(SimulatePosition simulation = SimulatePosition::WhenNoPosition);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (WorldContext = WorldContextObject, ToolTip = "Activates the Input Simulator in PIE mode."))
	static void EnableInputSimulator(UObject * WorldContextObject);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To make the rotation-only controller which uses the simulation pose follows the head's movement."))
	static void FollowHeadPosition(bool follow);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "PIE only. This API is used for pose simulation customization."))
	static void UpdateUnitySimulationSettingsFromJson(
		FVector HEADTOELBOW_OFFSET,
		FVector ELBOWTOWRIST_OFFSET,
		FVector WRISTTOCONTROLLER_OFFSET,
		FVector ELBOW_PITCH_OFFSET,
		float ELBOW_PITCH_ANGLE_MIN,
		float ELBOW_PITCH_ANGLE_MAX
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if a device is available. Supports left-handed mode."))
	static bool IsInputAvailable(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if a device's pose is valid. Supports left-handed mode."))
	static bool IsInputPoseValid(EWVR_DeviceType device = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if a device's button is pressed. Supports left-handed mode."))
	static bool IsInputButtonPressed(EWVR_DeviceType device, EWVR_InputId button_id);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if a device's button is touched. Supports left-handed mode."))
	static bool IsInputButtonTouched(EWVR_DeviceType device, EWVR_TouchId button_id);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a button's axis. Supports left-handed mode."))
	static FVector2D GetInputButtonAxis(EWVR_DeviceType device, EWVR_TouchId button_id);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To vibrate the specified device in a duration of microseconds. Supports left-handed mode."))
	static void TriggerHapticPulse(EWVR_DeviceType device, int32 duration_ms);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a button's mapping information. E.g. Right thumbstick button is mapping to dominant touchpad key event. Supports left-handed mode."))
	static EWVR_InputId GetInputMappingPair(EWVR_DeviceType type, EWVR_InputId button);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To retrieve a device's battery percentage. Supports left-handed mode."))
	static float GetDeviceBatteryPercentage(EWVR_DeviceType type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Input",
		meta = (ToolTip = "To check if a device's button is available. Supports left-handed mode."))
	static bool IsButtonAvailable(EWVR_DeviceType type, EWVR_InputId button);
};
