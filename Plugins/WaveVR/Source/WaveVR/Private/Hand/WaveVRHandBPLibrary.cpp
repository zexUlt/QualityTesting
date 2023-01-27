// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/WaveVRHandBPLibrary.h"
#include "Hand/WaveVRHandPose.h"

#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandBPLibrary, Log, All);

#pragma region
void UWaveVRHandBPLibrary::StartHandGesture()
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return;
	
	LOGD(LogWaveVRHandBPLibrary, "StartHandGesture()");
	pHandPose->StartHandGesture();
}
void UWaveVRHandBPLibrary::StopHandGesture()
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return;

	LOGD(LogWaveVRHandBPLibrary, "StopHandGesture()");
	pHandPose->StopHandGesture();
}
bool UWaveVRHandBPLibrary::IsHandGestureAvailable()
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return false;

	return pHandPose->IsHandGestureAvailable();
}
EWaveVRGestureType UWaveVRHandBPLibrary::GetStaticGestureType(EWaveVRHandType DevType)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return EWaveVRGestureType::Invalid;

	return pHandPose->GetStaticGestureType(DevType);
}
EWaveVRHandGestureStatus UWaveVRHandBPLibrary::GetHandGestureStatus()
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return EWaveVRHandGestureStatus::UNSUPPORT;

	return pHandPose->GetHandGestureStatus();
}
#pragma endregion Hand Gesture

void UWaveVRHandBPLibrary::StartHandTracking(EWaveVRTrackerType tracker)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return;

	LOGD(LogWaveVRHandBPLibrary, "StartHandTracking() %d", (uint8)tracker);
	pHandPose->StartHandTracking(tracker);
}
void UWaveVRHandBPLibrary::StopHandTracking(EWaveVRTrackerType tracker)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return;

	LOGD(LogWaveVRHandBPLibrary, "StopHandTracking() %d", (uint8)tracker);
	pHandPose->StopHandTracking(tracker);
}
bool UWaveVRHandBPLibrary::IsHandTrackingAvailable(EWaveVRTrackerType tracker)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return false;

	return pHandPose->IsHandTrackingAvailable(tracker);
}
EWaveVRHandTrackingStatus UWaveVRHandBPLibrary::GetHandTrackingStatus(EWaveVRTrackerType tracker)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return EWaveVRHandTrackingStatus::UNSUPPORT;

	return pHandPose->GetHandTrackingStatus(tracker);
}

bool UWaveVRHandBPLibrary::IsHandPoseValid(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return false;

	return pHandPose->IsHandPoseValid(tracker, hand);
}

bool UWaveVRHandBPLibrary::GetHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FRotator& OutRotation)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return false;

	return pHandPose->GetHandJointPose(tracker, hand, joint, OutPosition, OutRotation);
}
bool UWaveVRHandBPLibrary::GetSingleHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FQuat& OutRotation)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return false;

	return pHandPose->GetHandJointPose(tracker, hand, joint, OutPosition, OutRotation);
}
bool UWaveVRHandBPLibrary::GetAllHandJointPoses(EWaveVRTrackerType tracker, EWaveVRHandType hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return false;

	return pHandPose->GetAllHandJointPoses(tracker, hand, OutPositions, OutRotations);
}
float UWaveVRHandBPLibrary::GetHandConfidence(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return 0;

	return pHandPose->GetHandConfidence(tracker, hand);
}
EWaveVRHandMotion UWaveVRHandBPLibrary::GetHandMotion(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return EWaveVRHandMotion::Invalid;

	return pHandPose->GetHandMotion(tracker, hand);
}
float UWaveVRHandBPLibrary::GetHandPinchThreshold(EWaveVRTrackerType tracker)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return 0;

	return pHandPose->GetHandPinchThreshold(tracker);
}
float UWaveVRHandBPLibrary::GetHandPinchStrength(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return 0;

	return pHandPose->GetHandPinchStrength(tracker, hand);
}
FVector UWaveVRHandBPLibrary::GetHandPinchOrigin(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return FVector::ZeroVector;

	return pHandPose->GetHandPinchOrigin(tracker, hand);
}
FVector UWaveVRHandBPLibrary::GetHandPinchDirection(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return FVector::ZeroVector;

	return pHandPose->GetHandPinchDirection(tracker, hand);
}
EWaveVRHandHoldRoleType UWaveVRHandBPLibrary::GetHandHoldRole(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return EWaveVRHandHoldRoleType::None;

	return pHandPose->GetHandHoldRole(tracker, hand);
}
EWaveVRHandHoldObjectType UWaveVRHandBPLibrary::GetHandHoldType(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return EWaveVRHandHoldObjectType::None;

	return pHandPose->GetHandHoldType(tracker, hand);
}
FVector UWaveVRHandBPLibrary::GetHandScale(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return FVector::OneVector;

	FVector ret = pHandPose->GetHandScale(tracker, hand);
	return ret;
}
FVector UWaveVRHandBPLibrary::GetWristLinearVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return FVector::OneVector;

	FVector ret = pHandPose->GetWristLinearVelocity(tracker, hand);
	return ret;
}
FVector UWaveVRHandBPLibrary::GetWristAngularVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr)
		return FVector::OneVector;

	FVector ret = pHandPose->GetWristAngularVelocity(tracker, hand);
	return ret;
}
void UWaveVRHandBPLibrary::FuseWristPositionWithTracker(bool fuse)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr) { return; }

	LOGD(LogWaveVRHandBPLibrary, "FuseWristPositionWithTracker() %d", (uint8)fuse);
	pHandPose->FuseWristPositionWithTracker(fuse);
}

bool UWaveVRHandBPLibrary::IsWristPositionFused()
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr) { return false; }

	return pHandPose->IsWearingTracker();
}

void UWaveVRHandBPLibrary::ActivateHoldMotion(bool active)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr) { return; }

	LOGD(LogWaveVRHandBPLibrary, "ActivateHoldMotion() %d", (uint8)active);
	pHandPose->ActivateHoldMotion(active);
}

void UWaveVRHandBPLibrary::ActivateGunMode(bool active)
{
	WaveVRHandPose* pHandPose = WaveVRHandPose::GetInstance();
	if (pHandPose == nullptr) { return; }

	LOGD(LogWaveVRHandBPLibrary, "ActivateGunMode() %d", (uint8)active);
	pHandPose->ActivateGunMode(active);
}
