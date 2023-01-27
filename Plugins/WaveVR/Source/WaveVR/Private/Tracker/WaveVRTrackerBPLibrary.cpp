// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Tracker/WaveVRTrackerBPLibrary.h"

#include "Platforms/WaveVRLogWrapper.h"

void UWaveVRTrackerBPLibrary::StartTracker()
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return; }

	LOGD(LogWaveVRTrackerBPLibrary, "StartTracker()");
	pTracker->StartTracker();
}

void UWaveVRTrackerBPLibrary::StopTracker()
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return; }

	LOGD(LogWaveVRTrackerBPLibrary, "StopTracker()");
	pTracker->StopTracker();
}

EWaveVRTrackerStatus UWaveVRTrackerBPLibrary::GetTrackerStatus()
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return EWaveVRTrackerStatus::UNSUPPORT; }

	return pTracker->GetTrackerStatus();
}

bool UWaveVRTrackerBPLibrary::IsTrackerAvailable()
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->IsTrackerAvailable();
}

bool UWaveVRTrackerBPLibrary::IsTrackerConnected(EWaveVRTrackerId trackerId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->IsTrackerConnected(trackerId);
}

EWaveVRTrackerRole UWaveVRTrackerBPLibrary::GetTrackerRole(EWaveVRTrackerId trackerId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return EWaveVRTrackerRole::Undefined; }

	return pTracker->GetTrackerRole(trackerId);
}

bool UWaveVRTrackerBPLibrary::IsTrackerPoseValid(EWaveVRTrackerId trackerId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->IsTrackerPoseValid(trackerId);
}

bool UWaveVRTrackerBPLibrary::GetTrackerPosition(EWaveVRTrackerId trackerId, FVector& outPosition)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->GetTrackerPosition(trackerId, outPosition);
}

bool UWaveVRTrackerBPLibrary::GetTrackerRotation(EWaveVRTrackerId trackerId, FQuat& outOrientation)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->GetTrackerRotation(trackerId, outOrientation);
}

EWVR_AnalogType UWaveVRTrackerBPLibrary::GetTrackerAnalogType(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return EWVR_AnalogType::None; }

	return pTracker->GetTrackerAnalogType(trackerId, buttonId);
}

bool UWaveVRTrackerBPLibrary::IsTrackerButtonPressed(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->IsTrackerButtonPressed(trackerId, buttonId);
}

bool UWaveVRTrackerBPLibrary::IsTrackerButtonTouched(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	return pTracker->IsTrackerButtonTouched(trackerId, buttonId);
}

FVector2D UWaveVRTrackerBPLibrary::GetTrackerButtonAxis(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return FVector2D::ZeroVector; }

	return pTracker->GetTrackerButtonAxis(trackerId, buttonId);
}

float UWaveVRTrackerBPLibrary::GetTrackerBatteryLife(EWaveVRTrackerId trackerId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return 0; }

	return pTracker->GetTrackerBatteryLife(trackerId);
}

bool UWaveVRTrackerBPLibrary::TriggerTrackerVibration(EWaveVRTrackerId trackerId, int durationMicroSec, int frequency, float amplitude)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker == nullptr) { return false; }

	if (durationMicroSec < 0) { durationMicroSec = 500000; }
	if (frequency < 0) { frequency = 0; }
	return pTracker->TriggerTrackerVibration(trackerId, durationMicroSec, frequency, amplitude);
}

TArray<int> UWaveVRTrackerBPLibrary::GetTrackerExtendedData(EWaveVRTrackerId trackerId, int &validSize)
{
	TArray<int> exData;
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker != nullptr)
	{
		int32_t* trackerExtData = pTracker->GetTrackerExtendedData(trackerId, &validSize);
		if (validSize > 0)
		{
			for (int i = 0; i < validSize; i++)
			{
				//LOGI(LogWaveVRTrackerBPLibrary, "GetTrackerExtendedData() trackerId %d exData[%d] = %d", (uint8)trackerId, i, trackerExtData[i]);
				exData.Add(trackerExtData[i]);
			}
		}
	}

	return exData;
}

bool UWaveVRTrackerBPLibrary::RegisterTrackerInfoCallback(EWaveVRTrackerId trackerId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker != nullptr)
	{
		LOGI(LogWaveVRTrackerBPLibrary, "RegisterTrackerInfoCallback() trackerId %d", (uint8)trackerId);
		return pTracker->RegisterTrackerInfoCallback(trackerId);
	}
	return false;
}

FString UWaveVRTrackerBPLibrary::GetTrackerCallbackInfo(EWaveVRTrackerId trackerId)
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker != nullptr)
	{
		return pTracker->GetTrackerCallbackInfo(trackerId);
	}
	return FString(TEXT(""));
}

bool UWaveVRTrackerBPLibrary::UnregisterTrackerInfoCallback()
{
	WaveVRTrackerImpl* pTracker = WaveVRTrackerImpl::GetInstance();
	if (pTracker != nullptr)
	{
		LOGI(LogWaveVRTrackerBPLibrary, "UnregisterTrackerInfoCallback()");
		return pTracker->UnregisterTrackerInfoCallback();
	}
	return false;
}
