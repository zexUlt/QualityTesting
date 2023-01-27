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
#include "WaveVRBlueprintTypes.generated.h"

#pragma region
static const unsigned int EWVR_DeviceType_Count = 4;
/** Defines the type of tracked devices*/
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_DeviceType : uint8
{
	DeviceType_Invalid = 0,  //WVR_DeviceType::WVR_DeviceType_Invalid,
	DeviceType_HMD = 1,  //WVR_DeviceType::WVR_DeviceType_HMD,
	DeviceType_Controller_Right = 2,  //WVR_DeviceType::WVR_DeviceType_Controller_Right,
	DeviceType_Controller_Left = 3,  //WVR_DeviceType::WVR_DeviceType_Controller_Left,
	DeviceType_Camera = 4,  //WVR_DeviceType::WVR_DeviceType_Camera,
	DeviceType_EyeTracking = 5,  //WVR_DeviceType::WVR_DeviceType_EyeTracking,
	DeviceType_HandGesture_Right = 6,  //WVR_DeviceType::WVR_DeviceType_HandGesture_Right,
	DeviceType_HandGesture_Left = 7,  //WVR_DeviceType::WVR_DeviceType_HandGesture_Left,
	DeviceType_NaturalHand_Right = 8,  //WVR_DeviceType::WVR_DeviceType_NaturalHand_Right,
	DeviceType_NaturalHand_Left = 9,  //WVR_DeviceType::WVR_DeviceType_NaturalHand_Left,
	DeviceType_ElectronicHand_Right = 10, //WVR_DeviceType::WVR_DeviceType_ElectronicHand_Right,
	DeviceType_ElectronicHand_Left = 11, //WVR_DeviceType::WVR_DeviceType_ElectronicHand_Left,
};

static const int kAllButtonCount = 32; // Button ID is 0~31, total 32.
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_InputId : uint8
{
	NoUse = 0,  //WVR_InputId::WVR_InputId_Alias1_System,
	Menu = 1,  //WVR_InputId::WVR_InputId_Alias1_Menu,
	Grip = 2,  //WVR_InputId::WVR_InputId_Alias1_Grip,
	DPad_Left = 3,  //WVR_InputId::WVR_InputId_Alias1_DPad_Left,
	DPad_Up = 4,  //WVR_InputId::WVR_InputId_Alias1_DPad_Up,
	DPad_Right = 5,  //WVR_InputId::WVR_InputId_Alias1_DPad_Right,
	DPad_Down = 6,  //WVR_InputId::WVR_InputId_Alias1_DPad_Down,
	Volume_Up = 7,  //WVR_InputId::WVR_InputId_Alias1_Volume_Up,
	Volume_Down = 8,  //WVR_InputId::WVR_InputId_Alias1_Volume_Down,
	Bumper = 9,  //WVR_InputId::WVR_InputId_Alias1_Bumper,
	A = 10, //WVR_InputId::WVR_InputId_Alias1_A,
	B = 11, //WVR_InputId::WVR_InputId_Alias1_B,
	X = 12, //WVR_InputId::WVR_InputId_Alias1_X,
	Y = 13, //WVR_InputId::WVR_InputId_Alias1_Y,
	Back = 14, //WVR_InputId::WVR_InputId_Alias1_Back,
	Enter = 15, //WVR_InputId::WVR_InputId_Alias1_Enter,
	Touchpad = 16, //WVR_InputId::WVR_InputId_Alias1_Touchpad,
	Trigger = 17, //WVR_InputId::WVR_InputId_Alias1_Trigger,
	Thumbstick = 18, //WVR_InputId::WVR_InputId_Alias1_Thumbstick,

	//Max = 32, //WVR_InputId::WVR_InputId_Max
};

/// Note: if this value were changed, WaveVRInput would also be updated.
/// The input Ids listed here are Unreal needs, so no Digital Trigger
static const int InputButtonCount = 18;
static const EWVR_InputId InputButton[InputButtonCount] =
{
	EWVR_InputId::Menu,
	EWVR_InputId::Grip,
	EWVR_InputId::DPad_Left,
	EWVR_InputId::DPad_Up,
	EWVR_InputId::DPad_Right,	// 5
	EWVR_InputId::DPad_Down,
	EWVR_InputId::Volume_Up,
	EWVR_InputId::Volume_Down,
	EWVR_InputId::Bumper,
	EWVR_InputId::A,			// 10
	EWVR_InputId::B,
	EWVR_InputId::X,
	EWVR_InputId::Y,
	EWVR_InputId::Back,
	EWVR_InputId::Enter,		// 15
	EWVR_InputId::Touchpad,
	EWVR_InputId::Trigger,
	EWVR_InputId::Thumbstick,
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_TouchId : uint8
{
	NoUse = 0,  //WVR_InputId::WVR_InputId_Alias1_System,
	Grip = 2,  //WVR_InputId::WVR_InputId_Alias1_Grip,
	A = 10, //WVR_InputId::WVR_InputId_Alias1_A,
	B = 11, //WVR_InputId::WVR_InputId_Alias1_B,
	X = 12, //WVR_InputId::WVR_InputId_Alias1_X,
	Y = 13, //WVR_InputId::WVR_InputId_Alias1_Y,
	Touchpad = 16, //WVR_InputId::WVR_InputId_Alias1_Touchpad,
	Trigger = 17, //WVR_InputId::WVR_InputId_Alias1_Trigger,
	Thumbstick = 18, //WVR_InputId::WVR_InputId_Alias1_Thumbstick,
	Parking = 19, //WVR_InputId::WVR_InputId_Alias1_Parking
};

/// Note: if this value were changed, WaveVRInput would also be updated.
static const int TouchButtonCount = 9;
static const EWVR_TouchId TouchButton[TouchButtonCount] =
{
	EWVR_TouchId::Touchpad,
	EWVR_TouchId::Trigger,
	EWVR_TouchId::Thumbstick,
	EWVR_TouchId::Parking,
	EWVR_TouchId::Grip,	// 5
	EWVR_TouchId::A,
	EWVR_TouchId::B,
	EWVR_TouchId::X,
	EWVR_TouchId::Y,
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_Hand : uint8
{
	Hand_Controller_Invalid = 0,
	Hand_Controller_Right = 2,  // WVR_DeviceType::WVR_DeviceType_Controller_Right
	Hand_Controller_Left = 3    // WVR_DeviceType::WVR_DeviceType_Controller_Left
};
#pragma endregion Device and Button

#pragma region
/**
 * @brief The beam mode: fixed length of flexible length.
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EBeamMode : uint8
{
	Flexible = 0,
	Fixed = 1,
};
/**
 * @brief The interaction mode. If current system mode is "Controller", the "Default" mode will become "Controller".
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVRInteractionMode : uint8
{
	Invalid = 0,
	Default = 1, //WVR_InteractionMode::WVR_InteractionMode_SystemDefault,
	Gaze = 2, //WVR_InteractionMode::WVR_InteractionMode_Gaze,
	Controller = 3, //WVR_InteractionMode::WVR_InteractionMode_Controller,
	Hand = 4, //WVR_InteractionMode::WVR_InteractionMode_Hand,
};
/**
 * @brief The type of Gaze. You can use a timer, buttons, or both to trigger gaze events.
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVRGazeTriggerType : uint8
{
	Invalid = 0,
	Timeout = 1, //WVR_GazeTriggerType::WVR_GazeTriggerType_Timeout,
	Button = 2, //WVR_GazeTriggerType::WVR_GazeTriggerType_Button,
	TimeoutButton = 3, //WVR_GazeTriggerType::WVR_GazeTriggerType_TimeoutButton,
};
#pragma endregion Interaction Mode Manager

UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_DOF : uint8
{
	DOF_3,
	DOF_6,
	DOF_SYSTEM
};

UENUM(BlueprintType, Category = "WaveVR")
enum class SimulatePosition : uint8
{
	WhenNoPosition = 0,   // simulate when 3DoF.
	ForceSimulation = 1, // force using simulation pose.
	NoSimulation = 2
};

/**
* @brief Recenter effect of WVR_InAppRecenter
*/
UENUM(BlueprintType, Category = "WaveVR")
enum class ERecenterType : uint8
{
	Disabled = 0,    /**< Make everything back to system coordinate instantly */
	YawOnly = 1,    /**< Only adjust the Yaw angle */
	YawAndPosition = 2,    /**< Adjust the Yaw angle, and also reset user's position to Virtual World's center */
	RotationAndPosition = 3,     /**< Affect all the XYZ and Pitch Yaw Roll. It is sensitive to user's head gesture at recetnering. */
	Position = 4    /**< Reset user's position to Virtual World's center */
};

/**
* @brief UTexture2D format
*/
UENUM(BlueprintType, Category = "WaveVR")
enum class EUTex2DFmtType : uint8
{
	BMP = 0,    /** Note: BMP Image format */
	PNG = 1,    /** Note: PNG Image format */
	JPEG = 2     /** Note: JPEG Image format */
};

/**
 * @brief Foveated rendering mode.
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_FoveationMode : uint8
{
	Disable = 0,    /**< **WVR_FoveationMode_Disable**: Disable foveated rendering. */
	Enable = 1,    /**< **WVR_FoveationMode_Enable**: Enable foveated rendering and using @ref WVR_SetFoveationConfig to set config. */
	Default = 2,    /**< **WVR_FoveationMode_Default**: Default config depending on the device. Using @ref WVR_IsRenderFoveationDefaultOn and @ref WVR_GetFoveationDefaultConfig to query details. */
};

/**
 * @brief Peripheral Quality which using in @ref WVR_RenderFoveationParams_t.
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_PeripheralQuality : uint8
{
	Low = 0,    /**< **WVR_PeripheralQuality_High**: Peripheral quity is high and power saving is low. */
	Medium = 1,    /**< **WVR_PeripheralQuality_Medium**: Peripheral quity is medium and power saving is medium. */
	High = 2     /**< **WVR_PeripheralQuality_Low**: Peripheral quity is low and power saving is high. */
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EEye : uint8
{
	LEFT = 0,
	RIGHT = 1
};


/**
 * UE4 WaveVR_Screenshot
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EScreenshotMode : uint8
{
	DEFAULT = 0,
	RAW,
	DISTORTED
};

/**
 * UE4 WaveVR_RenderMask
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class ERenderMaskMode : uint8
{
	DISABLE = 0,	/* Render mask disable */
	ENABLE = 1,	/* Render mask enable */
	DEFAULT = 2		/* Render mask follow the global setting to enable or disable */
};

/**
 * UE4 Adaptive Quality Mode
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EAdaptiveQualityMode : uint8
{
	Quality_Oriented = 0,      /* SendQualityEvent On, AutoFoveation On. */
	Performance_Oriented = 1,  /* SendQualityEvent On, AutoFoveation On. */
	Customization = 2           /* Choose SendQualityEvent and AutoFoveation manually. */
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EAMCMode : uint8
{
	Off,  /* Always Off. */
	Force_UMC,  /* Use UMC. */
	Auto,  /* UMC will dynamicly turn on or off due to rendering and performance status. */
	//Force_PMC  /* Use PMC. */
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EAMCStatus : uint8
{
	Off,  /* Off. */
	UMC,  /* Use UMC. */
	//PMC  /* Use PMC. */
};

/**
 * @brief The options of controller pose mode
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_ControllerPoseMode : uint8
{
	ControllerPoseMode_Raw = 0, /**< Raw mode (default). It would be the same as one of three other modes (Trigger/Panel/Handle). */
	ControllerPoseMode_Trigger = 1, /**< Trigger mode: Controller ray is parallel to the trigger button of controller. */
	ControllerPoseMode_Panel = 2, /**< Panel mode: Controller ray is parallel to the panel of controller. */
	ControllerPoseMode_Handle = 3, /**< Handle mode: Controller ray is parallel to the handle of controller. */
};

/**
 * @brief The system coordinate
 */
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_CoordinateSystem : uint8
{
	/**< The tracking data is based on local coordinate system. */
	Local = 0, //WVR_CoordinateSystem_Local
	/**< The tracking data is based on global coordinate system. */
	World = 1, //WVR_CoordinateSystem_Global
};

static const unsigned int kVibrationDurationMax = 1000, kVibrationDurationDefault = 500;
static const float kVibrationAmplitudeDefault = 0.7f;
UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_Intensity : uint8
{
	Invalid = 0,
	/**< Vibration intensity is very weak. */
	Weak = 1,	// WVR_Intensity::WVR_Intensity_Weak
	/**< Vibration intensity is weak. */
	Light = 2,	// WVR_Intensity::WVR_Intensity_Light
	/**< Vibration intensity is normal. */
	Normal = 3,	// WVR_Intensity::WVR_Intensity_Normal
	/**< Vibration intensity is strong. */
	Strong = 4, // WVR_Intensity::WVR_Intensity_Strong
	/**< Vibration intensity is very strong. */
	Severe = 5, // WVR_Intensity::WVR_Intensity_Severe
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_EventType : uint8
{
	/// Common event region
	/**< Application Quit. */
	Quit = 0,
	/**< @ref WVR_InteractionMode changed. Use GetInteractionMode to get the interaction mode. */
	InteractionModeChanged = 1,
	/**< @ref WVR_GazeTriggerType changed. Use GetGazeTriggerType to get the gaze trigger type. */
	GazeTriggerTypeChanged = 2,
	/**< Notification for tracking mode change(3 DoF/6 DoF). Use GetSupportedNumOfDoF to get the current tracking mode.*/
	TrackingModeChanged = 3,
	/**< Notification for recommended quality to be Lower from runtime. */
	RecommendedQuality_Lower = 4,
	/**< Notification for recommended quality to be Higher from runtime. */
	RecommendedQuality_Higher = 5,
	/**< Notification for changed gesture. */
	HandGesture_Changed = 6,
	/**< Notification for abnormal gesture. */
	HandGesture_Abnormal = 7,
	/**< Notification for abnormal hand tracking. */
	HandTracking_Abnormal = 8,
	/**< Notification for arena have been changed. */
	ArenaChanged = 9,
	/**< Notification for HMD screen off. */
	RenderingToBePaused = 10,
	/**< Notification for HMD screen on. */
	RenderingToBeResumed = 11,

	/// Device events region
	/**< EWVR_DeviceType is connected. */
	DeviceConnected = 100,
	/**< EWVR_DeviceType is disconnected. */
	DeviceDisconnected = 101,
	/**< EWVR_DeviceType configuration has changed. */
	DeviceStatusUpdate = 102,
	/**< When user takes off the HMD*/
	DeviceSuspend = 103,
	/**< When user puts the HMD back on*/
	DeviceResume = 104,
	/**< The interpupillary distance (IPD) has changed. */
	IpdChanged = 105,
	/**< Controller roles have switched. */
	DeviceRoleChanged = 106,
	/**< The battery status of EWVR_DeviceType device has changed. Use getDeviceBatteryPercentage to check the current battery status. */
	BatteryStatusUpdate = 107,
	/**< The charged status of HMD has changed. */
	ChargeStatusUpdate = 108,
	/**< EWVR_DeviceType device error occurs. */
	DeviceErrorStatusUpdate = 109,
	/**< The battery temperature status of EWVR_DeviceType device has changed. */
	BatteryTemperatureStatusUpdate = 110,
	/**< Notification for successful recenter for the 6 DoF device*/
	RecenterSuccess = 111,
	/**< Notification for recenter failed for the 6 DoF device*/
	RecenterFail = 112,
	/**< Notification for recenter successful for the 3 DoF device*/
	RecenterSuccess3DoF = 113,
	/**< Notification for recenter failed for the 3 DoF device*/
	RecenterFail3DoF = 114,
	/**< Notification of the finish of clearing opeartion of HMD tracking map from device service.*/
	ClearHmdTrackingMapDone = 115,
	/**< Notification for input device mapping table changed.*/
	InputDevMappingChanged = 116,
	/**< The battery percentage of EWVR_DeviceType device has changed. Use getDeviceBatteryPercentage to check the current battery level. */
	BatteryPercentageUpdate = 117,
	/**< The device infomation of EWVR_DeviceType device has changed. */
	DeviceInfoUpdate = 118,
	/**< Notification for passthrough overlay is shown by the system.*/
	PassthroughOverlayShownBySystem = 150,
	/**< Notification for passthrough overlay is hidden by the system. */
	PassthroughOverlayHiddenBySystem = 151,
	ControllerPoseModeChanged = 152,
	ControllerPoseModeOffsetReady = 153,

	/// Input Event region
	/**< EWVR_InputId status changed to pressed. */
	ButtonPressed = 200,
	/**< EWVR_InputId status changed to not pressed */
	ButtonUnpressed = 201,
	/**< EWVR_InputId status changed to touched. */
	TouchTapped = 202,
	/**< EWVR_InputId status changed to untouched. */
	TouchUntapped = 203,
	/**< Notification for swipe motion (left to right) on the touchpad */
	LeftToRightSwipe = 204,
	/**< Notification for swipe motion (right to left) on the touchpad */
	RightToLeftSwipe = 205,
	/**< Notification for swipe motion (down to up) on the touchpad */
	DownToUpSwipe = 206,
	/**< Notification for swipe motion (up to down) on the touchpad */
	UpToDownSwipe = 207,
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_AnalogType : uint8
{
	None = 0, //WVR_AnalogType.WVR_AnalogType_None,
	XY = 1, //WVR_AnalogType.WVR_AnalogType_2D,
	XOnly = 2, //WVR_AnalogType.WVR_AnalogType_1D,
};

UENUM(BlueprintType, Category = "WaveVR")
enum class EWVR_InputType : uint8
{
	None = 0,
	Button = 1, //WVR_InputType::WVR_InputType_Button
	Touch = 2, //WVR_InputType::WVR_InputType_Touch
	Analog = 4, //WVR_InputType::WVR_InputType_Analog
};
