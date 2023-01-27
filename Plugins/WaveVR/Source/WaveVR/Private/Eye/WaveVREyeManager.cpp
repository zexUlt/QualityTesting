// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Eye/WaveVREyeManager.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

#include "wvr_eyetracking.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREyeManager, Log, All);

WaveVREyeManager * WaveVREyeManager::Instance = nullptr;
WVR_EyeTracking_t eyeData;

WaveVREyeManager::WaveVREyeManager()
{
	Instance = this;
}

WaveVREyeManager::~WaveVREyeManager()
{
	Instance = nullptr;
}

void WaveVREyeManager::InitEyeData()
{
	locationSpace = EWVR_CoordinateSystem::World;

	eyeStatus = EWaveVREyeTrackingStatus::UNSUPPORT;
	hasEyeData = false;

	/* Combined Eye */
	combinedMask = 0;
	combinedOrigin = FVector::ZeroVector;
	combinedDirection = FVector::ZeroVector;

	/* Left Eye */
	leftMask = 0;
	leftOrigin = FVector::ZeroVector;
	leftDirection = FVector::ZeroVector;
	leftOpenness = 0;
	leftPupilDiameter = 0;
	leftPupilPosition = FVector2D::ZeroVector;

	/* Right Eye */
	rightMask = 0;
	rightOrigin = FVector::ZeroVector;
	rightDirection = FVector::ZeroVector;
	rightOpenness = 0;
	rightPupilDiameter = 0;
	rightPupilPosition = FVector2D::ZeroVector;

	eyeData.timestamp = 0;

	m_Runnable = FWaveVREyeRunnable::JoyInit();
	LOGD(LogWaveVREyeManager, "InitEyeData()");
}

void WaveVREyeManager::TickEyeData()
{
	logCount++;
	logCount %= 500;
	printable = (logCount == 0);

	if (!m_Runnable)
	{
		eyeStatus = EWaveVREyeTrackingStatus::UNSUPPORT;
		hasEyeData = false;
		return;
	}

	if (GWorld && GWorld->GetWorld()->WorldType == EWorldType::Type::Editor)
		return;

	eyeStatus = m_Runnable->GetEyeTrackingStatus();
	if (eyeStatus == EWaveVREyeTrackingStatus::AVAILABLE)
	{
		if (!enableEyeTracking)
		{
			LOGD(LogWaveVREyeManager, "TickEyeData() Stops eye tracking.");
			m_Runnable->StopEyeTracking();
		}
		else
		{
			hasEyeData = (FWaveVRAPIWrapper::GetInstance()->GetEyeTracking(&eyeData, static_cast<WVR_CoordinateSystem>(locationSpace)) == WVR_Result::WVR_Success);
			if (hasEyeData) UpdateEyeData();

			if (printable)
			{
				LOGD(LogWaveVREyeManager, "TickEyeData() locationSpace %d, hasEyeData %d", (uint8)locationSpace, (uint8)hasEyeData);
				LOGD(LogWaveVREyeManager, "TickEyeData() combinedMask %u, combinedOrigin (%f, %f, %f), combinedDirection (%f, %f, %f)"
					, static_cast<uint32_t>(combinedMask), combinedOrigin.X, combinedOrigin.Y, combinedOrigin.Z, combinedDirection.X, combinedDirection.Y, combinedDirection.Z);
				LOGD(LogWaveVREyeManager, "TickEyeData() leftMask %u, leftOrigin (%f, %f, %f), leftDirection (%f, %f, %f)"
					, static_cast<uint32_t>(leftMask), leftOrigin.X, leftOrigin.Y, leftOrigin.Z, leftDirection.X, leftDirection.Y, leftDirection.Z);
				LOGD(LogWaveVREyeManager, "TickEyeData() rightMask %u, rightOrigin (%f, %f, %f), rightDirection (%f, %f, %f)"
					, static_cast<uint32_t>(rightMask), rightOrigin.X, rightOrigin.Y, rightOrigin.Z, rightDirection.X, rightDirection.Y, rightDirection.Z);
			}
		}
	}
	else if (eyeStatus == EWaveVREyeTrackingStatus::NOT_START || eyeStatus == EWaveVREyeTrackingStatus::START_FAILURE)
	{
		hasEyeData = false;
		if (enableEyeTracking)
		{
			LOGD(LogWaveVREyeManager, "TickEyeData() Starts eye tracking.");
			m_Runnable->StartEyeTracking();
		}
	}
}

void WaveVREyeManager::UpdateEyeData()
{
	if (!hasEyeData)
		return;

	/* Combined Eye */
	combinedMask = eyeData.combined.eyeTrackingValidBitMask;
	if ((combinedMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0)
		combinedOrigin = CoordinateUtil::GetVector3(eyeData.combined.gazeOrigin, GetWorldToMetersScale());
	if ((combinedMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0)
	{
		combinedDirection = CoordinateUtil::GetVector3(eyeData.combined.gazeDirectionNormalized, GetWorldToMetersScale());
		if (NormalizeX) { CoordinateUtil::Vector3NormalizeX(combinedDirection); }
	}

	/* Left Eye */
	leftMask = eyeData.left.eyeTrackingValidBitMask;
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0)
		leftOrigin = CoordinateUtil::GetVector3(eyeData.left.gazeOrigin, GetWorldToMetersScale());
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0)
	{
		leftDirection = CoordinateUtil::GetVector3(eyeData.left.gazeDirectionNormalized, GetWorldToMetersScale());
		if (NormalizeX) { CoordinateUtil::Vector3NormalizeX(leftDirection); }
	}
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_EyeOpennessValid) != 0)
		leftOpenness = eyeData.left.eyeOpenness;
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilDiameterValid) != 0)
		leftPupilDiameter = eyeData.left.pupilDiameter;
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilPositionInSensorAreaValid) != 0)
		leftPupilPosition = CoordinateUtil::GetVector2(eyeData.left.pupilPositionInSensorArea, GetWorldToMetersScale());

	/* Right Eye */
	rightMask = eyeData.right.eyeTrackingValidBitMask;
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0)
		rightOrigin = CoordinateUtil::GetVector3(eyeData.right.gazeOrigin, GetWorldToMetersScale());
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0)
	{
		rightDirection = CoordinateUtil::GetVector3(eyeData.right.gazeDirectionNormalized, GetWorldToMetersScale());
		if (NormalizeX) { CoordinateUtil::Vector3NormalizeX(rightDirection); }
	}
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_EyeOpennessValid) != 0)
		rightOpenness = eyeData.right.eyeOpenness;
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilDiameterValid) != 0)
		rightPupilDiameter = eyeData.right.pupilDiameter;
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilPositionInSensorAreaValid) != 0)
		rightPupilPosition = CoordinateUtil::GetVector2(eyeData.right.pupilPositionInSensorArea, GetWorldToMetersScale());
}

void WaveVREyeManager::SetEyeSpace(EWVR_CoordinateSystem space)
{
	locationSpace = space;
	LOGD(LogWaveVREyeManager, "SetEyeSpace() %d", (uint8)locationSpace);
}
void WaveVREyeManager::StartEyeTracking(EWVR_CoordinateSystem coordinate) {
	LOGD(LogWaveVREyeManager, "StartEyeTracking() %d", (uint8)coordinate);
	enableEyeTracking = true;
	locationSpace = coordinate;
}
void WaveVREyeManager::StopEyeTracking() {
	LOGD(LogWaveVREyeManager, "StopEyeTracking()");
	enableEyeTracking = false;
}
void WaveVREyeManager::RestartEyeTracking() {
	if (m_Runnable) {
		LOGD(LogWaveVREyeManager, "RestartEyeTracking()");
		m_Runnable->RestartEyeTracking();
	}
}

bool WaveVREyeManager::IsStereoEyeDataAvailable() {
	bool available =
		hasEyeData &&
		((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0) &&
		((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0) &&
		((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0) &&
		((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0);

	return available;
}
bool WaveVREyeManager::GetCombinedEyeOrigin(FVector& origin) {
	if ((combinedMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0)
	{
		origin = combinedOrigin;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetCombindedEyeDirectionNormalized(FVector& direction) {
	if ((combinedMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0)
	{
		direction = combinedDirection;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetLeftEyeOrigin(FVector& origin) {
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0)
	{
		origin = leftOrigin;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetLeftEyeDirectionNormalized(FVector& direction) {
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0)
	{
		direction = leftDirection;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetLeftEyeOpenness(float& openness) {
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_EyeOpennessValid) != 0)
	{
		openness = leftOpenness;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetLeftEyePupilDiameter(float& diameter) {
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilDiameterValid) != 0)
	{
		diameter = leftPupilDiameter;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetLeftEyePupilPositionInSensorArea(FVector2D& position) {
	if ((leftMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilPositionInSensorAreaValid) != 0)
	{
		position = leftPupilPosition;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetRightEyeOrigin(FVector& origin) {
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeOriginValid) != 0)
	{
		origin = rightOrigin;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetRightEyeDirectionNormalized(FVector& direction) {
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid) != 0)
	{
		direction = rightDirection;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetRightEyeOpenness(float& openness) {
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_EyeOpennessValid) != 0)
	{
		openness = rightOpenness;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetRightEyePupilDiameter(float& diameter) {
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilDiameterValid) != 0)
	{
		diameter = rightPupilDiameter;
		return hasEyeData;
	}
	return false;
}
bool WaveVREyeManager::GetRightEyePupilPositionInSensorArea(FVector2D& position) {
	if ((rightMask & (uint64_t)WVR_EyeTrackingStatus::WVR_PupilPositionInSensorAreaValid) != 0)
	{
		position = rightPupilPosition;
		return hasEyeData;
	}
	return false;
}

#pragma region
float WaveVREyeManager::GetWorldToMetersScale()
{
	if (IsInGameThread() && GWorld != nullptr)
	{
		// For example, One world unit need multiply 100 to become 1 meter.
		float wtm = GWorld->GetWorldSettings()->WorldToMeters;
		//LOGI(LogWaveVREyeManager, "GWorld->GetWorldSettings()->WorldToMeters = %f", wtm);
		return wtm;
	}
	return 100.0f;
}
#pragma endregion Major Standalone Function
