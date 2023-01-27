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

#include "EyeTrackerTypes.h"
#include "IEyeTracker.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/World.h"

#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREyeTracker, Log, All);

class FWaveVREyeTracker : public IEyeTracker
{
public:
	FWaveVREyeTracker();

	virtual ~FWaveVREyeTracker();

private:
	// IEyeTracker
	virtual void SetEyeTrackedPlayer(APlayerController*) override {}
	virtual bool GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const override;
	virtual bool GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const override;
	virtual EEyeTrackerStatus GetEyeTrackerStatus() const override;
	virtual bool IsStereoGazeDataAvailable() const override;
	//~ IEyeTracker
};
