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

#include "WaveVRBlueprintFunctionLibrary.h"

#include "WaveVRSettings.generated.h"

UENUM()
enum class EFoveatedRenderingMode : uint8
{
	Disable = 0x0,
	Enable = 0x1,
	/*Enable/Disable up to devices*/
	Default = 0x2,
};

UENUM()
enum class EFoveatedRenderingPeripheralQuality : uint8
{
	/* power saving is high. */
	Low = 0x0,
	/* power saving is medium. */
	Medium = 0x1,
	/* power saving is low. */
	High = 0x2,
};

UENUM()
enum class EDirectPreviewConnectType : uint8
{
	USB,
	WiFi
};

UENUM()
enum class EDirectPreviewUpdateFrequency : uint8
{
	RuntimeDefined,
	FPS_15,
	FPS_30,
	FPS_45,
	FPS_60,
	FPS_75
};

UENUM()
enum class EAMCModeS: uint8
{
	/* Always Off. */
	Off = 0,
	/* Use UMC. */
	Force_UMC,
	/* UMC will dynamicly turn on or off due to rendering and performance status. */
	Auto,
	/* Use PMC. */
	//Force_PMC
};


UENUM()
enum class EAQMode: uint8
{
	/* SendQualityEvent On, AutoFoveation On. */
	Quality_Oriented,
	/* SendQualityEvent On, AutoFoveation On. */
	Performance_Oriented,
	/* Choose SendQualityEvent and AutoFoveation manually. */
	Customization
};

UENUM()
enum class ETimeWarpStabilizedMode : uint8
{
	/* WVR_TimeWarpStabilizedMode_Off. */
	Off  = 0,
	/* WVR_TimeWarpStabilizedMode_On. */
	On   = 1,
	/* WVR_TimeWarpStabilizedMode_Auto. */
	Auto = 2,
};

UENUM()
enum class ESupportedFPS : uint8
{
	/* App will run with HMD default FPS. */
	HMD_Default = 0,
	/*
	Declare the App can support up to 120 FPS. The HMD display will switch to 120 FPS to run your App if it can support.

	CAUTION: You must tune your App to make it suitable for running at 120 FPS, or App might get worse rendering performance and cause jitter phenomenon.
	*/
	FPS_120 = 120,
};

UENUM()
enum class EDirectPreviewdDevice : uint8
{
	FOCUS_3 = 0,
	FOCUS_OR_FOCUS_PLUS = 1
};

UENUM()
enum class EColorGamut : uint8
{
	Native = 0,
	sRGB = 1,
	DisplayP3
};

/**
 * WaveVR developer settings
 */
UCLASS(config=Engine, defaultconfig)
class WAVEVREDITOR_API UWaveVRSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
#if 0
	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		EditCondition = "bPMCSupported",
		ConsoleVariable = "wvr.tripleDepthBuffer", DisplayName = "Enable triple depth buffer",
		ToolTip = "Triple depth buffer can be used for Partial Motion Compensation.  However the depth buffer usage are 3 times larger.  This config is not allowed to be changed in runtime."))
	bool bTripleDepthBuffer;
#endif

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.AMC.mode", DisplayName = "AMC Mode",
		ToolTip = "AMC can do motion compensation in both rotation and translation. If AMC is disabled, only ATW will be used.  If want to enable Auto or Force PMC mode, triple depth buffer must be enabled.  Dynamically change mode also need the triple depth mode."))
	EAMCModeS AMCMode;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.EyeTracking.enable", DisplayName = "Enable Eye tracking",
		ToolTip = "If not supported by device, it will be disabled."))
	bool bEyeTrackingEnable;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.RenderMask.enable", DisplayName = "Enable Render Mask",
		ToolTip = "RenderMask help to skip the rendering to the area of eye buffer which is not seen by player.  If device is not supported, it will be disable.  And it will be also disabled if HMD's LateUpdate is enabled.  However the LateUpdate is default enabled now."))
	bool bRenderMaskEnable;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.FoveatedRendering.mode", DisplayName = "Foveated Rendering Mode",
		ToolTip = "Foveated Rendering helps save device power. Please Ignore the following foveated parameters if you select Disable or Default. The foveated parameters depend on device if you select Default. Warning: Please make sure the Adaptive Quality AutoFoveation is disabled. And both Quality/Performance oriented will automatically enable Adaptive Quality AutoFoveation."))
	EFoveatedRenderingMode FoveatedRenderingMode;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		EditConditionHides,
		Editcondition="FoveatedRenderingMode == EFoveatedRenderingMode::Enable",
		ConsoleVariable = "wvr.FoveatedRendering.peripheralFOV", DisplayName = "Foveated Rendering Peripheral FOV",
		ToolTip = "Where to start to decrease quality.", ClampMin = "1", ClampMax = "179"))
	uint32 FoveatedRenderingPeripheralFOV;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		EditConditionHides,
		Editcondition="FoveatedRenderingMode == EFoveatedRenderingMode::Enable",
		ConsoleVariable = "wvr.FoveatedRendering.peripheralQuality", DisplayName = "Foveated Rendering Peripheral Quality",
		ToolTip = "Quality High means the image quality is good but power saving is low."))
	EFoveatedRenderingPeripheralQuality FoveatedRenderingPeripheralQuality;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.TimeWarpStabilizedMode", DisplayName = "Time Warp Stabilized Mode",
		ToolTip = "If you select On, it will reduce jitter in some case. Please refer developer guide."))
	ETimeWarpStabilizedMode TimeWarpStabilizedMode;

#if 0
	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.SupportedFPS", DisplayName = "Supported FPS [Experimental]",
		ToolTip = "Set the App supported FPS."))
	ESupportedFPS SupportedFPS;
#endif

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.FadeOut", DisplayName = "Enable Fade Out",
		ToolTip = "Select this when you make sure that you need the fade out effect. Warning: This option will increase the GPU usage."))
	bool bFadeOutEnable;

	UPROPERTY(EditAnywhere, config, Category = "Render", meta = (
		ConsoleVariable = "wvr.FrameSharpnessEnhancement", DisplayName = "Frame Sharpness Enhancement",
		ToolTip = "Select this when you make sure that you need to enhance frame sharpness (clarity of text). Warning: This option will increase the GPU usage."))
	bool bFrameSharpnessEnhancement;

#if 0
	// ColorGamut Prefer List
	UPROPERTY(EditAnywhere, config, Category = "ColorGamut", meta = (
		ConsoleVariable = "wvr.ColorGamut.preference0", DisplayName = "Preference 0",
		ToolTip = "Wave will try apply the color gamut preferences from 0 to 3.  Once a color gamut successfully applied, Wave will not try apply the rest preferences.  Native is default acceptable for all device."))
	EColorGamut colorGamutPreference0;

	UPROPERTY(EditAnywhere, config, Category = "ColorGamut", meta = (
		ConsoleVariable = "wvr.ColorGamut.preference1", DisplayName = "Preference 1",
		ToolTip = "Wave will try apply the color gamut preferences from 0 to 3.  Once a color gamut successfully applied, Wave will not try apply the rest preferences.  Native is default acceptable for all device."))
	EColorGamut colorGamutPreference1;

	UPROPERTY(EditAnywhere, config, Category = "ColorGamut", meta = (
		ConsoleVariable = "wvr.ColorGamut.preference2", DisplayName = "Preference 2",
		ToolTip = "Wave will try apply the color gamut preferences from 0 to 3.  Once a color gamut successfully applied, Wave will not try apply the rest preferences.  Native is default acceptable for all device."))
	EColorGamut colorGamutPreference2;

	UPROPERTY(EditAnywhere, config, Category = "ColorGamut", meta = (
		ConsoleVariable = "wvr.ColorGamut.preference3", DisplayName = "Preference 3",
		ToolTip = "Wave will try apply the color gamut preferences from 0 to 3.  Once a color gamut successfully applied, Wave will not try apply the rest preferences.  Native is default acceptable for all device."))
	EColorGamut colorGamutPreference3;
#endif

	UPROPERTY(EditAnywhere, config, Category = "Profile", meta = (
		ConsoleVariable = "wvr.RenderDoc.enable", DisplayName = "Enable RenderDoc",
		ToolTip = "If enabled, RenderDoc v1.9 will be installed into your package for graphic debug.  You should disable it when your debug task is done."))
	bool bRenderDocEnable;


	UPROPERTY(EditAnywhere, config, Category = "DirectPreview", meta = (
		ConsoleVariable = "wvr.DirectPreview.ConnectType", DisplayName = "Connect Type",
		ToolTip = "Choose the way to connect the HMD with PC. The Editor will get the poses of HMD in both Wi-Fi and USB type. In addition, the rendered picture (PC side) will stream back to HMD while choosiong Wi-Fi type. 5GHz Wi-fi is recommended for getting better performance."))
	EDirectPreviewConnectType DirectPreviewConnectType;

	UPROPERTY(EditAnywhere, config, Category = "DirectPreview", meta = (
		EditConditionHides,
		Editcondition="DirectPreviewConnectType == EDirectPreviewConnectType::WiFi",
		ConsoleVariable = "wvr.DirectPreview.DeviceWiFiIP", DisplayName = "Device Wi-Fi IP",
		ToolTip = "Fill in the Wi-Fi IP of HMD.(Needed)."))
	FString sDirectPreviewDeviceWiFiIP;

	UPROPERTY(EditAnywhere, config, Category = "DirectPreview", meta = (
		EditConditionHides,
		Editcondition="DirectPreviewConnectType == EDirectPreviewConnectType::WiFi",
		ConsoleVariable = "wvr.DirectPreview.RenderDevice", DisplayName = "Render Target Device Name",
		ToolTip = "Choose the HMD device which you are using."))
		EDirectPreviewdDevice DirectPreviewTargetDevice;

	UPROPERTY(EditAnywhere, config, Category = "DirectPreview", meta = (
		EditConditionHides,
		Editcondition="DirectPreviewConnectType == EDirectPreviewConnectType::WiFi",
		ConsoleVariable = "wvr.DirectPreview.EnablePreviewImage", DisplayName = "Enable Preview Image",
		ToolTip = "Enable to preview image in the HMD."))
	bool bDirectPreviewEnablePreviewImage;

	UPROPERTY(EditAnywhere, config, Category = "DirectPreview", meta = (
		EditConditionHides,
		Editcondition="DirectPreviewConnectType == EDirectPreviewConnectType::WiFi",
		ConsoleVariable = "wvr.DirectPreview.UpdateFrequency", DisplayName = "Update Frequency",
		ToolTip = "Frequency to send images to the HMD."))
	EDirectPreviewUpdateFrequency DirectPreviewUpdateFrequency;

	UPROPERTY(EditAnywhere, config, Category = "DirectPreview", meta = (
		EditConditionHides,
		Editcondition="DirectPreviewConnectType == EDirectPreviewConnectType::WiFi",
		ConsoleVariable = "wvr.DirectPreview.RegularlySaveImages", DisplayName = "Regularly Save Images",
		ToolTip = "Enable to save images regulary."))
	bool bDirectPreviewEnableRegularlySaveImages;

	UPROPERTY(EditAnywhere, config, Category = "Performance", meta = (
		ConsoleVariable = "wvr.AdaptiveQuality", DisplayName = "Enable Adaptive Quality",
		ToolTip = "AdaptiveQuality help control the CPU and GPU frequency to save power.  Default is enabled.  If turn it off, the CPU and GPU will run at full speed."))
	bool bAdaptiveQuality;

	UPROPERTY(EditAnywhere, config, Category = "Performance", meta = (
		EditConditionHides,
		Editcondition="bAdaptiveQuality",
		ConsoleVariable = "wvr.AdaptiveQuality.mode", DisplayName = "AQ Mode",
		ToolTip = "AQ Mode make it much easier to cofigure Adaptive Quality feature. Please make sure you don't need to use Foveated Rendering feature manually because both Quality/Performance oriented will overwrite Foveated Rendering effects."))
	EAQMode AQMode;

	UPROPERTY(EditAnywhere, config, Category = "Performance", meta = (
		EditConditionHides,
		Editcondition="AQMode == EAQMode::Customization && bAdaptiveQuality",
		ConsoleVariable = "wvr.AdaptiveQuality.sendQualityEvent", DisplayName = "Adaptive Quality SendQualityEvent",
		ToolTip = "Send events when the recommended quality has changed."))
	bool bAdaptiveQualitySendQualityEvent;

	UPROPERTY(EditAnywhere, config, Category = "Performance", meta = (
		EditConditionHides,
		Editcondition="AQMode == EAQMode::Customization && bAdaptiveQuality",
		ConsoleVariable = "wvr.AdaptiveQuality.autoFoveation", DisplayName = "Adaptive Quality AutoFoveation",
		ToolTip = "Automatically enable/disable Foveated Rendering and adjust Foveated Rendering PeripheralQuality if needed. Please make sure you don't need to use Foveated Rendering feature manually because this setting will overwrite Foveated Rendering effects."))
	bool bAdaptiveQualityAutoFoveation;

	UPROPERTY(EditAnywhere, config, Category = "Hand", meta = (
		ConsoleVariable = "wvr.HandTracking.enable", DisplayName = "Enable Hand Tracking",
		ToolTip = "To modify the AndroidManifest.xml for using the Wave Hand Tracking feature."))
	bool bHandTrackingEnable;

	UPROPERTY(EditAnywhere, config, Category = "Tracker", meta = (
		ConsoleVariable = "wvr.Tracker.enable", DisplayName = "Enable Tracker",
		ToolTip = "To modify the AndroidManifest.xml for using the Wave Tracker feature."))
	bool bTrackerEnable;

	UPROPERTY(EditAnywhere, config, Category = "InteractionMode", meta = (
		ConsoleVariable = "wvr.SimultaneousInteraction.enable", DisplayName = "Enable Simultaneous Interaction",
		ToolTip = "To modify the AndroidManifest.xml for using the Simultaneous Interaction feature."))
	bool bSimultaneousInteractionEnable;

	UPROPERTY(EditAnywhere, config, Category = "Lip", meta = (
		ConsoleVariable = "wvr.LipExpression.enable", DisplayName = "Enable Lip Expression",
		ToolTip = "To modify the AndroidManifest.xml for using the Lip Expression feature."))
	bool bLipExpressionEnabled;

#if 0
public:
	UPROPERTY()
	bool bPMCSupported;
#endif

public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface
};
