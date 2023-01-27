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

#include "Eye/WaveVREyeEnums.h"
#include "WaveVRBlueprintFunctionLibrary.h"

#include "WaveVREyeBPLibrary.generated.h"

UCLASS()
class WAVEVR_API UWaveVREyeBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Starts the Eye Tracking component in local space or world space."))
	static void StartEyeTracking(EWVR_CoordinateSystem coordinate);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves current Eye Tracking space."))
	static EWVR_CoordinateSystem GetEyeSpace();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Sets up current Eye Tracking space."))
	static void SetEyeSpace(EWVR_CoordinateSystem space);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Stops the Eye Tracking component."))
	static void StopEyeTracking();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Restarts the eye tracking service."))
	static void RestartEyeTracking();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves current eye tracking service status."))
	static EWaveVREyeTrackingStatus GetEyeTrackingStatus();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Checks whether the eye tracking service is available."))
	static bool IsEyeTrackingAvailable();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Checks whether the stereo eye (right and left eye) tracking data is available."))
	static bool IsStereoEyeDataAvailable();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the origin location of combined eye."))
	static bool GetCombinedEyeOrigin(FVector& origin);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the looking direction (x-normalized) of combined eye."))
	static bool GetCombindedEyeDirectionNormalized(FVector& direction);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the origin location of left eye."))
	static bool GetLeftEyeOrigin(FVector& origin);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the looking direction (x-normalized) of left eye."))
	static bool GetLeftEyeDirectionNormalized(FVector& direction);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the value representing how open the left eye is."))
	static bool GetLeftEyeOpenness(float& openness);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the diameter of left eye pupil in millimeters."))
	static bool GetLeftEyePupilDiameter(float& diameter);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the normalized position of left eye pupil in [0,1]."))
	static bool GetLeftEyePupilPositionInSensorArea(FVector2D& position);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the origin location of right eye."))
	static bool GetRightEyeOrigin(FVector& origin);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the looking direction (x-normalized) of right eye."))
	static bool GetRightEyeDirectionNormalized(FVector& direction);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the value representing how open the right eye is."))
	static bool GetRightEyeOpenness(float& openness);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the diameter of right eye pupil in millimeters."))
	static bool GetRightEyePupilDiameter(float& diameter);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye",
		meta = (ToolTip = "Retrieves the normalized position of right eye pupil in [0,1]."))
	static bool GetRightEyePupilPositionInSensorArea(FVector2D& position);
};
