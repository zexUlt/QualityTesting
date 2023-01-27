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

#include "WaveVRTrackerUtils.generated.h"

#pragma region
const int kTrackerCount = 9; // depends on EWaveVRTrackerId

UENUM(BlueprintType, Category = "WaveVR|Tracker")
enum class EWaveVRTrackerId : uint8
{
	Tracker0 = 0, //WVR_TrackerId.WVR_TrackerId_0
	Tracker1 = 1, //WVR_TrackerId.WVR_TrackerId_1
	Tracker2 = 2, //WVR_TrackerId.WVR_TrackerId_2,
	Tracker3 = 3, //WVR_TrackerId.WVR_TrackerId_3,
	Tracker4 = 4, //WVR_TrackerId.WVR_TrackerId_4,
	Tracker5 = 5, //WVR_TrackerId.WVR_TrackerId_5,
	Tracker6 = 6, //WVR_TrackerId.WVR_TrackerId_6,
	Tracker7 = 7, //WVR_TrackerId.WVR_TrackerId_7,
	Tracker8 = 8, //WVR_TrackerId.WVR_TrackerId_8,
};
const EWaveVRTrackerId k_TrackerIds[kTrackerCount] =
{
	EWaveVRTrackerId::Tracker0,
	EWaveVRTrackerId::Tracker1,
	EWaveVRTrackerId::Tracker2,
	EWaveVRTrackerId::Tracker3,
	EWaveVRTrackerId::Tracker4,
	EWaveVRTrackerId::Tracker5,
	EWaveVRTrackerId::Tracker6,
	EWaveVRTrackerId::Tracker7,
	EWaveVRTrackerId::Tracker8,
};
#pragma endregion Tracker ID

UENUM(BlueprintType, Category = "WaveVR|Tracker")
enum class EWaveVRTrackerRole : uint8
{
	Undefined = 0, //WVR_TrackerRole.WVR_TrackerRole_Undefined,
	Standalone = 1, //WVR_TrackerRole.WVR_TrackerRole_Standalone,
	Pair1_Right = 2, //WVR_TrackerRole.WVR_TrackerRole_Pair1_Right,
	Pair1_Left = 3, //WVR_TrackerRole.WVR_TrackerRole_Pair1_Left,
	Pair2_Right = 4, //WVR_TrackerRole.WVR_TrackerRole_Pair2_Right,
	Pair2_Left = 5, //WVR_TrackerRole.WVR_TrackerRole_Pair2_Left,
};

#pragma region
const int kTrackerButtonCount = 7;

UENUM(BlueprintType, Category = "WaveVR|Tracker")
enum class EWaveVRTrackerButton : uint8
{
	System		= 0, //WVR_InputId.WVR_InputId_0,
	Menu		= 1,//WVR_InputId.WVR_InputId_Alias1_Menu,
	A			= 10,//WVR_InputId.WVR_InputId_Alias1_A
	B			= 11,//WVR_InputId.WVR_InputId_Alias1_B
	X			= 12,//WVR_InputId.WVR_InputId_Alias1_X
	Y			= 13,//WVR_InputId.WVR_InputId_Alias1_Y
	Trigger		= 17, //WVR_InputId.WVR_InputId_Alias1_Trigger,
};

const EWaveVRTrackerButton k_TrackerButtons[kTrackerButtonCount] =
{
	EWaveVRTrackerButton::System,
	EWaveVRTrackerButton::Menu,
	EWaveVRTrackerButton::A,
	EWaveVRTrackerButton::B,
	EWaveVRTrackerButton::X,
	EWaveVRTrackerButton::Y,
	EWaveVRTrackerButton::Trigger,
};
#pragma endregion Tracker Button

UENUM(BlueprintType, Category = "WaveVR|Tracker")
enum class EWaveVRTrackerStatus : uint8
{
	// Initial, can call Start API in this state.
	NOT_START,
	START_FAILURE,

	// Processing, should NOT call API in this state.
	STARTING,
	STOPING,

	// Running, can call Stop API in this state.
	AVAILABLE,

	// Do nothing.
	UNSUPPORT
};
