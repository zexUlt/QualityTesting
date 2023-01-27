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
#include "LipExpression/FWaveVRLipExpThread.h"
#include "LipExpression/WaveVRLipExpUtils.h"

#include "WaveVRBlueprintFunctionLibrary.h"

class WAVEVR_API WaveVRLipExpImpl
{
protected:
	static WaveVRLipExpImpl * Instance;
public:
	static inline WaveVRLipExpImpl * GetInstance() { return Instance; }
public:
	WaveVRLipExpImpl();
	~WaveVRLipExpImpl();

public:
	void InitLipExpData();
	void TickLipExpData();
private:
	FWaveVRLipExpThread * m_LipExpThread;

private:
	// Life cycle
	bool CanStartLipExp();
	bool CanStopLipExp();

	// Lip Expression Data
	float s_LipExpValues[(uint8)EWaveVRLipExp::Max] = { 0 };
	TArray<float> s_LipExpData;
	bool hasLipExpData = false;
	void UpdateData();

// Public Interface
public:
	void StartLipExp();
	void StopLipExp();
	EWaveVRLipExpStatus GetLipExpStatus();
	bool IsLipExpAvailable();
	float GetLipExpValue(EWaveVRLipExp lipExp)
	{
		if (!hasLipExpData) { return 0; }
		return s_LipExpValues[(uint8)lipExp];
	}
	bool GetLipExpData(TArray<float>& OutValue)
	{
		if (!hasLipExpData) { return false; }
		OutValue = s_LipExpData;
		return true;
	}

#pragma region
private:
	const EWaveVRLipExp s_LipExpressions[(uint8)EWaveVRLipExp::Max] = {
		EWaveVRLipExp::Jaw_Right,
		EWaveVRLipExp::Jaw_Left,
		EWaveVRLipExp::Jaw_Forward,
		EWaveVRLipExp::Jaw_Open,
		EWaveVRLipExp::Mouth_Ape_Shape,
		EWaveVRLipExp::Mouth_Upper_Right,		// 5
		EWaveVRLipExp::Mouth_Upper_Left,
		EWaveVRLipExp::Mouth_Lower_Right,
		EWaveVRLipExp::Mouth_Lower_Left,
		EWaveVRLipExp::Mouth_Upper_Overturn,
		EWaveVRLipExp::Mouth_Lower_Overturn,	// 10
		EWaveVRLipExp::Mouth_Pout,
		EWaveVRLipExp::Mouth_Smile_Right,
		EWaveVRLipExp::Mouth_Smile_Left,
		EWaveVRLipExp::Mouth_Sad_Right,
		EWaveVRLipExp::Mouth_Sad_Left,			// 15
		EWaveVRLipExp::Cheek_Puff_Right,
		EWaveVRLipExp::Cheek_Puff_Left,
		EWaveVRLipExp::Cheek_Suck,
		EWaveVRLipExp::Mouth_Upper_UpRight,
		EWaveVRLipExp::Mouth_Upper_UpLeft,		// 20
		EWaveVRLipExp::Mouth_Lower_DownRight,
		EWaveVRLipExp::Mouth_Lower_DownLeft,
		EWaveVRLipExp::Mouth_Upper_Inside,
		EWaveVRLipExp::Mouth_Lower_Inside,
		EWaveVRLipExp::Mouth_Lower_Overlay,		// 25
		EWaveVRLipExp::Tongue_Longstep1,
		EWaveVRLipExp::Tongue_Left,
		EWaveVRLipExp::Tongue_Right,
		EWaveVRLipExp::Tongue_Up,
		EWaveVRLipExp::Tongue_Down,				// 30
		EWaveVRLipExp::Tongue_Roll,
		EWaveVRLipExp::Tongue_Longstep2,
		EWaveVRLipExp::Tongue_UpRight_Morph,
		EWaveVRLipExp::Tongue_UpLeft_Morph,
		EWaveVRLipExp::Tongue_DownRight_Morph,	// 35
		EWaveVRLipExp::Tongue_DownLeft_Morph,
	};
	EWaveVRLipExp GetLipExpId(uint8 id) { return s_LipExpressions[id]; }
	float GetWorldToMetersScale();

	bool LogInterval();
	uint64_t logFrame = 0;
	int logCount = 0;
#pragma endregion Major Standalone Function
};


