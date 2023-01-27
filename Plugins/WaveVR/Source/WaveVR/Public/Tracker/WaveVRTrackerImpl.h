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
#include "FWaveVRTrackerThread.h"
#include "WaveVRTrackerUtils.h"

#include "WaveVRBlueprintFunctionLibrary.h"

class WAVEVR_API WaveVRTrackerImpl
{
protected:
	static WaveVRTrackerImpl * Instance;
public:
	static inline WaveVRTrackerImpl * GetInstance() { return Instance; }
public:
	WaveVRTrackerImpl();
	~WaveVRTrackerImpl();

public:
	void InitTrackerData();
	void TickTrackerData();
private:
	FWaveVRTrackerThread * m_TrackerThread;

// Called by WVRHMD
public:
	void OnTrackerConnection(uint8 trackerId, bool connected);

	void OnButtonPress(uint8 trackerId, uint8 buttonId, bool press);
	void OnButtonTouch(uint8 trackerId, uint8 buttonId, bool touch);

	void OnTrackerBatteryLevelUpdate(uint8 trackerId);

private:
	// Life cycle
	bool CanStartTracker();
	bool CanStopTracker();

	// Connection
	TMap<EWaveVRTrackerId, bool> s_Connections;
	void CheckConnections(EWaveVRTrackerId trackerId);
	void CheckStatusWhenConnectionChanges(EWaveVRTrackerId trackerId);

	// Tracker Role
	TMap<EWaveVRTrackerId, EWaveVRTrackerRole> s_Roles;
	void CheckRole(EWaveVRTrackerId trackerId);

	// Capabilities
	void CheckCapabilities(EWaveVRTrackerId trackerId);
	void ResetCapabilities(EWaveVRTrackerId trackerId);

	// Pose State
	TMap<EWaveVRTrackerId, FVector> s_Positions;
	TMap<EWaveVRTrackerId, FQuat> s_Orientations;
	TMap<EWaveVRTrackerId, bool> s_ValidPoses;
	void CheckPoseState(EWaveVRTrackerId trackerId);

	// Input Capability
	TMap<EWaveVRTrackerId, int32_t> s_ButtonBits;
	TMap<EWaveVRTrackerId, int32_t> s_TouchBits;
	TMap<EWaveVRTrackerId, int32_t> s_AnalogBits;
	void CheckInputCapability(EWaveVRTrackerId trackerId);
	bool IsInputAvailable(EWaveVRTrackerId trackerId, EWVR_InputType inputType, EWaveVRTrackerButton button);

	// Input Analog
	TMap<EWaveVRTrackerId, TMap<uint8, EWVR_AnalogType>> s_InputAnalogs;
	void CheckAnalogType(EWaveVRTrackerId trackerId);

	// Button State
	TMap<EWaveVRTrackerId, TArray<bool>> s_ButtonPress;
	TMap<EWaveVRTrackerId, TArray<bool>> s_ButtonTouch;
	TMap<EWaveVRTrackerId, TArray<FVector2D>> s_ButtonAxis;
	void CheckButtonAxis(EWaveVRTrackerId trackerId, EWaveVRTrackerButton button);
	void CheckButtonStates(EWaveVRTrackerId trackerId);

	// Battery
	TMap<EWaveVRTrackerId, float> s_BatteryLife;
	void CheckBatteryLife(EWaveVRTrackerId trackerId);

	// Extended Data
	TMap<EWaveVRTrackerId, int32_t> s_ExtDataSize;
	TMap<EWaveVRTrackerId, int32_t*> s_ExtData;
	void CheckExtendedData(EWaveVRTrackerId trackerId);

// Public Interface
public:
	void StartTracker();
	void StopTracker();
	EWaveVRTrackerStatus GetTrackerStatus();
	bool IsTrackerAvailable();

	bool IsTrackerConnected(EWaveVRTrackerId trackerId) { return s_Connections[trackerId]; }

	EWaveVRTrackerRole GetTrackerRole(EWaveVRTrackerId trackerId) { return s_Roles[trackerId]; }

	bool IsTrackerPoseValid(EWaveVRTrackerId trackerId) { return s_ValidPoses[trackerId]; }
	bool GetTrackerPosition(EWaveVRTrackerId trackerId, FVector& outPosition);
	bool GetTrackerRotation(EWaveVRTrackerId trackerId, FQuat& outOrientation);

	EWVR_AnalogType GetTrackerAnalogType(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);

	bool IsTrackerButtonPressed(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);
	bool IsTrackerButtonTouched(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);
	FVector2D GetTrackerButtonAxis(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId);

	float GetTrackerBatteryLife(EWaveVRTrackerId trackerId);

	bool TriggerTrackerVibration(EWaveVRTrackerId trackerId, uint32_t durationMicroSec = 500000, uint32_t frequency = 0, float amplitude = 0.5f);

	int32_t* GetTrackerExtendedData(EWaveVRTrackerId trackerId, int32_t *validSize);

	bool RegisterTrackerInfoCallback(EWaveVRTrackerId trackerId);
	FString GetTrackerCallbackInfo(EWaveVRTrackerId trackerId);
	bool UnregisterTrackerInfoCallback();

#pragma region
private:
	EWaveVRTrackerId GetTrackerId(uint8 id);
	float GetWorldToMetersScale();

	bool LogInterval();
	uint64_t logFrame = 0;
	int logCount = 0;
#pragma endregion Major Standalone Function
};


