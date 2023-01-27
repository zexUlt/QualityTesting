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
#include "EyeExpression/FWaveVREyeExpThread.h"
#include "EyeExpression/WaveVREyeExpUtils.h"

#include "WaveVRBlueprintFunctionLibrary.h"

class WAVEVR_API WaveVREyeExpImpl
{
protected:
	static WaveVREyeExpImpl* Instance;
public:
	static inline WaveVREyeExpImpl* GetInstance() { return Instance; }
public:
	WaveVREyeExpImpl();
	~WaveVREyeExpImpl();

public:
	void InitEyeExpData();
	void TickEyeExpData();
private:
	FWaveVREyeExpThread* m_EyeExpThread;

private:
	// Life cycle
	bool CanStartEyeExp();
	bool CanStopEyeExp();

	// Eye Expression Data
	TArray<float> s_EyeExpData;
	bool hasEyeExpData = false;
	void UpdateData();

// Public Interface
public:
	void StartEyeExp();
	void StopEyeExp();
	EWaveVREyeExpStatus GetEyeExpStatus();
	bool IsEyeExpAvailable();
	float GetEyeExpValue(EWaveVREyeExp eyeExp)
	{
		if (!hasEyeExpData) { return 0; }
		return s_EyeExpData[(uint8)eyeExp];
	}
	bool GetEyeExpData(TArray<float>& OutValue)
	{
		if (!hasEyeExpData) { return false; }
		OutValue = s_EyeExpData;
		return true;
	}

#pragma region
private:
	const EWaveVREyeExp s_EyeExpressions[(uint8)EWaveVREyeExp::MAX] = {
		EWaveVREyeExp::LEFT_BLINK, // 0
		EWaveVREyeExp::LEFT_WIDE,
		EWaveVREyeExp::RIGHT_BLINK,
		EWaveVREyeExp::RIGHT_WIDE,
		EWaveVREyeExp::LEFT_SQUEEZE,
		EWaveVREyeExp::RIGHT_SQUEEZE, // 5
		EWaveVREyeExp::LEFT_DOWN,
		EWaveVREyeExp::RIGHT_DOWN,
		EWaveVREyeExp::LEFT_OUT,
		EWaveVREyeExp::RIGHT_IN,
		EWaveVREyeExp::LEFT_IN, // 10
		EWaveVREyeExp::RIGHT_OUT,
		EWaveVREyeExp::LEFT_UP,
		EWaveVREyeExp::RIGHT_UP,
	};
	EWaveVREyeExp GetEyeExp(uint8 id) { return s_EyeExpressions[id]; }
	float GetWorldToMetersScale();

	bool LogInterval();
	uint64_t logFrame = 0;
	int logCount = 0;
#pragma endregion Major Standalone Function
};
