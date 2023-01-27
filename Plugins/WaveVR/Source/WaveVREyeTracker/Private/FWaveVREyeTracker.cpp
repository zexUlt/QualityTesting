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

#include "FWaveVREyeTracker.h"
#include "Eye/WaveVREyeBPLibrary.h"
#include "WaveVRBlueprintFunctionLibrary.h"

FWaveVREyeTracker::FWaveVREyeTracker()
{
	LOGI(LogWaveVREyeTracker, "FWaveVREyeTracker() ctor.");
	if (!UWaveVREyeBPLibrary::IsEyeTrackingAvailable())
	{
		LOGI(LogWaveVREyeTracker, "FWaveVREyeTracker() Starts eye tracking.");
		UWaveVREyeBPLibrary::StartEyeTracking(EWVR_CoordinateSystem::World);
	}
}

FWaveVREyeTracker::~FWaveVREyeTracker()
{
	LOGI(LogWaveVREyeTracker, "~FWaveVREyeTracker() dtor.");
	if (UWaveVREyeBPLibrary::IsEyeTrackingAvailable())
	{
		LOGI(LogWaveVREyeTracker, "~FWaveVREyeTracker() Stops eye tracking.");
		UWaveVREyeBPLibrary::StopEyeTracking();
	}
}

bool FWaveVREyeTracker::GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const
{
	bool hasOrigin = UWaveVREyeBPLibrary::GetCombinedEyeOrigin(OutGazeData.GazeOrigin);
	bool hasDirection = UWaveVREyeBPLibrary::GetCombindedEyeDirectionNormalized(OutGazeData.GazeDirection);
	OutGazeData.ConfidenceValue = ((hasOrigin && hasDirection) ? 1 : 0);
	return (hasOrigin && hasDirection);
}

bool FWaveVREyeTracker::GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const
{
	bool hasLeftOrigin = UWaveVREyeBPLibrary::GetLeftEyeOrigin(OutGazeData.LeftEyeOrigin);
	bool hasLeftDirection = UWaveVREyeBPLibrary::GetLeftEyeDirectionNormalized(OutGazeData.LeftEyeDirection);
	bool hasRightOrigin = UWaveVREyeBPLibrary::GetRightEyeOrigin(OutGazeData.RightEyeOrigin);
	bool hasRightDirection = UWaveVREyeBPLibrary::GetRightEyeDirectionNormalized(OutGazeData.RightEyeDirection);
	OutGazeData.ConfidenceValue = ((hasLeftOrigin && hasLeftDirection && hasRightOrigin && hasRightDirection) ? 1 : 0);
	return (hasLeftOrigin && hasLeftDirection && hasRightOrigin && hasRightDirection);
}

EEyeTrackerStatus FWaveVREyeTracker::GetEyeTrackerStatus() const
{
	EWaveVREyeTrackingStatus status = UWaveVREyeBPLibrary::GetEyeTrackingStatus();
	if (status == EWaveVREyeTrackingStatus::AVAILABLE)
		return EEyeTrackerStatus::Tracking;
	if (status == EWaveVREyeTrackingStatus::UNSUPPORT)
		return EEyeTrackerStatus::NotConnected;
	
	return EEyeTrackerStatus::NotTracking;
}

bool FWaveVREyeTracker::IsStereoGazeDataAvailable() const
{
	return UWaveVREyeBPLibrary::IsStereoEyeDataAvailable();
}
