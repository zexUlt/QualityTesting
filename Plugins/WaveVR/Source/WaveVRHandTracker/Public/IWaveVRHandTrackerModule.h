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

#include "IHandTracker.h"

#include "GameFramework/WorldSettings.h"
#include "Engine/World.h"
#include "HeadMountedDisplayTypes.h"

#include "Hand/WaveVRHandBPLibrary.h"

class IWaveVRHandTrackerModule : public IHandTracker, public IModuleInterface
{
public:
	IWaveVRHandTrackerModule();

	virtual ~IWaveVRHandTrackerModule();

	virtual void StartupModule() override
	{
		IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);
	}
	static inline IWaveVRHandTrackerModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IWaveVRHandTrackerModule>("WaveVRHandTracker");
	}
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("WaveVRHandTracker");
	}

private:
	// IHandTracker
	virtual FName GetHandTrackerDeviceTypeName() const override;
	virtual bool IsHandTrackingStateValid() const override;
	virtual bool GetKeypointState(EControllerHand Hand, EHandKeypoint Keypoint, FTransform& OutTransform, float& OutRadius) const override;
	virtual bool GetAllKeypointStates(EControllerHand Hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<float>& OutRadii) const override;
	// ~IHandTracker

	static inline EWaveVRHandJoint GetWaveJointType(EHandKeypoint Keypoint)
	{
		switch (Keypoint)
		{
		case EHandKeypoint::Palm:				return EWaveVRHandJoint::Palm;
		case EHandKeypoint::Wrist:				return EWaveVRHandJoint::Wrist;
		case EHandKeypoint::ThumbMetacarpal:	return EWaveVRHandJoint::Thumb_Joint0;
		case EHandKeypoint::ThumbProximal:		return EWaveVRHandJoint::Thumb_Joint1;
		case EHandKeypoint::ThumbDistal:		return EWaveVRHandJoint::Thumb_Joint2;
		case EHandKeypoint::ThumbTip:			return EWaveVRHandJoint::Thumb_Tip;
		case EHandKeypoint::IndexMetacarpal:	return EWaveVRHandJoint::Index_Joint0;
		case EHandKeypoint::IndexProximal:		return EWaveVRHandJoint::Index_Joint1;
		case EHandKeypoint::IndexIntermediate:	return EWaveVRHandJoint::Index_Joint2;
		case EHandKeypoint::IndexDistal:		return EWaveVRHandJoint::Index_Joint3;
		case EHandKeypoint::IndexTip:			return EWaveVRHandJoint::Index_Tip;
		case EHandKeypoint::MiddleMetacarpal:	return EWaveVRHandJoint::Middle_Joint0;
		case EHandKeypoint::MiddleProximal:		return EWaveVRHandJoint::Middle_Joint1;
		case EHandKeypoint::MiddleIntermediate:	return EWaveVRHandJoint::Middle_Joint2;
		case EHandKeypoint::MiddleDistal:		return EWaveVRHandJoint::Middle_Joint3;
		case EHandKeypoint::MiddleTip:			return EWaveVRHandJoint::Middle_Tip;
		case EHandKeypoint::RingMetacarpal:		return EWaveVRHandJoint::Ring_Joint0;
		case EHandKeypoint::RingProximal:		return EWaveVRHandJoint::Ring_Joint1;
		case EHandKeypoint::RingIntermediate:	return EWaveVRHandJoint::Ring_Joint2;
		case EHandKeypoint::RingDistal:			return EWaveVRHandJoint::Ring_Joint3;
		case EHandKeypoint::RingTip:			return EWaveVRHandJoint::Ring_Tip;
		case EHandKeypoint::LittleMetacarpal:	return EWaveVRHandJoint::Pinky_Joint0;
		case EHandKeypoint::LittleProximal:		return EWaveVRHandJoint::Pinky_Joint1;
		case EHandKeypoint::LittleIntermediate:	return EWaveVRHandJoint::Pinky_Joint2;
		case EHandKeypoint::LittleDistal:		return EWaveVRHandJoint::Pinky_Joint3;
		case EHandKeypoint::LittleTip:			return EWaveVRHandJoint::Pinky_Tip;
		default:
			break;
		}
		return EWaveVRHandJoint::Wrist;
	}
};

