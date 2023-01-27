// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Tracker/WaveVRTrackerImpl.h"

#include "GameFramework/WorldSettings.h"
#include "HeadMountedDisplayTypes.h"
#include "XRTrackingSystemBase.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRTrackerImpl, Log, All);

WaveVRTrackerImpl * WaveVRTrackerImpl::Instance = nullptr;
TMap<EWaveVRTrackerId, WVR_TrackerCapabilities> s_Capabilities;

static FWaveVRAPIWrapper * Interop;

static WVR_InputId GetInputId(EWaveVRTrackerButton button)
{
	WVR_InputId id = WVR_InputId::WVR_InputId_Alias1_System;

	switch (button)
	{
	case EWaveVRTrackerButton::Menu:
		id = WVR_InputId::WVR_InputId_Alias1_Menu;
		break;
	case EWaveVRTrackerButton::A:
		id = WVR_InputId::WVR_InputId_Alias1_A;
		break;
	case EWaveVRTrackerButton::B:
		id = WVR_InputId::WVR_InputId_Alias1_B;
		break;
	case EWaveVRTrackerButton::X:
		id = WVR_InputId::WVR_InputId_Alias1_X;
		break;
	case EWaveVRTrackerButton::Y:
		id = WVR_InputId::WVR_InputId_Alias1_Y;
		break;
	case EWaveVRTrackerButton::Trigger:
		id = WVR_InputId::WVR_InputId_Alias1_Trigger;
		break;
	default:
		break;
	}

	return id;
}

static EWVR_AnalogType GetAnalogType(WVR_AnalogType analog)
{
	if (analog == WVR_AnalogType::WVR_AnalogType_None) { return EWVR_AnalogType::None; }
	if (analog == WVR_AnalogType::WVR_AnalogType_2D) { return EWVR_AnalogType::XY; }
	if (analog == WVR_AnalogType::WVR_AnalogType_1D) { return EWVR_AnalogType::XOnly; }

	return EWVR_AnalogType::None;
}

WaveVRTrackerImpl::WaveVRTrackerImpl()
{
	Instance = this;
	Interop = FWaveVRAPIWrapper::GetInstance();
}

WaveVRTrackerImpl::~WaveVRTrackerImpl()
{
	Instance = nullptr;
}

void WaveVRTrackerImpl::InitTrackerData()
{
	for (int i = 0; i < kTrackerCount; i++)
	{
		// Connection
		s_Connections.Add(k_TrackerIds[i], false);

		// Tracker Role
		s_Roles.Add(k_TrackerIds[i], EWaveVRTrackerRole::Undefined);

		// Capabilities
		s_Capabilities.Add(k_TrackerIds[i], WVR_TrackerCapabilities());
		s_Capabilities[k_TrackerIds[i]].supportsOrientationTracking = false;
		s_Capabilities[k_TrackerIds[i]].supportsPositionTracking = false;
		s_Capabilities[k_TrackerIds[i]].supportsInputDevice = false;
		s_Capabilities[k_TrackerIds[i]].supportsHapticVibration = false;
		s_Capabilities[k_TrackerIds[i]].supportsBatteryLevel = false;

		// Pose State
		s_Positions.Add(k_TrackerIds[i], FVector::ZeroVector);
		s_Orientations.Add(k_TrackerIds[i], FQuat::Identity);
		s_ValidPoses.Add(k_TrackerIds[i], false);

		// Input Capability
		s_ButtonBits.Add(k_TrackerIds[i], 0);
		s_TouchBits.Add(k_TrackerIds[i], 0);
		s_AnalogBits.Add(k_TrackerIds[i], 0);

		// Input Analog
		s_InputAnalogs.Add(k_TrackerIds[i]);
		for (uint8 j = 0; j < (uint8)kAllButtonCount; j++)
		{
			s_InputAnalogs[k_TrackerIds[i]].Add(j, EWVR_AnalogType::None);
		}

		// Button State
		s_ButtonPress.Add(k_TrackerIds[i]);
		s_ButtonPress[k_TrackerIds[i]].Init(false, (uint8)kAllButtonCount);
		s_ButtonTouch.Add(k_TrackerIds[i]);
		s_ButtonTouch[k_TrackerIds[i]].Init(false, (uint8)kAllButtonCount);
		s_ButtonAxis.Add(k_TrackerIds[i]);
		s_ButtonAxis[k_TrackerIds[i]].Init(FVector2D::ZeroVector, (uint8)kAllButtonCount);

		// Battery
		s_BatteryLife.Add(k_TrackerIds[i], 0);

		// Extended Data
		s_ExtDataSize.Add(k_TrackerIds[i], 0);
		s_ExtData.Add(k_TrackerIds[i], new int32_t());
	}

	m_TrackerThread = FWaveVRTrackerThread::JoyInit();

	LOGD(LogWaveVRTrackerImpl, "InitTrackerData()");
}
bool WaveVRTrackerImpl::LogInterval()
{
	if (logFrame != GFrameCounter)
	{
		logFrame = GFrameCounter;
		logCount++;
		logCount %= 1000;
	}
	return (logCount == 0);
}
void WaveVRTrackerImpl::TickTrackerData()
{
	LogInterval();
	if (m_TrackerThread == NULL) { return; }

	if (GWorld && GWorld->GetWorld()->WorldType == EWorldType::Type::Editor)
		return;

	for (int i = 0; i < kTrackerCount; i++)
	{
		EWaveVRTrackerId trackerId = k_TrackerIds[i];

		CheckConnections(trackerId);
		CheckPoseState(trackerId);
		CheckExtendedData(trackerId);
		for (int j = 0; j < kTrackerButtonCount; j++)
		{
			CheckButtonAxis(trackerId, k_TrackerButtons[j]);
		}

		if (LogInterval())
		{
			LOGD(LogWaveVRTrackerImpl, "TickTrackerData() tracker: %d, role: %d\nsupportsOrientationTracking: %d, supportsPositionTracking: %d\nsupportsInputDevice: %d, supportsHapticVibration: %d, supportsBatteryLevel: %d\nvalid pose: %d, button: %d, touch: %d, analog: %d, battery: %f"
				, (uint8)trackerId
				, (uint8)s_Roles[trackerId]
				, (uint8)s_Capabilities[trackerId].supportsOrientationTracking
				, (uint8)s_Capabilities[trackerId].supportsPositionTracking
				, (uint8)s_Capabilities[trackerId].supportsInputDevice
				, (uint8)s_Capabilities[trackerId].supportsHapticVibration
				, (uint8)s_Capabilities[trackerId].supportsBatteryLevel
				, (uint8)s_ValidPoses[trackerId]
				, s_ButtonBits[trackerId]
				, s_TouchBits[trackerId]
				, s_AnalogBits[trackerId]
				, s_BatteryLife[trackerId]);
		}
	}
}

#pragma region
bool WaveVRTrackerImpl::CanStartTracker()
{
	if (m_TrackerThread == NULL) { return false; }

	EWaveVRTrackerStatus status = m_TrackerThread->GetTrackerStatus();
	if (status == EWaveVRTrackerStatus::AVAILABLE ||
		status == EWaveVRTrackerStatus::UNSUPPORT ||
		status == EWaveVRTrackerStatus::STARTING ||
		status == EWaveVRTrackerStatus::STOPING)
	{
		return false;
	}

	return true;
}
bool WaveVRTrackerImpl::CanStopTracker()
{
	if (m_TrackerThread == NULL) { return false; }

	EWaveVRTrackerStatus status = m_TrackerThread->GetTrackerStatus();
	if (status == EWaveVRTrackerStatus::AVAILABLE) { return true; }
	return false;
}
#pragma endregion Life cycle

#pragma region
void WaveVRTrackerImpl::CheckConnections(EWaveVRTrackerId trackerId)
{
	bool connected = false;
	EWaveVRTrackerStatus status = m_TrackerThread->GetTrackerStatus();
	if (status == EWaveVRTrackerStatus::AVAILABLE)
	{
		WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);
		connected = Interop->IsTrackerConnected(tracker);
	}

	if (s_Connections[trackerId] != connected)
	{
		s_Connections[trackerId] = connected;
		LOGD(LogWaveVRTrackerImpl, "CheckConnections() tracker %d, connected %d", (uint8)trackerId, (uint8)s_Connections[trackerId]);
		CheckStatusWhenConnectionChanges(trackerId);
	}
}
void WaveVRTrackerImpl::OnTrackerConnection(uint8 trackerId, bool connected)
{
	LOGD(LogWaveVRTrackerImpl, "OnTrackerConnection() tracker %d, connected %d", trackerId, (uint8)connected);
	EWaveVRTrackerId id = GetTrackerId(trackerId);

	if (s_Connections[id] != connected)
	{
		s_Connections[id] = connected;
		CheckStatusWhenConnectionChanges(id);
	}
}
void WaveVRTrackerImpl::CheckStatusWhenConnectionChanges(EWaveVRTrackerId trackerId)
{
	CheckRole(trackerId);
	CheckCapabilities(trackerId);
	CheckInputCapability(trackerId);
	CheckAnalogType(trackerId);
	CheckButtonStates(trackerId);
	CheckBatteryLife(trackerId);
}
#pragma endregion Connection

#pragma region
void WaveVRTrackerImpl::CheckRole(EWaveVRTrackerId trackerId)
{
	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);
	s_Roles[trackerId] = (s_Connections[trackerId] ?
		(EWaveVRTrackerRole)Interop->GetTrackerRole(tracker) : EWaveVRTrackerRole::Undefined);
	LOGD(LogWaveVRTrackerImpl, "CheckRole() tracker %d, role %d", (uint8)trackerId, (uint8)s_Roles[trackerId]);
}
#pragma endregion Tracker Role

#pragma region
void WaveVRTrackerImpl::CheckCapabilities(EWaveVRTrackerId trackerId)
{
	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);

	if (s_Connections[trackerId])
	{
		WVR_Result result = Interop->GetTrackerCapabilities(tracker, &s_Capabilities[trackerId]);
		if (result != WVR_Result::WVR_Success) { ResetCapabilities(trackerId); }

		LOGD(LogWaveVRTrackerImpl, "CheckCapabilities() tracker %d, result %d\nsupportsOrientationTracking: %d\nsupportsPositionTracking: %d\nsupportsInputDevice: %d\nsupportsHapticVibration: %d\nsupportsBatteryLevel: %d"
			, (uint8)trackerId, (uint8)result
			, (uint8)s_Capabilities[trackerId].supportsOrientationTracking
			, (uint8)s_Capabilities[trackerId].supportsPositionTracking
			, (uint8)s_Capabilities[trackerId].supportsInputDevice
			, (uint8)s_Capabilities[trackerId].supportsHapticVibration
			, (uint8)s_Capabilities[trackerId].supportsBatteryLevel);
	}
	else
	{
		ResetCapabilities(trackerId);
	}
}
void WaveVRTrackerImpl::ResetCapabilities(EWaveVRTrackerId trackerId)
{
	s_Capabilities[trackerId].supportsOrientationTracking = false;
	s_Capabilities[trackerId].supportsPositionTracking = false;
	s_Capabilities[trackerId].supportsInputDevice = false;
	s_Capabilities[trackerId].supportsHapticVibration = false;
	s_Capabilities[trackerId].supportsBatteryLevel = false;
}
#pragma endregion Capabilities

#pragma region
void WaveVRTrackerImpl::CheckPoseState(EWaveVRTrackerId trackerId)
{
	s_ValidPoses[trackerId] = false;

	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);

	if (s_Connections[trackerId])
	{
		if (s_Capabilities[trackerId].supportsOrientationTracking)
		{
			WVR_PoseOriginModel origin = WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnHead_3DoF;
			if (s_Capabilities[trackerId].supportsPositionTracking)
			{
				IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
				EHMDTrackingOrigin::Type dofType = XRSystem->GetTrackingOrigin();
				WVR_PoseOriginModel originModel =
					(dofType == EHMDTrackingOrigin::Type::Floor ?
						WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnGround : WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnHead);

				origin = originModel;
			}

			WVR_PoseState_t pose;
			WVR_Result result = Interop->GetTrackerPoseState(tracker, origin, 0, &pose);
			if (result == WVR_Result::WVR_Success)
			{
				s_ValidPoses[trackerId] = pose.isValidPose;
				CoordinateUtil::MatrixToPose(pose.poseMatrix, s_Orientations[trackerId], s_Positions[trackerId], GetWorldToMetersScale());
			}
		}
	}
}
#pragma endregion Pose State

#pragma region
void WaveVRTrackerImpl::CheckInputCapability(EWaveVRTrackerId trackerId)
{
	WVR_TrackerId id = static_cast<WVR_TrackerId>(trackerId);

	s_ButtonBits[trackerId] = s_Connections[trackerId] ?
		(s_Capabilities[trackerId].supportsInputDevice ?
			Interop->GetTrackerInputDeviceCapability(id, WVR_InputType::WVR_InputType_Button) : 0)
		: 0;
	LOGD(LogWaveVRTrackerImpl, "CheckInputCapability() tracker %d, button %d", (uint8)trackerId, s_ButtonBits[trackerId]);

	s_TouchBits[trackerId] = s_Connections[trackerId] ?
		(s_Capabilities[trackerId].supportsInputDevice ?
			Interop->GetTrackerInputDeviceCapability(id, WVR_InputType::WVR_InputType_Touch) : 0)
		: 0;
	LOGD(LogWaveVRTrackerImpl, "CheckInputCapability() tracker %d, touch %d", (uint8)trackerId, s_TouchBits[trackerId]);

	s_AnalogBits[trackerId] = s_Connections[trackerId] ?
		(s_Capabilities[trackerId].supportsInputDevice ?
			Interop->GetTrackerInputDeviceCapability(id, WVR_InputType::WVR_InputType_Analog) : 0)
		: 0;
	LOGD(LogWaveVRTrackerImpl, "CheckInputCapability() tracker %d, analog %d", (uint8)trackerId, s_AnalogBits[trackerId]);
}
bool WaveVRTrackerImpl::IsInputAvailable(EWaveVRTrackerId trackerId, EWVR_InputType inputType, EWaveVRTrackerButton button)
{
	bool ret = false;
	
	int32_t input = 1 << (uint8)button;
	switch (inputType)
	{
	case EWVR_InputType::Button:
		ret = ((s_ButtonBits[trackerId] & input) == input);
		break;
	case EWVR_InputType::Touch:
		ret = ((s_TouchBits[trackerId] & input) == input);
		break;
	case EWVR_InputType::Analog:
		ret = ((s_AnalogBits[trackerId] & input) == input);
		break;
	default:
		break;
	}
	
	return ret;
}
#pragma endregion Input Capability

#pragma region
void WaveVRTrackerImpl::CheckAnalogType(EWaveVRTrackerId trackerId)
{
	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);

	for (int i = 0; i < kTrackerButtonCount; i++)
	{
		WVR_InputId id = GetInputId(k_TrackerButtons[i]);
		s_InputAnalogs[trackerId][(uint8)k_TrackerButtons[i]] = s_Connections[trackerId] ?
			(IsInputAvailable(trackerId, EWVR_InputType::Analog, k_TrackerButtons[i]) ?
				GetAnalogType(Interop->GetTrackerInputDeviceAnalogType(tracker, id)) : EWVR_AnalogType::None)
			: EWVR_AnalogType::None;
	}
}
#pragma endregion Input Analog

#pragma region
void WaveVRTrackerImpl::CheckButtonAxis(EWaveVRTrackerId trackerId, EWaveVRTrackerButton button)
{
	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);
	WVR_InputId id = GetInputId(button);

	if (s_Connections[trackerId])
	{
		if (IsInputAvailable(trackerId, EWVR_InputType::Analog, button))
		{
			WVR_Axis_t axis = Interop->GetTrackerInputAnalogAxis(tracker, id);
			s_ButtonAxis[trackerId][(uint8)button].X = axis.x;
			s_ButtonAxis[trackerId][(uint8)button].Y = axis.y;
		}
		else
		{
			s_ButtonAxis[trackerId][(uint8)button] = FVector2D::ZeroVector;
		}
	}
	else
	{
		s_ButtonAxis[trackerId][(uint8)button] = FVector2D::ZeroVector;
	}
}
void WaveVRTrackerImpl::CheckButtonStates(EWaveVRTrackerId trackerId)
{
	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);

	for (int i = 0; i < kTrackerButtonCount; i++)
	{
		WVR_InputId id = GetInputId(k_TrackerButtons[i]);

		s_ButtonPress[trackerId][(uint8)k_TrackerButtons[i]] = s_Connections[trackerId] ?
			(IsInputAvailable(trackerId, EWVR_InputType::Button, k_TrackerButtons[i]) ?
				Interop->GetTrackerInputButtonState(tracker, id) : false)
			: false;

		s_ButtonTouch[trackerId][(uint8)k_TrackerButtons[i]] = s_Connections[trackerId] ?
			(IsInputAvailable(trackerId, EWVR_InputType::Touch, k_TrackerButtons[i]) ?
				Interop->GetTrackerInputTouchState(tracker, id) : false)
			: false;

		// Axis is checked every tick.
	}
}
void WaveVRTrackerImpl::OnButtonPress(uint8 trackerId, uint8 buttonId, bool press)
{
	EWaveVRTrackerId id = GetTrackerId(trackerId);
	s_ButtonPress[id][buttonId] = press;
	LOGD(LogWaveVRTrackerImpl, "OnButtonPress() tracker %d, button %d, press %d", trackerId, buttonId, (uint8)s_ButtonPress[id][buttonId]);
}
void WaveVRTrackerImpl::OnButtonTouch(uint8 trackerId, uint8 buttonId, bool touch)
{
	EWaveVRTrackerId id = GetTrackerId(trackerId);
	s_ButtonTouch[id][buttonId] = touch;
	LOGD(LogWaveVRTrackerImpl, "OnButtonTouch() tracker %d, button %d, touch %d", trackerId, buttonId, (uint8)s_ButtonTouch[id][buttonId]);
}
#pragma endregion Button State

#pragma region
void WaveVRTrackerImpl::CheckBatteryLife(EWaveVRTrackerId trackerId)
{
	WVR_TrackerId tracker = static_cast<WVR_TrackerId>(trackerId);

	s_BatteryLife[trackerId] = s_Connections[trackerId] ?
		(s_Capabilities[trackerId].supportsBatteryLevel ?
			Interop->GetTrackerBatteryLevel(tracker) : 0)
		: 0;

	LOGD(LogWaveVRTrackerImpl, "CheckBatteryLife() tracker %d, battery %f", (uint8)trackerId, s_BatteryLife[trackerId]);
}
void WaveVRTrackerImpl::OnTrackerBatteryLevelUpdate(uint8 trackerId)
{
	EWaveVRTrackerId id = GetTrackerId(trackerId);
	CheckBatteryLife(id);

	LOGD(LogWaveVRTrackerImpl, "OnTrackerBatteryLevelUpdate() tracker %d, battery %f", trackerId, s_BatteryLife[id]);
}
#pragma endregion Battery

#pragma region
void WaveVRTrackerImpl::CheckExtendedData(EWaveVRTrackerId trackerId)
{
	if (s_Connections[trackerId])
	{
		int32_t size = 0;
		int32_t *p = Interop->GetTrackerExtendedData(static_cast<WVR_TrackerId>(trackerId), &size);

		if (size > 0)
		{
			if (s_ExtDataSize[trackerId] != size) // different size, re-alloc memory.
			{
				s_ExtDataSize[trackerId] = size;

				delete[] s_ExtData[trackerId];
				s_ExtData[trackerId] = new int32_t[s_ExtDataSize[trackerId]];
			}
			for (int i = 0; i < s_ExtDataSize[trackerId]; i++)
			{
				s_ExtData[trackerId][i] = *(p + i);
			}
		}
		// if size <= 0, do nothing to keep old value.
	}
	// if disconnected, do nothing to keep old value.

	/*LOGD(LogWaveVRTrackerImpl, "CheckExtendedData() trackerId %d size %d", (uint8)trackerId, s_ExtDataSize[trackerId]);
	for (int i = 0; i < s_ExtDataSize[trackerId]; i++)
	{
		LOGD(LogWaveVRTrackerImpl, "CheckExtendedData() trackerId %d exData[%d] = %d", (uint8)trackerId, i, s_ExtData[trackerId][i]);
	}*/
}
#pragma endregion Extended Data

#pragma region
void WaveVRTrackerImpl::StartTracker()
{
	if (CanStartTracker())
	{
		LOGD(LogWaveVRTrackerImpl, "StartTracker()");
		m_TrackerThread->StartTracker();
	}
}
void WaveVRTrackerImpl::StopTracker()
{
	if (CanStopTracker())
	{
		LOGD(LogWaveVRTrackerImpl, "StopTracker()");
		m_TrackerThread->StopTracker();
	}
}
EWaveVRTrackerStatus WaveVRTrackerImpl::GetTrackerStatus()
{
	if (m_TrackerThread)
	{
		return m_TrackerThread->GetTrackerStatus();
	}
	return EWaveVRTrackerStatus::UNSUPPORT;
}
bool WaveVRTrackerImpl::IsTrackerAvailable()
{
	if (m_TrackerThread)
	{
		EWaveVRTrackerStatus status = m_TrackerThread->GetTrackerStatus();
		return (status == EWaveVRTrackerStatus::AVAILABLE);
	}

	return false;
}

bool WaveVRTrackerImpl::GetTrackerPosition(EWaveVRTrackerId trackerId, FVector& outPosition)
{
	if (!IsTrackerPoseValid(trackerId)) { return false; }

	outPosition = s_Positions[trackerId];
	return true;
}
bool WaveVRTrackerImpl::GetTrackerRotation(EWaveVRTrackerId trackerId, FQuat& outOrientation)
{
	if (!IsTrackerPoseValid(trackerId)) { return false; }

	outOrientation = s_Orientations[trackerId];
	return true;
}

EWVR_AnalogType WaveVRTrackerImpl::GetTrackerAnalogType(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	return s_InputAnalogs[trackerId][(uint8)buttonId];
}

bool WaveVRTrackerImpl::IsTrackerButtonPressed(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	return s_ButtonPress[trackerId][(uint8)buttonId];
}
bool WaveVRTrackerImpl::IsTrackerButtonTouched(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	return s_ButtonTouch[trackerId][(uint8)buttonId];
}
FVector2D WaveVRTrackerImpl::GetTrackerButtonAxis(EWaveVRTrackerId trackerId, EWaveVRTrackerButton buttonId)
{
	return s_ButtonAxis[trackerId][(uint8)buttonId];
}

float WaveVRTrackerImpl::GetTrackerBatteryLife(EWaveVRTrackerId trackerId)
{
	return s_BatteryLife[trackerId];
}

bool WaveVRTrackerImpl::TriggerTrackerVibration(EWaveVRTrackerId trackerId, uint32_t durationMicroSec, uint32_t frequency, float amplitude)
{
	amplitude = FMath::Clamp<float>(amplitude, 0, 1);
	if (s_Capabilities[trackerId].supportsHapticVibration)
	{
		LOGD(LogWaveVRTrackerImpl, "TriggerTrackerVibration() tracker: %d, durationMicroSec: %d, frequency: %d, amplitude: %f", (uint8)trackerId, durationMicroSec, frequency, amplitude);
		WVR_Result result = Interop->TriggerTrackerVibration(static_cast<WVR_TrackerId>(trackerId), durationMicroSec, frequency, amplitude);
		return (result == WVR_Result::WVR_Success);
	}
	return false;
}

int32_t* WaveVRTrackerImpl::GetTrackerExtendedData(EWaveVRTrackerId trackerId, int32_t *validSize)
{
	*validSize = s_ExtDataSize[trackerId];
	return s_ExtData[trackerId];
}

static TMap< WVR_TrackerId, FString > s_CallbackInfo = {
	{ WVR_TrackerId_0, FString(TEXT("")) },
	{ WVR_TrackerId_1, FString(TEXT("")) },
	{ WVR_TrackerId_2, FString(TEXT("")) },
	{ WVR_TrackerId_3, FString(TEXT("")) },
	{ WVR_TrackerId_4, FString(TEXT("")) },
	{ WVR_TrackerId_5, FString(TEXT("")) },
	{ WVR_TrackerId_6, FString(TEXT("")) },
	{ WVR_TrackerId_7, FString(TEXT("")) },
	{ WVR_TrackerId_8, FString(TEXT("")) },
};
static void TrackerInfoCallback(WVR_TrackerId trackerId, const char* cbInfo, uint64_t* timestamp)
{
	UE_LOG(LogWaveVRTrackerImpl, Log, TEXT("TrackerInfoCallback() trackerId %d, cbInfo %s, timestamp %lu."), (uint8_t)trackerId, ANSI_TO_TCHAR(cbInfo), *timestamp);
	if (s_CallbackInfo.Contains(trackerId)) { s_CallbackInfo[trackerId] = FString::Printf(TEXT("%s-%lu"), ANSI_TO_TCHAR(cbInfo), *timestamp); }
}

static WVR_TrackerInfoNotify m_TrackerInfo;
bool WaveVRTrackerImpl::RegisterTrackerInfoCallback(EWaveVRTrackerId trackerId)
{
	LOGD(LogWaveVRTrackerImpl, "RegisterTrackerInfoCallback() trackerId %d", (uint8)trackerId);
	m_TrackerInfo.callback = TrackerInfoCallback;
	WVR_Result result = FWaveVRAPIWrapper::GetInstance()->RegisterTrackerInfoCallback(&m_TrackerInfo);
	return (result == WVR_Result::WVR_Success);
}
FString WaveVRTrackerImpl::GetTrackerCallbackInfo(EWaveVRTrackerId trackerId)
{
	return s_CallbackInfo[static_cast<WVR_TrackerId>(trackerId)];
}
bool WaveVRTrackerImpl::UnregisterTrackerInfoCallback()
{
	LOGD(LogWaveVRTrackerImpl, "UnregisterTrackerInfoCallback()");
	WVR_Result result = FWaveVRAPIWrapper::GetInstance()->UnregisterTrackerInfoCallback();
	return (result == WVR_Result::WVR_Success);
}
#pragma endregion Public Interface

#pragma region
EWaveVRTrackerId WaveVRTrackerImpl::GetTrackerId(uint8 id)
{
	if (id == 0) { return EWaveVRTrackerId::Tracker0; }
	if (id == 1) { return EWaveVRTrackerId::Tracker1; }
	if (id == 2) { return EWaveVRTrackerId::Tracker2; }
	if (id == 3) { return EWaveVRTrackerId::Tracker3; }
	if (id == 4) { return EWaveVRTrackerId::Tracker4; }
	if (id == 5) { return EWaveVRTrackerId::Tracker5; }
	if (id == 6) { return EWaveVRTrackerId::Tracker6; }
	if (id == 7) { return EWaveVRTrackerId::Tracker7; }
	if (id == 8) { return EWaveVRTrackerId::Tracker8; }

	return EWaveVRTrackerId::Tracker0;
}

float WaveVRTrackerImpl::GetWorldToMetersScale()
{
	if (IsInGameThread() && GWorld != nullptr)
	{
		// For example, One world unit need multiply 100 to become 1 meter.
		float wtm = GWorld->GetWorldSettings()->WorldToMeters;
		//LOGI(LogWaveVRHandPose, "GWorld->GetWorldSettings()->WorldToMeters = %f", wtm);
		return wtm;
	}
	return 100.0f;
}
#pragma endregion Major Standalone Function
