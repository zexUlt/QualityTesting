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
#include "Tracker/WaveVRTrackerUtils.h"
#include "Tracker/WaveVRTrackerImpl.h"
#include "WaveVRTrackerBPLibrary.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRTrackerBPLibrary, Log, All);

UCLASS()
class WAVEVR_API UWaveVRTrackerBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "To enable the Tracker component."))
	static void StartTracker();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "To disable the Tracker component."))
	static void StopTracker();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves the Tracker status."))
	static EWaveVRTrackerStatus GetTrackerStatus();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Check if the Tracker is available to use."))
	static bool IsTrackerAvailable();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Check if a Tracker is connected."))
	static bool IsTrackerConnected(EWaveVRTrackerId trackerId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker's role. Refers to EWaveVRTrackerRole."))
	static EWaveVRTrackerRole GetTrackerRole(EWaveVRTrackerId trackerId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Checks if the tracker's pose is valid."))
	static bool IsTrackerPoseValid(EWaveVRTrackerId trackerId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker's position."))
	static bool GetTrackerPosition(EWaveVRTrackerId trackerId, FVector& outPosition);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker's orientation."))
	static bool GetTrackerRotation(EWaveVRTrackerId trackerId, FQuat& outOrientation);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker button's analog type."))
	static EWVR_AnalogType GetTrackerAnalogType(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Check if a Tracker button is pressed."))
	static bool IsTrackerButtonPressed(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Check if a Tracker button is touched."))
	static bool IsTrackerButtonTouched(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker button's axis."))
	static FVector2D GetTrackerButtonAxis(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker's battery life."))
	static float GetTrackerBatteryLife(EWaveVRTrackerId trackerId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Triggers a Tracker's haptic pulse. Default duration is 0.5s, frequency 0 uses system default value, amplitude (0~1) is 0.5f."))
	static bool TriggerTrackerVibration(EWaveVRTrackerId trackerId, int durationMicroSec = 500000, int frequency = 0, float amplitude = 0.5f);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a Tracker's extended data."))
	static TArray<int> GetTrackerExtendedData(EWaveVRTrackerId trackerId, int &validSize);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Registers the Tracker's callback. Retrieves the callback string after registered by GetTrackerCallbackInfo()."))
	static bool RegisterTrackerInfoCallback(EWaveVRTrackerId trackerId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Retrieves a tracker's callback string."))
	static FString GetTrackerCallbackInfo(EWaveVRTrackerId trackerId);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Tracker",
		meta = (ToolTip = "Unregisters a Tracker's callback."))
	static bool UnregisterTrackerInfoCallback();
};
