// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "IWaveVRHandTrackerModule.h"
#include "WaveVRBlueprintFunctionLibrary.h"

#include "Platforms/WaveVRLogWrapper.h"
DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandTracker, Log, All);

IWaveVRHandTrackerModule::IWaveVRHandTrackerModule()
{
	LOGI(LogWaveVRHandTracker, "IWaveVRHandTrackerModule() ctor.");
}

IWaveVRHandTrackerModule::~IWaveVRHandTrackerModule()
{
	LOGI(LogWaveVRHandTracker, "~IWaveVRHandTrackerModule() dtor.");
}

FName IWaveVRHandTrackerModule::GetHandTrackerDeviceTypeName() const
{
	static FName DeviceTypeName = FName(TEXT("WaveHandTracker"));
	return DeviceTypeName;
}

bool IWaveVRHandTrackerModule::IsHandTrackingStateValid() const
{
	return UWaveVRHandBPLibrary::IsHandTrackingAvailable(EWaveVRTrackerType::Natural);
}

bool IWaveVRHandTrackerModule::GetKeypointState(EControllerHand Hand, EHandKeypoint Keypoint, FTransform& OutTransform, float& OutRadius) const
{
	bool ret = false;
	FVector OutPosition = FVector::ZeroVector;
	FQuat OutRotation = FQuat::Identity;

	if (Hand == EControllerHand::Left)
	{
		ret = UWaveVRHandBPLibrary::GetSingleHandJointPose(EWaveVRTrackerType::Natural, EWaveVRHandType::Left, GetWaveJointType(Keypoint), OutPosition, OutRotation);
	}
	if (Hand == EControllerHand::Right)
	{
		ret = UWaveVRHandBPLibrary::GetSingleHandJointPose(EWaveVRTrackerType::Natural, EWaveVRHandType::Right, GetWaveJointType(Keypoint), OutPosition, OutRotation);
	}
	if (ret)
	{
		OutTransform.SetLocation(OutPosition);
		OutTransform.SetRotation(OutRotation);
	}
	return ret;
}

bool IWaveVRHandTrackerModule::GetAllKeypointStates(EControllerHand Hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<float>& OutRadii) const
{
	bool ret = false;

	if (Hand == EControllerHand::Left)
	{
		ret = UWaveVRHandBPLibrary::GetAllHandJointPoses(EWaveVRTrackerType::Natural, EWaveVRHandType::Left, OutPositions, OutRotations);
	}
	if (Hand == EControllerHand::Right)
	{
		ret = UWaveVRHandBPLibrary::GetAllHandJointPoses(EWaveVRTrackerType::Natural, EWaveVRHandType::Right, OutPositions, OutRotations);
	}

	return ret;
}

IMPLEMENT_MODULE(IWaveVRHandTrackerModule, WaveVRHandTracker)
