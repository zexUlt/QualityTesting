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
#include "FWaveVRHandThread.h"
#include "WaveVRHandUtils.h"

class WAVEVR_API WaveVRHandPose
{
protected:
	static WaveVRHandPose * Instance;
public:
	static inline WaveVRHandPose * GetInstance() { return Instance; }
public:
	WaveVRHandPose();
	~WaveVRHandPose();

#pragma region
public:
	void InitHandData();
	void TickHandData();
	void CheckPoseFusion();
private:
	void UpdateLeftHandGestureData();
	void UpdateRightHandGestureData();
private:
	FWaveVRHandThread * m_HandThread;
	FVector BONE_OFFSET_L;
	FVector BONE_OFFSET_R;

	bool m_WearingTracker = false;
#pragma endregion Hand Data Update
#pragma region
public:
	void StartHandGesture();
	void StopHandGesture();
	bool IsHandGestureAvailable();
	EWaveVRGestureType GetStaticGestureType(EWaveVRHandType DevType);
	EWaveVRHandGestureStatus GetHandGestureStatus();

private:
	bool m_EnableHandGesture;
	bool bHandGestureDataUpdated;
	EWaveVRGestureType prevStaticGestureRight, prevStaticGestureLeft;
	EWaveVRGestureType currStaticGestureRight, currStaticGestureLeft;
#pragma endregion Hand Gesture
#pragma region
public:
	void StartHandTracking(EWaveVRTrackerType tracker);
	void StopHandTracking(EWaveVRTrackerType tracker);
	bool IsHandTrackingAvailable(EWaveVRTrackerType tracker);
	EWaveVRHandTrackingStatus GetHandTrackingStatus(EWaveVRTrackerType tracker);
	float GetHandConfidence(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	bool IsHandPoseValid(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	bool GetHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FRotator& OutRotation);
	bool GetHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FQuat& OutRotation);
	bool GetAllHandJointPoses(EWaveVRTrackerType tracker, EWaveVRHandType hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations);
	EWaveVRHandMotion GetHandMotion(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	float GetHandPinchThreshold(EWaveVRTrackerType tracker);
	float GetHandPinchStrength(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	FVector GetHandPinchOrigin(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	FVector GetHandPinchDirection(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	EWaveVRHandHoldRoleType GetHandHoldRole(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	EWaveVRHandHoldObjectType GetHandHoldType(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	FVector GetHandScale(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	FVector GetWristLinearVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	FVector GetWristAngularVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand);
	void FuseWristPositionWithTracker(bool fuse);
	void ActivateHoldMotion(bool active);
	void ActivateGunMode(bool active);

	bool IsWearingTracker() { return m_WearingTracker; }

private:
	bool m_EnableNaturalTracker;
	bool hasNaturalTrackerInfo;
	bool hasNaturalTrackerData;
	uint32_t m_NaturalTrackerStopTick, m_NaturalTrackerStartTick;
	TArray<FVector> s_NaturalJointPositionLeft, s_NaturalJointPositionRight;
	TArray<FQuat> s_NaturalJointRotationLeft, s_NaturalJointRotationRight;

	bool m_EnableElectronicTracker;
	bool hasElectronicTrackerInfo;
	bool hasElectronicTrackerData;
	FVector NaturalHandScaleL, NaturalWristLinearVelocityL, NaturalWristAngularVelocityL;
	FVector NaturalHandScaleR, NaturalWristLinearVelocityR, NaturalWristAngularVelocityR;
	FVector ElectronicHandScaleL, ElectronicWristLinearVelocityL, ElectronicWristAngularVelocityL;
	FVector ElectronicHandScaleR, ElectronicWristLinearVelocityR, ElectronicWristAngularVelocityR;
	uint32_t m_ElectronicTrackerStopTick, m_ElectronicTrackerStartTick;
	TArray<FVector> s_ElectronicJointPositionLeft, s_ElectronicJointPositionRight;
	TArray<FQuat> s_ElectronicJointRotationLeft, s_ElectronicJointRotationRight;

	const char *kHoldGunOn = "PLAYER02PUM_HOLD_GUN_ON";
	const char *kHoldGunOff = "PLAYER02PUM_HOLD_GUN_OFF";
	const char *kGunModeOn = "PLAYER01PUM_GUNMODE=1";
	const char* kGunModeOff = "PLAYER01PUM_GUNMODE=0";

#pragma endregion Hand Tracking

#pragma region
private:
	float GetWorldToMetersScale();

	int logCount = 0;
	bool printable = false;
#pragma endregion Major Standalone Function
};
