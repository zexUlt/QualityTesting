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

#include "CoreMinimal.h"  // Always be first included

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"

#include "WaveVRBlueprintTypes.h"

#include "WaveVRBlueprintFunctionLibrary.generated.h"  // Always be last included

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRBPFunLib, Log, All);

UCLASS()
class WAVEVR_API UWaveVRBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the position and rotation of a device."))
	static bool GetDevicePose(FVector& OutPosition, FRotator& OutOrientation, EWVR_DeviceType Type = EWVR_DeviceType::DeviceType_HMD);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the position and rotation of a device."))
	static bool GetDevicePose2(FVector& OutPosition, FQuat& OutOrientation, EWVR_DeviceType Type = EWVR_DeviceType::DeviceType_HMD);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To enable or disable position and rotation prediction of the device. HMD always apply rotation prediction and cannot be disabled. HMD position prediction and controller pose prediction are disabled by default."))
	static void SetPosePredictEnabled(EWVR_DeviceType Type, bool enabled_position_predict, bool enabled_rotation_predict);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the pose matrix of the device in view point with the specified type.  Data are in left hand rules and positive Z is backward."))
	static FMatrix GetDeviceMatrix(EWVR_DeviceType type = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the velocity of a device. The data is in left hand  Z-up coordinate."))
	static FVector GetDeviceVelocity(EWVR_DeviceType type = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the angular velocity of a device. The data is in left hand  Z-up coordinate."))
	static FVector GetDeviceAngularVelocity(EWVR_DeviceType type = EWVR_DeviceType::DeviceType_Controller_Right);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To check if a device is connected."))
	static bool IsDeviceConnected(EWVR_DeviceType Type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To check if a device has valid poses."))
	static bool IsDevicePoseValid(EWVR_DeviceType Type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the supported degree of freedom of a device."))
	static EWVR_DOF GetSupportedNumOfDoF(EWVR_DeviceType Type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "The position of HMD is (0,0,0) if IsTrackingPosition is false."))
	static void SetTrackingHMDPosition(bool IsTrackingPosition);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To get the latest status of calling SetTrackingHMDPosition."))
	static bool IsTrackingHMDPosition();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "The rotation of HMD is (0,0,0) if IsTrackingRotation is false."))
	static void SetTrackingHMDRotation(bool IsTrackingRotation);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "Set the HMD and controllers to rotation-only. Calling SetTrackingOrigin will cancel this effect."))
	static void SetTrackingOrigin3Dof();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check the application runs in foreground or background, false for foreground."))
	static bool IsInputFocusCapturedBySystem();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To re-center the HMD position. Please refer to ERecenterType for the re-center type."))
	static void InAppRecenter(ERecenterType type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Foveation",
		meta = (ToolTip = "To check whether the device supports the foveated render feature or not."))
	static bool IsRenderFoveationSupport();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Foveation",
		meta = (ToolTip = "To check whether the foveated render feature is enabled or not."))
	static bool IsRenderFoveationEnabled();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Foveation",
		meta = (ToolTip = "To set foveated render mode."))
	static void SetFoveationMode(EWVR_FoveationMode Mode = EWVR_FoveationMode::Default);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Foveation",
		meta = (ToolTip = "To configure the foveated render feature. EEye {LEFT, Right} means to which eye the parameter will be applied. Focal_X/Focal_Y means the X/Y coordinate of the assigned eye. The original point (0,0) resides on the center of the eye. The domain value is between {-1, 1}. FOV represents the angle of the clear region. EWVR_PeripheralQuality {Low, Medium, High} represents the resolution of the peripheral region. Warning: Please make sure you disable AdaptiveQuality or enable AdaptiveQuality with Customization mode and deselect AutoFoveation because both AdaptiveQuality's Quality/Performance oriented will enable AutoFoveation which will overwrite Foveated Rendering effects."))
	static void SetFoveationParams(EEye Eye = EEye::LEFT, float Focal_X = 0.0f, float Focal_Y = 0.0f, float FOV = 90.0f,
					EWVR_PeripheralQuality PeripheralQuality = EWVR_PeripheralQuality::High);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Foveation",
		meta = (ToolTip = "To get the foveated render parameter. EEye {LEFT, Right} means to which eye the parameter will be applied. Focal_X/Focal_Y means the X/Y coordinate of the assigned eye. The original point (0,0) resides on the center of the eye. The domain value is between {-1, 1}. FOV represents the angle of the clear region. EWVR_PeripheralQuality {Low, Medium, High} represents the resolution of the peripheral region. Warning: Please make sure you disable AdaptiveQuality or enable AdaptiveQuality with Customization mode and deselect AutoFoveation because both AdaptiveQuality's Quality/Performance oriented will enable AutoFoveation which will overwrite Foveated Rendering effects."))
	static void GetFoveationParams(EEye Eye, float& Focal_X, float& Focal_Y, float& FOV, EWVR_PeripheralQuality& PeripheralQuality);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|AdaptiveQuality",
		meta = (ToolTip = "To check if the AdaptiveQuality feature is enabled."))
	static bool IsAdaptiveQualityEnabled();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|AdaptiveQuality",
		meta = (ToolTip = "To enable or disable the AdaptiveQuality feature with events sent and/or auto adjust foveation. The AutoFoveation will overwrite foveation mode. Warning: Please make sure you don't need to use Foveated Rendering feature manually because both Quality/Performance oriented will overwrite Foveated Rendering effects.", BlueprintInternalUseOnly = "true"))
	static bool EnableAdaptiveQuality_K2(bool Enable, EAdaptiveQualityMode Mode, bool SendQualityEvent, bool AutoFoveation);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Set the AMC mode."))
	static void SetAMCMode(EAMCMode mode);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Get the AMC mode."))
	static EAMCMode GetAMCMode();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Get the AMC status."))
	static EAMCStatus GetAMCStatus();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To set the pose simulation type of the rotation-only controller. The simulation means the controller will use the arm model fake pose."))
	static void SetPoseSimulationOption(SimulatePosition Option = SimulatePosition::WhenNoPosition);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "To let the rotation-only controller which uses the simulation pose follow the head's movement."))
	static void SetFollowHead(bool follow = false);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves the connected device's battery percentage."))
	static float getDeviceBatteryPercentage(EWVR_DeviceType type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves the connected device's render model name."))
	static FString GetRenderModelName(EWVR_Hand hand);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves the HMD's eye buffer Resolution size."))
	static bool GetRenderTargetSize(FIntPoint& OutSize);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "This is an internal API, please do not call directly"))
	static AActor * LoadCustomControllerModel(EWVR_DeviceType device, EWVR_DOF dof, FTransform transform);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if the VR environment is left-handed mode."))
	static bool IsLeftHandedMode();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To enable or disable the neck model of a rotation-only head mount device. If enabled, the HMD will apply the neck model fake pose."))
	static void EnableNeckModel(bool enable);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To find the children of ParentWidget. The children widgets will be stored in ChildWidgets."))
	static int GetHoveredWidgetSeqId(UUserWidget* ParentWidget, TArray<UWidget*>& ChildWidgets);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To retrieve texture2D image from a image file."))
	static UTexture2D* GetTexture2DFromImageFile(FString imageFileName, FString imagePath, EUTex2DFmtType type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves the API level of WaveVR SDK, e.g. SDK4.0 = API Level 6."))
	static int GetWaveVRRuntimeVersion();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves the mapping key of a device button. E.g. Dominant Touchpad can be mapping from Left controller Thumbstick button."))
	static EWVR_InputId GetInputMappingPair(EWVR_DeviceType type, EWVR_InputId button);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if a device button is available."))
	static bool IsButtonAvailable(EWVR_DeviceType type, EWVR_InputId button);

#pragma region
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if a device button is pressed."))
	static bool GetInputButtonState(EWVR_DeviceType type, EWVR_InputId id);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if a device button is touched."))
	static bool GetInputTouchState(EWVR_DeviceType type, EWVR_InputId id);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if a device button is pressed."))
	static bool IsButtonPressed(EWVR_DeviceType type, EWVR_InputId id);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if a device button is pressed."))
	static bool IsButtonTouched(EWVR_DeviceType type, EWVR_TouchId id);
#pragma endregion Button

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|SplashScreen",
		meta = (ToolTip = "To configure the splash feature. InSplashTexture is the desired Texture which will show. BackGroundColor means the background color while showing a background removal texture (texture which has alpha channel). ScaleFactor means the amplification of the desired texture size, 1 is the original size. Shift means the shift offset by pixel applied to the texture in screen space. (0,0) means put the texture in the center. EnableAutoLoading means if the splash texture will show while switching between maps or not."))
	static void SetSplashParam(UTexture2D* InSplashTexture, FLinearColor BackGroundColor, float ScaleFactor, FVector2D Shift, bool EnableAutoLoading);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|SplashScreen",
		meta = (ToolTip = "Manually force showing the splash texture."))
	static void ShowSplashScreen();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|SplashScreen",
		meta = (ToolTip = "Manually force hding the splash texture."))
	static void HideSplashScreen();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|DirectPreview",
		meta = (ToolTip = "Check if Direct Preview is running."))
	static bool IsDirectPreview();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To send a particular string as parameters to a Wave service."))
	static void SetParameters(EWVR_DeviceType type, FString pchValue);

#pragma region ScreenshotMode
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Screenshot",
		meta = (ToolTip = "To set the screenshot mode. Please refer to the SDK documents for detailed information."))
	static bool ScreenshotMode(EScreenshotMode ScreenshotMode);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Screenshot",
		meta = (ToolTip = "To retrieve the file name and the saved path of the screenshot."))
	static void GetScreenshotFileInfo(FString &ImageFileName, FString &ImagePath);
#pragma endregion

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Debug",
		meta = (ToolTip = "This blueprint function is only used for debug.  The unit of gameThreadLoading and renderThreadLoading are microsecond. And the values are both limited in 100 milliseconds. Set them to simulate CPU loadings on both thread."))
	static void SimulateCPULoading(int gameThreadLoading, int renderThreadLoading);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "This blueprint function is for internal use only. LateUpdate is default enabled. doUpdateInGameThread will work if LateUpdate is enabled. And predictTimeInGameThread will work if LateUpdate and doUpdateInGameThread are both enabled. The unit of predictTimeInGameThread is millisecond, and the time will be limited in 100ms."))
	static void EnableLateUpdate(bool enable, bool doUpdateInGameThread = false, float predictTimeInGameThread = 0.0f);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager",
		meta = (ToolTip = "LateUpdate is default enabled. Use this function to check the status of LateUdate feature."))
	static bool IsLateUpdateEnabled();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager|PoseMode",
		meta = (ToolTip = "To set up the controller pose mode."))
	static bool SetControllerPoseMode(EWVR_Hand Type, EWVR_ControllerPoseMode Mode);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager|PoseMode",
		meta = (ToolTip = "To retrieve current controller pose mode."))
	static bool GetControllerPoseMode(EWVR_Hand Type, EWVR_ControllerPoseMode& OutMode);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|PoseManager|PoseMode",
		meta = (ToolTip = "To retrieve the controller offset of a pose mode."))
	static bool GetControllerPoseModeOffset(EWVR_Hand Type, EWVR_ControllerPoseMode Mode, FTransform& OutTransform);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves the actual Unreal engine world scale. E.g. 1 : 1.01 meter."))
	static float GetWorldToMetersScale();

#pragma region
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Controller",
		meta = (ToolTip = "To specify the controller used to interact with objects."))
	static void SetFocusController(EWVR_DeviceType focusedController);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Controller",
		meta = (ToolTip = "Retrieves the controller type used to interact with objects."))
	static EWVR_DeviceType GetFocusController();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Retrieves current interaction mode."))
	static EWVRInteractionMode GetInteractionMode();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To set the gaze trigger type: Timer or Button or both."))
	static bool SetGazeTriggerType(EWVRGazeTriggerType type);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To retrieve the current gaze trigger type."))
	static EWVRGazeTriggerType GetGazeTriggerType();

#pragma endregion Interaction Mode Manager
#pragma region
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "In Mixed Mode, to check if a device is idle."))
	static bool IsDeviceTableStatic(EWVR_DeviceType type);
#pragma endregion Simultaneous Interaction

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Vibrates a controller with an intensity in a duration of milliseconds."))
	static void TriggerVibration(EWVR_DeviceType controller, int32 durationMilliSec, EWVR_Intensity intensity);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "Vibrates a controller with an amplitude (0~1) in a duration of milliseconds."))
	static void TriggerVibrationScale(EWVR_DeviceType controller, int32 durationMilliSec, float amplitude);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To enhance frame sharpness. Please make sure the [Frame Sharpness Enhancement] at project settings is enabled, or this setter will not work. The level value should be between [0, 1]."))
	static void SetFrameSharpnessEnhancementLevel(float level = 0.5f);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To show or hide the passthrough overlay. Parameter show is to decide to show or hide passthrough overlay. Parameter delaySubmit is to decide to delay the renderthread when the passthrough overlay is showing to improve the latency. Parameter showIndicator is to decide if passthrough overlay will show the controller indicator or not. Warning: Passthrough overlay will not work if passthrough underlay is showing."))
	static bool ShowPassthroughOverlay(bool show, bool delaySubmit = false, bool showIndicator = false);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To check if the passthrough overlay is showing or not"))
	static bool IsPassthroughOverlayVisible();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To adjust the transparency of passthrough overlay. The alpha value should be between 0.0 to 1.0."))
	static bool SetPassthroughOverlayAlpha(const float alpha = 1.0f);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR",
		meta = (ToolTip = "To show or hide the passthrough underlay. Warning: Passthrough underlay will not work if passthrough overlay is showing."))
	static bool ShowPassthroughUnderlay(bool show);
};
