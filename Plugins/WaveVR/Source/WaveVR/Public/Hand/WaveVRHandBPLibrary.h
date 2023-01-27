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

#include "WaveVRHandEnums.h"

#include "WaveVRHandBPLibrary.generated.h"

UCLASS()
class WAVEVR_API UWaveVRHandBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To enable the Hand Gesture component."))
	static void StartHandGesture();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To disable the Hand Gesture component."))
	static void StopHandGesture();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To check if the Hand Gesture component is available."))
	static bool IsHandGestureAvailable();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "If the Hand Gesture component is available, this API is used to retrieve current static gesture type."))
	static EWaveVRGestureType GetStaticGestureType(EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To check current Hand Gesture status."))
	static EWaveVRHandGestureStatus GetHandGestureStatus();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To enable the Hand Tracking component."))
	static void StartHandTracking(EWaveVRTrackerType tracker);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To disable the Hand Tracking component."))
	static void StopHandTracking(EWaveVRTrackerType tracker);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To check if the Hand Tracking component is available."))
	static bool IsHandTrackingAvailable(EWaveVRTrackerType tracker);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To check current Hand Tracking status."))
	static EWaveVRHandTrackingStatus GetHandTrackingStatus(EWaveVRTrackerType tracker);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To check if current hand pose is valid."))
	static bool IsHandPoseValid(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "If the Hand Tracking is available, this API is used to retrieve the left or right hand joint poses of the natural or electronic tracker."))
	static bool GetHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FRotator& OutRotation);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "If the Hand Tracking is available, this API is used to retrieve the left or right hand joint poses of the natural or electronic tracker."))
	static bool GetSingleHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FQuat& OutRotation);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "If the Hand Tracking is available, this API is used to retrieve all left or right hand joint poses of the natural or electronic tracker."))
	static bool GetAllHandJointPoses(EWaveVRTrackerType tracker, EWaveVRHandType hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand confidence (0~1), 1 means the most accurate."))
	static float GetHandConfidence(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get current hand motion."))
	static EWaveVRHandMotion GetHandMotion(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand pinch strength recommended threshold (0~1), 1 means the thumb and index finger tip are touching."))
	static float GetHandPinchThreshold(EWaveVRTrackerType tracker);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand pinch strength (0~1), 1 means the thumb and index finger tip are touching."))
	static float GetHandPinchStrength(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand pinch origin location in the world space."))
	static FVector GetHandPinchOrigin(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand pinch direction in the world space."))
	static FVector GetHandPinchDirection(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand's rold when holding."))
	static EWaveVRHandHoldRoleType GetHandHoldRole(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand hold object type when holding."))
	static EWaveVRHandHoldObjectType GetHandHoldType(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To get the hand size scale which is compared to our default model."))
	static FVector GetHandScale(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "Retrieves the wrist linear velocity."))
	static FVector GetWristLinearVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "Retrieves the wrist angular velocity."))
	static FVector GetWristAngularVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To fuse the wrist position with tracker's tracking."))
	static void FuseWristPositionWithTracker(bool fuse);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "Checks if the wrist position is fused with tracker."))
	static bool IsWristPositionFused();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To activate the hold motion."))
	static void ActivateHoldMotion(bool active);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand",
		meta = (ToolTip = "To activate the gun mode."))
	static void ActivateGunMode(bool active);
};
