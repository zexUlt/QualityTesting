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

#include "IEyeTrackerModule.h"
#include "Modules/ModuleManager.h"

class IWaveVREyeTrackerModule : public IEyeTrackerModule
{
public:
	static inline IWaveVREyeTrackerModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IWaveVREyeTrackerModule>("WaveVREyeTracker");
	}
	
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("WaveVREyeTracker");
	}
	
	virtual FString GetModuleKeyName() const override
	{
		return TEXT("WaveVREyeTracker");
	}

	virtual bool IsEyeTrackerConnected() const override;
	virtual TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe > CreateEyeTracker() override;

private:
	TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe > pEyeTracker = nullptr;
};
