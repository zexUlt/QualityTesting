// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "HAL/IConsoleManager.h"

// https://docs.unrealengine.com/en-us/Programming/Development/Tools/ConsoleManager

namespace Wave {
namespace ConsoleVariable {

/*
static TAutoConsoleVariable<int32> CVarTemplate(
	TEXT("wvr."),
	1,  // Default value
	TEXT("1. Enable .\n")
	TEXT("0. Disable it.\n")
	TEXT("note, or orther description.\n"),
	ECVF_RenderThreadSafe);
*/

/****************************************************
 *
 * Console Variable: HMD
 *
 ****************************************************/

static TAutoConsoleVariable<int32> CVarEyeTrackingEnable(
	TEXT("wvr.EyeTracking.enable"),
	/*default value*/ 0,
	TEXT("1. Enable the eye tracking feature.\n")
	TEXT("0. Disable it.\n")
	TEXT("If not support by device, it will be disabled.\n"),
	ECVF_RenderThreadSafe);


/****************************************************
 *
 * Console Variable: Render
 *
 ****************************************************/

static TAutoConsoleVariable<int32> CVarColorGamutPreference0(
	TEXT("wvr.ColorGamut.preference0"),
	/*default value*/ 0,
	TEXT("  0: Display Color Gamut set as Native.  It is available for all device.  Wave will not do any color management for Native.\n")
	TEXT("  1: Display Color Gamut set as sRGB.\n")
	TEXT("  2: Display Color Gamut set as DisplayP3.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarColorGamutPreference1(
	TEXT("wvr.ColorGamut.preference1"),
	/*default value*/ 1,
	TEXT("  0: Display Color Gamut set as Native.  It is available for all device.  Wave will not do any color management for Native.\n")
	TEXT("  1: Display Color Gamut set as sRGB.\n")
	TEXT("  2: Display Color Gamut set as DisplayP3.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarColorGamutPreference2(
	TEXT("wvr.ColorGamut.preference2"),
	/*default value*/ 2,
	TEXT("  0: Display Color Gamut set as Native.  It is available for all device.  Wave will not do any color management for Native.\n")
	TEXT("  1: Display Color Gamut set as sRGB.\n")
	TEXT("  2: Display Color Gamut set as DisplayP3.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarColorGamutPreference3(
	TEXT("wvr.ColorGamut.preference3"),
	/*default value*/ 0,
	TEXT("  0: Display Color Gamut set as Native.  It is available for all device.  Wave will not do any color management for Native.\n")
	TEXT("  1: Display Color Gamut set as sRGB.\n")
	TEXT("  2: Display Color Gamut set as DisplayP3.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarTripleDepthBuffer(
	TEXT("wvr.tripleDepthBuffer"),
	/*default value*/ 0,
	TEXT("  0: Disable triple depth buffer.  PMC will not be supported.\n")
	TEXT("  1: Enable triple depth buffer.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarAMCMode(
	TEXT("wvr.AMC.mode"),
	/*default value*/ 0,
	TEXT("0. Off.  No AMC.\n")
	TEXT("1. Force UMC\n")
	TEXT("2. UMC will dynamicly turn on or off due to rendering and performance status.\n")
	TEXT("3. Force PMC.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarRenderMaskEnable(
	TEXT("wvr.RenderMask.enable"),
	/*default value*/ 1,
	TEXT("1. Enable the RenderMask to skip the rendering on invisible area.  Will not work if HMD LateUpdate is enabled.\n")
	TEXT("0. RenderMask will disable.  Render to whole output texture.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarFoveatedRenderingMode(
	TEXT("wvr.FoveatedRendering.mode"),
	/*default value*/ 2,
	TEXT("  0: disable\n")
	TEXT("  1: enable\n")
	TEXT("  2: default (decide by device)"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarFoveatedRenderingPeripheralFOV(
	TEXT("wvr.FoveatedRendering.peripheralFOV"),
	/*default value*/ 33,  // Magic number means use default.
	TEXT("The peripheral FOV in degree. Input integer number.\n"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarFoveatedRenderingPeripheralQuality(
	TEXT("wvr.FoveatedRendering.peripheralQuality"),
	/*default value*/ 2,
	TEXT("  0: low quality\n")
	TEXT("  1: middle quality\n")
	TEXT("  2: high quality (default)"),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarTimeWarpStabilizedMode(
	TEXT("wvr.TimeWarpStabilizedMode"),
	/*default value*/ 2,
	TEXT("  0: Off.\n")
	TEXT("  1: On.\n")
	TEXT("  2: Auto.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarSupportedFPS(
	TEXT("wvr.SupportedFPS"),
	/*default value*/ 0,
	TEXT("  0: Default value which depends on HMD.\n")
	TEXT("  120: Set FPS as 120.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarFadeOut(
	TEXT("wvr.FadeOut"),
	/*default value*/ 0,
	TEXT("  0: Disable.\n")
	TEXT("  1: Enable.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarFrameSharpnessEnhancement(
	TEXT("wvr.FrameSharpnessEnhancement"),
	/*default value*/ 0,
	TEXT("  0: Disable.\n")
	TEXT("  1: Enable.\n"),
	ECVF_SetByProjectSetting);

/****************************************************
 *
 * Console Variable: Direct Preview
 *
 ****************************************************/

static TAutoConsoleVariable<int32> CVarConnectType(
	TEXT("wvr.DirectPreview.ConnectType"),
	/*default value*/ 1,
	TEXT("0. USB.\n")
	TEXT("1. Wi-Fi.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<FString> CVarDeviceWiFiIP(
	TEXT("wvr.DirectPreview.DeviceWiFiIP"),
	/*default value*/ TEXT(""),
	TEXT("HMD's Wi-Fi IP.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<bool> CVarEnablePreviewImage(
	TEXT("wvr.DirectPreview.EnablePreviewImage"),
	/*default value*/ true,
	TEXT("true. Enable to preview image in the HMD.\n")
	TEXT("false. Disable to preview image in the HMD.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarUpdateFrequency(
	TEXT("wvr.DirectPreview.UpdateFrequency"),
	/*default value*/ 4,
	TEXT("0. Runtime defined.\n")
	TEXT("1. FPS_15.\n")
	TEXT("2. FPS_30.\n")
	TEXT("3. FPS_45.\n")
	TEXT("4. FPS_60.\n")
	TEXT("5. FPS_75.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<bool> CVarRegularlySaveImages(
	TEXT("wvr.DirectPreview.RegularlySaveImages"),
	/*default value*/ false,
	TEXT("true. Enable to save images regulary.\n")
	TEXT("false. Disable to save images regulary.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarRenderTargetDevice(
	TEXT("wvr.DirectPreview.RenderDevice"),
	/*default value*/ 0,
	TEXT("0. Render target device is FOCUS_3.\n")
	TEXT("1. Render target device is FOCUS or FOCUS_PLUS.\n"),
	ECVF_SetByProjectSetting);

/****************************************************
 *
 * Console Variable: Performance
 *
 ****************************************************/

static TAutoConsoleVariable<int32> CVarAdaptiveQuality(
	TEXT("wvr.AdaptiveQuality"),
	/*default value*/ 1,
	TEXT("0. Always use full speed CPU and GPU.\n")
	TEXT("1. Adjuest CPU/GPU performance by the loading.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarAdaptiveQualityMode(
	TEXT("wvr.AdaptiveQuality.mode"),
	/*default value*/ 0,
	TEXT("Description here.\n")
	TEXT("  0: Quality_Oriented\n")
	TEXT("  1: Performance_Oriented\n")
	TEXT("  2: Customization"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarAdaptiveQualitySendQualityEvent(
	TEXT("wvr.AdaptiveQuality.sendQualityEvent"),
	/*default value*/ 1,
	TEXT("0. Do not Sent events.\n")
	TEXT("1. Sent events to inform Developers to adjuest quality.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarAdaptiveQualityAutoFoveation(
	TEXT("wvr.AdaptiveQuality.autoFoveation"),
	/*default value*/ 1,
	TEXT("0. Do not adjust foveation automatically.\n")
	TEXT("1. Enable/ disable foveation or adjust PeripheralQuality automatically.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarHandTrackingEnable(
	TEXT("wvr.HandTracking.enable"),
	/*default value*/ 0,
	TEXT("0. Disable the Hand Tracking feature.\n")
	TEXT("1. Enable the Hand Tracking feature.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarTrackerEnable(
	TEXT("wvr.Tracker.enable"),
	/*default value*/ 0,
	TEXT("0. Disable the Tracker feature.\n")
	TEXT("1. Enable the Tracker feature.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarSimultaneousInteractionEnable(
	TEXT("wvr.SimultaneousInteraction.enable"),
	/*default value*/ 0,
	TEXT("0. Disable the Simultaneous Interaction feature.\n")
	TEXT("1. Enable the Simultaneous Interaction feature.\n"),
	ECVF_SetByProjectSetting);

static TAutoConsoleVariable<int32> CVarLipExpressionEnable(
	TEXT("wvr.LipExpression.enable"),
	/*default value*/ 0,
	TEXT("0. Disable the Lip Expression feature.\n")
	TEXT("1. Enable the Lip Expression feature.\n"),
	ECVF_SetByProjectSetting);
/****************************************************
 *
 * Console Variable: Debug
 *
 ****************************************************/
static TAutoConsoleVariable<int32> CVarRenderDocEnable(
	TEXT("wvr.RenderDoc.enable"),
	/*default value*/ 0,
	TEXT("0. Disable.\n")
	TEXT("1. Enabled.  RenderDoc will be installed into package.\n"),
	ECVF_SetByProjectSetting);

}  // namespace ConsoleVariable
}  // namespace Wave
