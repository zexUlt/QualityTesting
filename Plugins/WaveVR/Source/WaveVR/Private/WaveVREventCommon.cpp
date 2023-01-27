// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVREventCommon.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRHMD.h"
#include "WaveVRController.h"

bool UWaveVREventCommon::bInitialized(false);

FSettingsControllerRoleChangeNative UWaveVREventCommon::OnControllerRoleChangeNative;

FOnResumeNative UWaveVREventCommon::OnResumeNative;

FTrackingModeChangeNative UWaveVREventCommon::OnTrackingModeChangeNative;
FControllerPoseModeChangedNative UWaveVREventCommon::OnControllerPoseModeChangedNative;
FOEMConfigChangeNative UWaveVREventCommon::OnOEMConfigChangeNative;
EWVR_DOF UWaveVREventCommon::currentDoF(EWVR_DOF::DOF_SYSTEM);
bool UWaveVREventCommon::bCheckTrackingMode(false);

FAllCommonEventNative UWaveVREventCommon::OnAllEventNative;

FConnectionChangeNative UWaveVREventCommon::OnConnectionChangeNative;

FAllButtonPressNative UWaveVREventCommon::OnAllButtonPressNative_HMD;
FAllButtonPressNative UWaveVREventCommon::OnAllButtonPressNative_Right;
FAllButtonPressNative UWaveVREventCommon::OnAllButtonPressNative_Left;

FAllButtonTouchNative UWaveVREventCommon::OnAllButtonTouchNative_HMD;
FAllButtonTouchNative UWaveVREventCommon::OnAllButtonTouchNative_Right;
FAllButtonTouchNative UWaveVREventCommon::OnAllButtonTouchNative_Left;

// Sets default values for this component's properties
UWaveVREventCommon::UWaveVREventCommon()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UWaveVREventCommon::BeginPlay()
{
	Super::BeginPlay();

	// Role Change
	UWaveVREventCommon::OnControllerRoleChangeNative.AddDynamic(this, &UWaveVREventCommon::OnControllerRoleChangeHandling);
	// On Application Resume
	UWaveVREventCommon::OnResumeNative.AddDynamic(this, &UWaveVREventCommon::OnResumeHandling);
	// Tracking Mode Change
	UWaveVREventCommon::OnTrackingModeChangeNative.AddDynamic(this, &UWaveVREventCommon::OnTrackingModeChangeHandling);
	bCheckTrackingMode = true;
	UWaveVREventCommon::OnOEMConfigChangeNative.AddDynamic(this, &UWaveVREventCommon::OnOEMConfigChangeHandling);
	// All Common Event
	UWaveVREventCommon::OnAllEventNative.AddDynamic(this, &UWaveVREventCommon::OnAllEventHandling);
	// Connection Change
	UWaveVREventCommon::OnConnectionChangeNative.AddDynamic(this, &UWaveVREventCommon::OnConnectionChangeHandling);
	// Button
	UWaveVREventCommon::OnAllButtonPressNative_HMD.AddDynamic(this, &UWaveVREventCommon::OnAllButtonPressHandling_HMD);
	UWaveVREventCommon::OnAllButtonPressNative_Right.AddDynamic(this, &UWaveVREventCommon::OnAllButtonPressHandling_Right);
	UWaveVREventCommon::OnAllButtonPressNative_Left.AddDynamic(this, &UWaveVREventCommon::OnAllButtonPressHandling_Left);
	UWaveVREventCommon::OnAllButtonTouchNative_HMD.AddDynamic(this, &UWaveVREventCommon::OnAllButtonTouchHandling_HMD);
	UWaveVREventCommon::OnAllButtonTouchNative_Right.AddDynamic(this, &UWaveVREventCommon::OnAllButtonTouchHandling_Right);
	UWaveVREventCommon::OnAllButtonTouchNative_Left.AddDynamic(this, &UWaveVREventCommon::OnAllButtonTouchHandling_Left);

	bInitialized = true;
}

#pragma region
void UWaveVREventCommon::OnControllerRoleChangeHandling()
{
	OnControllerRoleChangeBp.Broadcast();
}
#pragma endregion Role Change
#pragma region
void UWaveVREventCommon::OnResumeHandling()
{
	LOGD(LogWaveVREventCommon, "OnResumeHandling()");

	OnResumeBp.Broadcast();
}
#pragma endregion On Application Resume

#pragma region
void UWaveVREventCommon::OnTrackingModeChangeHandling()
{
	LOGD(LogWaveVREventCommon, "OnTrackingModeChangeHandling() current DoF: %d", (uint8)currentDoF);

	bCheckTrackingMode = true;
}

void UWaveVREventCommon::OnControllerPoseModeChangedHandling(uint8 Device, uint8 Mode, FTransform Transform)
{
	LOGD(LogWaveVREventCommon, "OnControllerPoseModeChangedHandling() broadcast native event to blueprint.");
	OnControllerPoseModeChangedBp.Broadcast(Device, Mode, Transform);
}

void UWaveVREventCommon::OnOEMConfigChangeHandling()
{
	LOGD(LogWaveVREventCommon, "OnOEMConfigChangeHandling(), call to blueprint if any");

	OnOEMConfigChangeBp.Broadcast();
}
void UWaveVREventCommon::ShouldCheckDoFOnResume(bool check)
{
	LOGD(LogWaveVREventCommon, "ShouldCheckDoFOnResume() %d", check);
	bCheckTrackingMode = check;
}
EWVR_DOF UWaveVREventCommon::GetDegreeOfFreedom()
{
	if (!bInitialized)
	{
		currentDoF = UWaveVRBlueprintFunctionLibrary::IsTrackingHMDPosition() ? EWVR_DOF::DOF_6 : EWVR_DOF::DOF_3;
	}
	return currentDoF;
}
#pragma endregion Tracking Mode Change

#pragma region
void UWaveVREventCommon::OnAllEventHandling(int32 event_num)
{
	OnAllEventBp.Broadcast(event_num);

	WVR_EventType event = static_cast<WVR_EventType>(event_num);
	switch (event)
	{
	case WVR_EventType_Quit:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_Quit");
		OnCommonEventBp.Broadcast(EWVR_EventType::Quit);
		break;
	case WVR_EventType_InteractionModeChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_InteractionModeChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::InteractionModeChanged);
		break;
	case WVR_EventType_GazeTriggerTypeChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_GazeTriggerTypeChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::GazeTriggerTypeChanged);
		break;
	case WVR_EventType_TrackingModeChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_TrackingModeChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::TrackingModeChanged);
		break;
	case WVR_EventType_RecommendedQuality_Lower:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RecommendedQuality_Lower");
		OnCommonEventBp.Broadcast(EWVR_EventType::RecommendedQuality_Lower);
		break;
	case WVR_EventType_RecommendedQuality_Higher:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RecommendedQuality_Higher");
		OnCommonEventBp.Broadcast(EWVR_EventType::RecommendedQuality_Higher);
		break;
	case WVR_EventType_HandGesture_Changed:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_HandGesture_Changed");
		OnCommonEventBp.Broadcast(EWVR_EventType::HandGesture_Changed);
		break;
	case WVR_EventType_HandGesture_Abnormal:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_HandGesture_Abnormal");
		OnCommonEventBp.Broadcast(EWVR_EventType::HandGesture_Abnormal);
		break;
	case WVR_EventType_HandTracking_Abnormal:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_HandTracking_Abnormal");
		OnCommonEventBp.Broadcast(EWVR_EventType::HandTracking_Abnormal);
		break;
	case WVR_EventType_ArenaChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ArenaChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::ArenaChanged);
		break;
	case WVR_EventType_RenderingToBePaused:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RenderingToBePaused");
		OnCommonEventBp.Broadcast(EWVR_EventType::RenderingToBePaused);
		break;
	case WVR_EventType_RenderingToBeResumed:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RenderingToBeResumed");
		OnCommonEventBp.Broadcast(EWVR_EventType::RenderingToBeResumed);
		break;
	case WVR_EventType_DeviceConnected:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceConnected");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceConnected);
		break;
	case WVR_EventType_DeviceDisconnected:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceDisconnected");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceDisconnected);
		break;
	case WVR_EventType_DeviceStatusUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceStatusUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceStatusUpdate);
		break;
	case WVR_EventType_DeviceSuspend:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceSuspend");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceSuspend);
		break;
	case WVR_EventType_DeviceResume:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceResume");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceResume);
		break;
	case WVR_EventType_IpdChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_IpdChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::IpdChanged);
		break;
	case WVR_EventType_DeviceRoleChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceRoleChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceRoleChanged);
		break;
	case WVR_EventType_BatteryStatusUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_BatteryStatusUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::BatteryStatusUpdate);
		break;
	case WVR_EventType_ChargeStatusUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ChargeStatusUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::ChargeStatusUpdate);
		break;
	case WVR_EventType_DeviceErrorStatusUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceErrorStatusUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceErrorStatusUpdate);
		break;
	case WVR_EventType_BatteryTemperatureStatusUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_BatteryTemperatureStatusUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::BatteryTemperatureStatusUpdate);
		break;
	case WVR_EventType_RecenterSuccess:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RecenterSuccess");
		OnCommonEventBp.Broadcast(EWVR_EventType::RecenterSuccess);
		break;
	case WVR_EventType_RecenterFail:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RecenterFail");
		OnCommonEventBp.Broadcast(EWVR_EventType::RecenterFail);
		break;
	case WVR_EventType_RecenterSuccess3DoF:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RecenterSuccess3DoF");
		OnCommonEventBp.Broadcast(EWVR_EventType::RecenterSuccess3DoF);
		break;
	case WVR_EventType_RecenterFail3DoF:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RecenterFail3DoF");
		OnCommonEventBp.Broadcast(EWVR_EventType::RecenterFail3DoF);
		break;
	case WVR_EventType_ClearHmdTrackingMapDone:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ClearHmdTrackingMapDone");
		OnCommonEventBp.Broadcast(EWVR_EventType::ClearHmdTrackingMapDone);
		break;
	case WVR_EventType_InputDevMappingChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_InputDevMappingChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::InputDevMappingChanged);
		break;
	case WVR_EventType_BatteryPercentageUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_BatteryPercentageUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::BatteryPercentageUpdate);
		break;
	case WVR_EventType_DeviceInfoUpdate:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DeviceInfoUpdate");
		OnCommonEventBp.Broadcast(EWVR_EventType::DeviceInfoUpdate);
		break;
	case WVR_EventType_PassthroughOverlayShownBySystem:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_PassthroughOverlayShownBySystem");
		OnCommonEventBp.Broadcast(EWVR_EventType::PassthroughOverlayShownBySystem);
		break;
	case WVR_EventType_PassthroughOverlayHiddenBySystem:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_PassthroughOverlayHiddenBySystem");
		OnCommonEventBp.Broadcast(EWVR_EventType::PassthroughOverlayHiddenBySystem);
		break;
	case WVR_EventType_ControllerPoseModeChanged:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ControllerPoseModeChanged");
		OnCommonEventBp.Broadcast(EWVR_EventType::ControllerPoseModeChanged);
		break;
	case WVR_EventType_ControllerPoseModeOffsetReady:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ControllerPoseModeOffsetReady");
		OnCommonEventBp.Broadcast(EWVR_EventType::ControllerPoseModeOffsetReady);
		break;
	case WVR_EventType_ButtonPressed:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ButtonPressed");
		OnCommonEventBp.Broadcast(EWVR_EventType::ButtonPressed);
		break;
	case WVR_EventType_ButtonUnpressed:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_ButtonUnpressed");
		OnCommonEventBp.Broadcast(EWVR_EventType::ButtonUnpressed);
		break;
	case WVR_EventType_TouchTapped:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_TouchTapped");
		OnCommonEventBp.Broadcast(EWVR_EventType::TouchTapped);
		break;
	case WVR_EventType_TouchUntapped:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_TouchUntapped");
		OnCommonEventBp.Broadcast(EWVR_EventType::TouchUntapped);
		break;
	case WVR_EventType_LeftToRightSwipe:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_LeftToRightSwipe");
		OnCommonEventBp.Broadcast(EWVR_EventType::LeftToRightSwipe);
		break;
	case WVR_EventType_RightToLeftSwipe:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_RightToLeftSwipe");
		OnCommonEventBp.Broadcast(EWVR_EventType::RightToLeftSwipe);
		break;
	case WVR_EventType_DownToUpSwipe:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_DownToUpSwipe");
		OnCommonEventBp.Broadcast(EWVR_EventType::DownToUpSwipe);
		break;
	case WVR_EventType_UpToDownSwipe:
		LOGD(LogWaveVREventCommon, "OnAllEventHandling() WVR_EventType_UpToDownSwipe");
		OnCommonEventBp.Broadcast(EWVR_EventType::UpToDownSwipe);
		break;
	default:
		break;
	}
}
#pragma endregion All Common Event
#pragma region
void UWaveVREventCommon::OnConnectionChangeHandling(uint8 device, bool connection)
{
	LOGD(LogWaveVREventCommon, "OnConnectionChangeHandling() device: %d, connection: %d", device, (uint8)connection);
	OnConnectionChangeBp.Broadcast(device, connection);
}
#pragma endregion Connection Change
#pragma region
void UWaveVREventCommon::OnAllButtonPressHandling_HMD(uint8 button, bool down)
{
	LOGD(LogWaveVREventCommon, "OnAllButtonPressHandling_HMD() button: %d, down: %d", button, (uint8)down);
	OnAllButtonPressBp_HMD.Broadcast(button, down);
}
void UWaveVREventCommon::OnAllButtonPressHandling_Right(uint8 button, bool down)
{
	LOGD(LogWaveVREventCommon, "OnAllButtonPressHandling_Right() button: %d, down: %d", button, (uint8)down);
	OnAllButtonPressBp_Right.Broadcast(button, down);
}
void UWaveVREventCommon::OnAllButtonPressHandling_Left(uint8 button, bool down)
{
	LOGD(LogWaveVREventCommon, "OnAllButtonPressHandling_Left() button: %d, down: %d", button, (uint8)down);
	OnAllButtonPressBp_Left.Broadcast(button, down);
}
void UWaveVREventCommon::OnAllButtonTouchHandling_HMD(uint8 button, bool down)
{
	LOGD(LogWaveVREventCommon, "OnAllButtonTouchHandling_HMD() button: %d, down: %d", button, (uint8)down);
	OnAllButtonTouchBp_HMD.Broadcast(button, down);
}
void UWaveVREventCommon::OnAllButtonTouchHandling_Right(uint8 button, bool down)
{
	LOGD(LogWaveVREventCommon, "OnAllButtonTouchHandling_Right() button: %d, down: %d", button, (uint8)down);
	OnAllButtonTouchBp_Right.Broadcast(button, down);
}
void UWaveVREventCommon::OnAllButtonTouchHandling_Left(uint8 button, bool down)
{
	LOGD(LogWaveVREventCommon, "OnAllButtonTouchHandling_Left() button: %d, down: %d", button, (uint8)down);
	OnAllButtonTouchBp_Left.Broadcast(button, down);
}
#pragma endregion Button

// Called every frame
void UWaveVREventCommon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCheckTrackingMode)
	{
		// 0: DOF 3, 1: DOF 6, 2: DOF SYSTEM
		currentDoF = UWaveVRBlueprintFunctionLibrary::IsTrackingHMDPosition() ? EWVR_DOF::DOF_6 : EWVR_DOF::DOF_3;
		LOGD(LogWaveVREventCommon, "TickComponent() currentDoF: %d", (uint8)currentDoF);
		OnTrackingModeChangeBp.Broadcast();
		bCheckTrackingMode = false;
	}
	// ...
}

