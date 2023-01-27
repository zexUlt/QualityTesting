// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Eye/WaveVREyeBPLibrary.h"
#include "Eye/WaveVREyeManager.h"

#include "wvr_types.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREyeBPLibrary, Log, All);

void UWaveVREyeBPLibrary::StartEyeTracking(EWVR_CoordinateSystem coordinate)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return;

	LOGD(LogWaveVREyeBPLibrary, "StartEyeTracking() %d", (uint8)coordinate);
	pEyeManager->StartEyeTracking(coordinate);
}
EWVR_CoordinateSystem UWaveVREyeBPLibrary::GetEyeSpace()
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return EWVR_CoordinateSystem::World;

	return pEyeManager->GetEyeSpace();
}
void UWaveVREyeBPLibrary::SetEyeSpace(EWVR_CoordinateSystem space)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager != nullptr)
		return pEyeManager->SetEyeSpace(space);
}
void UWaveVREyeBPLibrary::StopEyeTracking()
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return;

	LOGD(LogWaveVREyeBPLibrary, "StopEyeTracking()");
	pEyeManager->StopEyeTracking();
}
void UWaveVREyeBPLibrary::RestartEyeTracking()
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return;

	LOGD(LogWaveVREyeBPLibrary, "RestartEyeTracking()");
	pEyeManager->RestartEyeTracking();
}
EWaveVREyeTrackingStatus UWaveVREyeBPLibrary::GetEyeTrackingStatus()
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return EWaveVREyeTrackingStatus::UNSUPPORT;

	return pEyeManager->GetEyeTrackingStatus();
}
bool UWaveVREyeBPLibrary::IsEyeTrackingAvailable()
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->IsEyeTrackingAvailable();
}
bool UWaveVREyeBPLibrary::IsStereoEyeDataAvailable()
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->IsStereoEyeDataAvailable();
}

bool UWaveVREyeBPLibrary::GetCombinedEyeOrigin(FVector& origin)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetCombinedEyeOrigin(origin);
}
bool UWaveVREyeBPLibrary::GetCombindedEyeDirectionNormalized(FVector& direction)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetCombindedEyeDirectionNormalized(direction);
}

bool UWaveVREyeBPLibrary::GetLeftEyeOrigin(FVector& origin)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetLeftEyeOrigin(origin);
}
bool UWaveVREyeBPLibrary::GetLeftEyeDirectionNormalized(FVector& direction)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetLeftEyeDirectionNormalized(direction);
}
bool UWaveVREyeBPLibrary::GetLeftEyeOpenness(float& openness)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetLeftEyeOpenness(openness);
}
bool UWaveVREyeBPLibrary::GetLeftEyePupilDiameter(float& diameter)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetLeftEyePupilDiameter(diameter);
}
bool UWaveVREyeBPLibrary::GetLeftEyePupilPositionInSensorArea(FVector2D& position)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetLeftEyePupilPositionInSensorArea(position);
}

bool UWaveVREyeBPLibrary::GetRightEyeOrigin(FVector& origin)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetRightEyeOrigin(origin);
}
bool UWaveVREyeBPLibrary::GetRightEyeDirectionNormalized(FVector& direction)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetRightEyeDirectionNormalized(direction);
}
bool UWaveVREyeBPLibrary::GetRightEyeOpenness(float& openness)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetRightEyeOpenness(openness);
}
bool UWaveVREyeBPLibrary::GetRightEyePupilDiameter(float& diameter)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetRightEyePupilDiameter(diameter);
}
bool UWaveVREyeBPLibrary::GetRightEyePupilPositionInSensorArea(FVector2D& position)
{
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	if (pEyeManager == nullptr)
		return false;

	return pEyeManager->GetRightEyePupilPositionInSensorArea(position);
}
