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

#include "IWaveVREyeTrackerModule.h"
#include "FWaveVREyeTracker.h"

bool IWaveVREyeTrackerModule::IsEyeTrackerConnected() const
{
	if (pEyeTracker)
		return (pEyeTracker->GetEyeTrackerStatus() == EEyeTrackerStatus::Tracking);

	return false;
}

TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe > IWaveVREyeTrackerModule::CreateEyeTracker()
{
	LOGI(LogWaveVREyeTracker, "CreateEyeTracker()");
	pEyeTracker = TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe >(new FWaveVREyeTracker);
	return pEyeTracker;
}

IMPLEMENT_MODULE(IWaveVREyeTrackerModule, WaveVREyeTracker)

