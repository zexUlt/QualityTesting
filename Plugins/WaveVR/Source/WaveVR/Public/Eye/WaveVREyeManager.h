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

#include "CoreMinimal.h"

#include "Eye/WaveVREyeEnums.h"
#include "Eye/FWaveVREyeRunnable.h"
#include "WaveVRBlueprintFunctionLibrary.h"

class WAVEVR_API WaveVREyeManager
{
protected:
	static WaveVREyeManager * Instance;
public:
	static inline WaveVREyeManager * GetInstance() { return Instance; }
public:
	WaveVREyeManager();
	~WaveVREyeManager();

public:
	void InitEyeData();
	void TickEyeData();

	void StartEyeTracking(EWVR_CoordinateSystem coordinate);
	EWVR_CoordinateSystem GetEyeSpace() {
		return locationSpace;
	}
	void SetEyeSpace(EWVR_CoordinateSystem space);
	void StopEyeTracking();
	void RestartEyeTracking();
	bool IsEyeTrackingAvailable() {
		return hasEyeData;
	}
	bool IsStereoEyeDataAvailable();
	EWaveVREyeTrackingStatus GetEyeTrackingStatus() {
		return eyeStatus;
	}

	bool GetCombinedEyeOrigin(FVector& origin);
	bool GetCombindedEyeDirectionNormalized(FVector& direction);

	bool GetLeftEyeOrigin(FVector& origin);
	bool GetLeftEyeDirectionNormalized(FVector& direction);
	bool GetLeftEyeOpenness(float& openness);
	bool GetLeftEyePupilDiameter(float& diameter);
	bool GetLeftEyePupilPositionInSensorArea(FVector2D& position);

	bool GetRightEyeOrigin(FVector& origin);
	bool GetRightEyeDirectionNormalized(FVector& direction);
	bool GetRightEyeOpenness(float& openness);
	bool GetRightEyePupilDiameter(float& diameter);
	bool GetRightEyePupilPositionInSensorArea(FVector2D& position);

private:
	FWaveVREyeRunnable * m_Runnable;
	EWaveVREyeTrackingStatus eyeStatus = EWaveVREyeTrackingStatus::UNSUPPORT;

	bool enableEyeTracking = false;
	EWVR_CoordinateSystem locationSpace = EWVR_CoordinateSystem::World;

	bool hasEyeData = false;

	void UpdateEyeData();

	bool NormalizeX = false;

	/* Combined Eye */
	uint64_t combinedMask = 0;
	FVector combinedOrigin = FVector::ZeroVector;
	FVector combinedDirection = FVector::ZeroVector;

	/* Left Eye */
	uint64_t leftMask = 0;
	FVector leftOrigin = FVector::ZeroVector;
	FVector leftDirection = FVector::ZeroVector;
	float leftOpenness = 0;
	float leftPupilDiameter = 0;
	FVector2D leftPupilPosition = FVector2D::ZeroVector;

	/* Right Eye */
	uint64_t rightMask = 0;
	FVector rightOrigin = FVector::ZeroVector;
	FVector rightDirection = FVector::ZeroVector;
	float rightOpenness = 0;
	float rightPupilDiameter = 0;
	FVector2D rightPupilPosition = FVector2D::ZeroVector;

#pragma region
private:
	float GetWorldToMetersScale();

	int logCount = 0;
	bool printable = false;
#pragma endregion Major Standalone Function
};
