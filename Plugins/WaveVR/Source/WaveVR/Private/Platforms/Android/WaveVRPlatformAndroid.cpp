// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#if PLATFORM_ANDROID
#include "WaveVRPlatformAndroid.h"
#include <string>
#include <dlfcn.h>

#include "../WaveVRAPIWrapper.h"
#include "HAL/IConsoleManager.h"
#include "WaveVRLogAndroid.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Platforms/WaveVRLogWrapper.h"

extern JavaVM* mJavaVM;
extern JNIEnv* JavaENV;
extern jclass mRWClazz;
extern jobject mRWObj;
extern jclass mOEMClass;
extern jobject mOEMObject;
extern jclass mFUClass;
extern jobject mFUObject;
extern jclass mPWclass;
extern jobject mPWObj;

extern FString GExternalFilePath;

extern "C" void WVR_PauseATW();   // New Api to replace SetATWActive(false)
extern "C" void WVR_ResumeATW();   // New Api to replace SetATWActive(true)
extern "C" bool WVR_IsATWActive();
extern "C" void WVR_SetATWActive(bool isActive, void *anativeWindow = nullptr);
extern "C" void WVR_SetRenderThreadId(int tid);
extern "C" void WVR_SetAppEditorVersion(AppEditorVersion v);
extern "C" WVR_TextureQueueHandle_t WVR_StoreRenderTextures(uint32_t texturesIDs[], int size, bool eEye, WVR_TextureTarget target);
extern "C" void WVR_GetStencilMesh(WVR_Eye eye, uint32_t* vertexCount, uint32_t* triangleCount, uint32_t floatArrayCount, float* vertexData, uint32_t intArrayCount, int* indexData);
extern "C" void WVR_SetFocusedController(WVR_DeviceType focusedController);
extern "C" WVR_DeviceType WVR_GetFocusedController();

extern "C" bool GetFrameBufferWithPoseState(uint8_t *pframebuffer, uint32_t frameBufferSize, WVR_PoseOriginModel origin, uint32_t predictInMs, WVR_PoseState_t *poseState);
extern "C" void ReleaseAll_Unreal();
extern "C" bool DrawTextureWithBuffer_Unreal(uint32_t textureid, WVR_CameraImageFormat imgFormat, uint8_t *frameBuffer, uint32_t size, uint32_t width, uint32_t height, bool enableCropping, bool clearClampRegion, bool noErrorContext);

extern "C" WVR_SubmitError WVR_SubmitCompositionLayers(const WVR_LayerSetParams_t* param);
extern "C" uint32_t WVR_GetMaxCompositionLayerCount();

typedef bool (*WVRAPI_SetColorGamut)(ColorGamut gamut);
WVRAPI_SetColorGamut WVR_SetColorGamutLocal = nullptr;

//extern "C" WVR_AMCMode WVR_GetAMCMode();
//extern "C" WVR_AMCStatus WVR_GetAMCStatus();

typedef WVR_AMCMode (* WVRAPI_GetAMCMode)();
WVRAPI_GetAMCMode WVR_GetAMCModeLocal = nullptr;

typedef WVR_AMCStatus (* WVRAPI_GetAMCStatus)();
WVRAPI_GetAMCStatus WVR_GetAMCStatusLocal = nullptr;

FWaveVRPlatformAndroid::FWaveVRPlatformAndroid() : mHandle(nullptr) {
	LOG_FUNC();

	// No linked part

	mHandle = dlopen("libwvr_api.so", RTLD_NOLOAD);
	if (mHandle == nullptr)
		return;

	WVR_SetColorGamutLocal = (WVRAPI_SetColorGamut)dlsym(mHandle, "WVR_SetColorGamut");
	WVR_GetAMCModeLocal = (WVRAPI_GetAMCMode)dlsym(mHandle, "WVR_GetAMCMode");
	WVR_GetAMCStatusLocal = (WVRAPI_GetAMCStatus)dlsym(mHandle, "WVR_GetAMCStatus");
}

FWaveVRPlatformAndroid::~FWaveVRPlatformAndroid() {
	LOG_FUNC();
}

uint64_t FWaveVRPlatformAndroid::GetSupportedFeatures() {
	LOG_FUNC();
	return WVR_GetSupportedFeatures();
}

WVR_InitError FWaveVRPlatformAndroid::Init(WVR_AppType type) {
	LOG_FUNC();
	return WVR_Init(type);
}
void FWaveVRPlatformAndroid::Quit() {
	LOG_FUNC();
	WVR_Quit();
}
void FWaveVRPlatformAndroid::GetSyncPose(WVR_PoseOriginModel originModel, WVR_DevicePosePair_t* retPose, uint32_t PoseCount) {
	LOG_FUNC();
	WVR_GetSyncPose(originModel, retPose, PoseCount);
}

void FWaveVRPlatformAndroid::GetPoseState(WVR_DeviceType type, WVR_PoseOriginModel originModel, uint32_t predictedMilliSec, WVR_PoseState_t *poseState) {
	LOG_FUNC();
	WVR_GetPoseState(type, originModel, predictedMilliSec, poseState);
}


void FWaveVRPlatformAndroid::SetPosePredictEnabled(WVR_DeviceType type, bool enabled_position_predict, bool enabled_rotation_predict) {
	LOG_FUNC();
	WVR_SetPosePredictEnabled(type, enabled_position_predict, enabled_rotation_predict);
}
bool FWaveVRPlatformAndroid::GetInputButtonState(WVR_DeviceType type, WVR_InputId id) {
	LOG_FUNC();
	return WVR_GetInputButtonState(type, id);
}
bool FWaveVRPlatformAndroid::GetInputTouchState(WVR_DeviceType type, WVR_InputId id) {
	LOG_FUNC();
	return WVR_GetInputTouchState(type, id);
}
WVR_Axis_t FWaveVRPlatformAndroid::GetInputAnalogAxis(WVR_DeviceType type, WVR_InputId id) {
	LOG_FUNC();
	return WVR_GetInputAnalogAxis(type, id);
}
bool FWaveVRPlatformAndroid::IsDeviceConnected(WVR_DeviceType type) {
	LOG_FUNC();
	return WVR_IsDeviceConnected(type);
}
uint32_t FWaveVRPlatformAndroid::GetParameters(WVR_DeviceType type, const char* param, char* ret, uint32_t bufferSize) {
	LOG_FUNC();
	return WVR_GetParameters(type, param, ret, bufferSize);
}
WVR_NumDoF FWaveVRPlatformAndroid::GetDegreeOfFreedom(WVR_DeviceType type) {
	LOG_FUNC();
	return WVR_GetDegreeOfFreedom(type);
}
float FWaveVRPlatformAndroid::GetDeviceBatteryPercentage(WVR_DeviceType type) {
	LOG_FUNC();
	return WVR_GetDeviceBatteryPercentage(type);
}
bool FWaveVRPlatformAndroid::PollEventQueue(WVR_Event_t* event) {
	LOG_FUNC();
	return WVR_PollEventQueue(event);
}

bool FWaveVRPlatformAndroid::GetRenderProps(WVR_RenderProps_t* props) {
	LOG_FUNC();
	return WVR_GetRenderProps(props);
}
bool FWaveVRPlatformAndroid::SetInputRequest(WVR_DeviceType type, const WVR_InputAttribute* request, uint32_t size) {
	LOG_FUNC();
	return WVR_SetInputRequest(type, request, size);
}
/*bool FWaveVRPlatformAndroid::SetInteractionMode(WVR_InteractionMode mode) {
	LOG_FUNC();
	return WVR_SetInteractionMode(mode);
}*/
WVR_InteractionMode FWaveVRPlatformAndroid::GetInteractionMode() {
	LOG_FUNC();
	return WVR_GetInteractionMode();
}
bool FWaveVRPlatformAndroid::SetGazeTriggerType(WVR_GazeTriggerType type) {
	LOG_FUNC();
	return WVR_SetGazeTriggerType(type);
}
WVR_GazeTriggerType FWaveVRPlatformAndroid::GetGazeTriggerType() {
	LOG_FUNC();
	return WVR_GetGazeTriggerType();
}
void FWaveVRPlatformAndroid::SetNeckModelEnabled(bool enabled) {
	LOG_FUNC();
	WVR_SetNeckModelEnabled(enabled);
}
WVR_DeviceType FWaveVRPlatformAndroid::GetDefaultControllerRole() {
	LOG_FUNC();
	return WVR_GetDefaultControllerRole();
}
void FWaveVRPlatformAndroid::SetArmSticky(bool stickyArm) {
	LOG_FUNC();
	WVR_SetArmSticky(stickyArm);
}
void FWaveVRPlatformAndroid::SetArmModel(WVR_SimulationType type) {
	LOG_FUNC();
	WVR_SetArmModel(type);
}
void FWaveVRPlatformAndroid::InAppRecenter(WVR_RecenterType recenterType) {
	LOG_FUNC();
	WVR_InAppRecenter(recenterType);
}
void FWaveVRPlatformAndroid::SetArenaVisible(WVR_ArenaVisible config) {
	LOG_FUNC();
	WVR_SetArenaVisible(config);
}
bool FWaveVRPlatformAndroid::StartCamera(WVR_CameraInfo_t *info) {
	LOG_FUNC();
	return WVR_StartCamera(info);
}
void FWaveVRPlatformAndroid::StopCamera() {
	LOG_FUNC();
	WVR_StopCamera();
}
bool FWaveVRPlatformAndroid::GetCameraFrameBuffer(uint8_t *pframebuffer, uint32_t frameBufferSize) {
	LOG_FUNC();
	return WVR_GetCameraFrameBuffer(pframebuffer, frameBufferSize);
}
bool FWaveVRPlatformAndroid::RequestScreenshot(uint32_t width, uint32_t height, WVR_ScreenshotMode mode, const char* filename) {
	LOG_FUNC();
	return WVR_RequestScreenshot(width, height, mode, filename);
}
uint32_t FWaveVRPlatformAndroid::GetWaveRuntimeVersion() {
	LOG_FUNC();
	return WVR_GetWaveRuntimeVersion();
}
bool FWaveVRPlatformAndroid::GetInputMappingPair(WVR_DeviceType type, WVR_InputId destination, WVR_InputMappingPair* pair) {
	LOG_FUNC();
	return WVR_GetInputMappingPair(type, destination, pair);
}
uint32_t FWaveVRPlatformAndroid::GetInputMappingTable(WVR_DeviceType type, WVR_InputMappingPair* table, uint32_t size) {
	LOG_FUNC();
	return WVR_GetInputMappingTable(type, table, size);
}
bool FWaveVRPlatformAndroid::IsInputFocusCapturedBySystem() {
	LOG_FUNC();
	return WVR_IsInputFocusCapturedBySystem();
}
void FWaveVRPlatformAndroid::TriggerVibration(WVR_DeviceType type, WVR_InputId id, uint32_t durationMicroSec, uint32_t frequency, WVR_Intensity intensity) {
	LOG_FUNC();
	WVR_TriggerVibration(type, id, durationMicroSec, frequency, intensity);
}
void FWaveVRPlatformAndroid::TriggerVibrationScale(WVR_DeviceType type, WVR_InputId inputId, uint32_t durationMicroSec, uint32_t frequency, float amplitude) {
	LOG_FUNC();
	WVR_TriggerVibrationScale(type, inputId, durationMicroSec, frequency, amplitude);
}
bool FWaveVRPlatformAndroid::SetControllerPoseMode(WVR_DeviceType type, WVR_ControllerPoseMode mode) {
	LOG_FUNC();
	return WVR_SetControllerPoseMode(type, mode);
}
bool FWaveVRPlatformAndroid::GetControllerPoseMode(WVR_DeviceType type, WVR_ControllerPoseMode *mode) {
	LOG_FUNC();
	return WVR_GetControllerPoseMode(type, mode);
}
bool FWaveVRPlatformAndroid::GetControllerPoseModeOffset(WVR_DeviceType type, WVR_ControllerPoseMode mode, WVR_Vector3f_t *translation, WVR_Quatf_t *quaternion) {
	LOG_FUNC();
	return WVR_GetControllerPoseModeOffset(type, mode, translation, quaternion);
}
bool FWaveVRPlatformAndroid::IsDeviceTableStatic(WVR_DeviceType type)
{
	return WVR_IsDeviceTableStatic(type);
}
void FWaveVRPlatformAndroid::SetParameters(WVR_DeviceType type, const char *pchValue) {
	LOG_FUNC();
	WVR_SetParameters(type, pchValue);
}
#if 0 /* PTW SUPPORT */
void FWaveVRPlatformAndroid::SetATWConfig(WVR_DepthOptions opts) {
	LOG_FUNC();
	//WVR_SetATWConfig(opts);
}
#endif
bool FWaveVRPlatformAndroid::ShowPassthroughOverlay(bool show, bool delaySubmit, bool showIndicator) {
	LOG_FUNC();
	return WVR_ShowPassthroughOverlay(show, delaySubmit, showIndicator);
}
bool FWaveVRPlatformAndroid::IsPassthroughOverlayVisible() {
	LOG_FUNC();
	return WVR_IsPassthroughOverlayVisible();
}
WVR_Result FWaveVRPlatformAndroid::SetPassthroughOverlayAlpha(const float alpha) {
	LOG_FUNC();
	return WVR_SetPassthroughOverlayAlpha(alpha);
}
WVR_Result FWaveVRPlatformAndroid::ShowPassthroughUnderlay(bool show) {
	LOG_FUNC();
	return WVR_ShowPassthroughUnderlay(show);
}

WVR_Result FWaveVRPlatformAndroid::ShowProjectedPassthrough(bool show) {
	return WVR_ShowProjectedPassthrough(show);
}
WVR_Result FWaveVRPlatformAndroid::SetProjectedPassthroughAlpha(float alpha) {
	return WVR_SetProjectedPassthroughAlpha(alpha);
}
WVR_Result FWaveVRPlatformAndroid::SetProjectedPassthroughPose(const WVR_Pose_t* pose) {
	return WVR_SetProjectedPassthroughPose(pose);
}
WVR_Result FWaveVRPlatformAndroid::SetProjectedPassthroughMesh(float* vertexBuffer, uint32_t vertextCount, uint32_t* indices, uint32_t indexCount) {
	return WVR_SetProjectedPassthroughMesh(vertexBuffer, vertextCount, indices, indexCount);
}

/* Hand */
WVR_Result FWaveVRPlatformAndroid::StartHandGesture(uint64_t demands) {
	LOG_FUNC();
	return WVR_StartHandGesture(demands);
}

void FWaveVRPlatformAndroid::StopHandGesture() {
	LOG_FUNC();
	WVR_StopHandGesture();
}

WVR_Result FWaveVRPlatformAndroid::GetHandGestureInfo(WVR_HandGestureInfo_t* info) {
	return WVR_GetHandGestureInfo(info);
}

WVR_Result FWaveVRPlatformAndroid::GetHandGestureData(WVR_HandGestureData *data) {
	return WVR_GetHandGestureData(data);
}

WVR_Result FWaveVRPlatformAndroid::StartHandTracking(WVR_HandTrackerType type) {
	LOG_FUNC();
	return WVR_StartHandTracking(type);
}
void FWaveVRPlatformAndroid::StopHandTracking(WVR_HandTrackerType type) {
	LOG_FUNC();
	WVR_StopHandTracking(type);
}

WVR_Result FWaveVRPlatformAndroid::GetHandJointCount(WVR_HandTrackerType type, uint32_t* jointCount)
{
	return WVR_GetHandJointCount(type, jointCount);
}

WVR_Result FWaveVRPlatformAndroid::GetHandTrackerInfo(WVR_HandTrackerType type, WVR_HandTrackerInfo_t* info)
{
	return WVR_GetHandTrackerInfo(type, info);
}

WVR_Result FWaveVRPlatformAndroid::GetHandTrackingData(WVR_HandTrackerType trackerType,
	WVR_HandModelType modelType,
	WVR_PoseOriginModel originModel,
	WVR_HandTrackingData* skeleton,
	WVR_HandPoseData_t* pose)
{
	return WVR_GetHandTrackingData(trackerType, modelType, originModel, skeleton, pose);
}

void FWaveVRPlatformAndroid::WearWristTracker(bool wear)
{
	LOG_FUNC();
	WVR_EnhanceHandStable(wear);
}

bool FWaveVRPlatformAndroid::IsWearingTracker()
{
	return WVR_IsEnhanceHandStable();
}

/** Tracker **/
WVR_Result FWaveVRPlatformAndroid::StartTracker()
{
	LOG_FUNC();
	return WVR_StartTracker();
}
void FWaveVRPlatformAndroid::StopTracker()
{
	LOG_FUNC();
	WVR_StopTracker();
}
bool FWaveVRPlatformAndroid::IsTrackerConnected(WVR_TrackerId trackerId)
{
	return WVR_IsTrackerConnected(trackerId);
}
WVR_TrackerRole FWaveVRPlatformAndroid::GetTrackerRole(WVR_TrackerId trackerId)
{
	return WVR_GetTrackerRole(trackerId);
}
WVR_Result FWaveVRPlatformAndroid::GetTrackerCapabilities(WVR_TrackerId trackerId, WVR_TrackerCapabilities_t* capabilities)
{
	return WVR_GetTrackerCapabilities(trackerId, capabilities);
}
WVR_Result FWaveVRPlatformAndroid::GetTrackerPoseState(WVR_TrackerId trackerId, WVR_PoseOriginModel originModel, uint32_t predictedMilliSec, WVR_PoseState_t *poseState)
{
	return WVR_GetTrackerPoseState(trackerId, originModel, predictedMilliSec, poseState);
}
int32_t FWaveVRPlatformAndroid::GetTrackerInputDeviceCapability(WVR_TrackerId trackerId, WVR_InputType inputType)
{
	return WVR_GetTrackerInputDeviceCapability(trackerId, inputType);
}
WVR_AnalogType FWaveVRPlatformAndroid::GetTrackerInputDeviceAnalogType(WVR_TrackerId trackerId, WVR_InputId id)
{
	return WVR_GetTrackerInputDeviceAnalogType(trackerId, id);
}
bool FWaveVRPlatformAndroid::GetTrackerInputButtonState(WVR_TrackerId trackerId, WVR_InputId id)
{
	return WVR_GetTrackerInputButtonState(trackerId, id);
}
bool FWaveVRPlatformAndroid::GetTrackerInputTouchState(WVR_TrackerId trackerId, WVR_InputId id)
{
	return WVR_GetTrackerInputTouchState(trackerId, id);
}
WVR_Axis_t FWaveVRPlatformAndroid::GetTrackerInputAnalogAxis(WVR_TrackerId trackerId, WVR_InputId id)
{
	return WVR_GetTrackerInputAnalogAxis(trackerId, id);
}
float FWaveVRPlatformAndroid::GetTrackerBatteryLevel(WVR_TrackerId trackerId)
{
	return WVR_GetTrackerBatteryLevel(trackerId);
}
WVR_Result FWaveVRPlatformAndroid::TriggerTrackerVibration(WVR_TrackerId trackerId, uint32_t durationMicroSec, uint32_t frequency, float amplitude)
{
	LOG_FUNC();
	return WVR_TriggerTrackerVibration(trackerId, durationMicroSec, frequency, amplitude);
}
int32_t* FWaveVRPlatformAndroid::GetTrackerExtendedData(WVR_TrackerId trackerId, int32_t* validSize)
{
	return WVR_GetTrackerExtendedData(trackerId, validSize);
}
WVR_Result FWaveVRPlatformAndroid::RegisterTrackerInfoCallback(WVR_TrackerInfoNotify_t* notify)
{
	LOG_FUNC();
	return WVR_RegisterTrackerInfoCallback(notify);
}
WVR_Result FWaveVRPlatformAndroid::UnregisterTrackerInfoCallback()
{
	LOG_FUNC();
	return WVR_UnregisterTrackerInfoCallback();
}

/** Lip Expression **/
WVR_Result FWaveVRPlatformAndroid::StartLipExp()
{
	LOG_FUNC();
	return WVR_StartLipExp();
}
void FWaveVRPlatformAndroid::StopLipExp()
{
	LOG_FUNC();
	WVR_StopLipExp();
}
WVR_Result FWaveVRPlatformAndroid::GetLipExpData(float* value)
{
	return WVR_GetLipExpData(value);
}

/* Eye */
WVR_Result FWaveVRPlatformAndroid::StartEyeTracking() {
	LOG_FUNC();
	return WVR_StartEyeTracking();
}
void FWaveVRPlatformAndroid::StopEyeTracking() {
	LOG_FUNC();
	WVR_StopEyeTracking();
}
WVR_Result FWaveVRPlatformAndroid::GetEyeTracking(WVR_EyeTracking_t *data, WVR_CoordinateSystem space) {
	//LOG_FUNC();
	return WVR_GetEyeTracking(data, space);
}

/* Eye Expression */
WVR_Result FWaveVRPlatformAndroid::StartEyeExp() {
	LOG_FUNC();
	return WVR_StartEyeExp();
}
void FWaveVRPlatformAndroid::StopEyeExp() {
	LOG_FUNC();
	return WVR_StopEyeExp();
}
WVR_Result FWaveVRPlatformAndroid::GetEyeExpData(WVR_EyeExp_t* value) {
	return WVR_GetEyeExpData(value);
}

WVR_RenderError FWaveVRPlatformAndroid::RenderInit(const WVR_RenderInitParams_t* param) {
	LOG_FUNC();
	return WVR_RenderInit(param);
}
WVR_Matrix4f_t FWaveVRPlatformAndroid::GetTransformFromEyeToHead(WVR_Eye eye, WVR_NumDoF dof) {
	LOG_FUNC();
	return WVR_GetTransformFromEyeToHead(eye, dof);
}
WVR_SubmitError FWaveVRPlatformAndroid::SubmitFrame(WVR_Eye eye, const WVR_TextureParams_t *param, const WVR_PoseState_t* pose, WVR_SubmitExtend extendMethod) {
	LOG_FUNC();
	return WVR_SubmitFrame(eye, param, pose, extendMethod);
}
WVR_TextureQueueHandle_t FWaveVRPlatformAndroid::ObtainTextureQueue(WVR_TextureTarget target, WVR_TextureFormat format, WVR_TextureType type, uint32_t width, uint32_t height, int32_t level) {
	LOG_FUNC();
	return WVR_ObtainTextureQueue(target, format, type, width, height, level);
}
uint32_t FWaveVRPlatformAndroid::GetTextureQueueLength(WVR_TextureQueueHandle_t handle) {
	LOG_FUNC();
	return WVR_GetTextureQueueLength(handle);
}
int32_t FWaveVRPlatformAndroid::GetAvailableTextureIndex(WVR_TextureQueueHandle_t handle) {
	LOG_FUNC();
	return WVR_GetAvailableTextureIndex(handle);
}
WVR_TextureParams_t FWaveVRPlatformAndroid::GetTexture(WVR_TextureQueueHandle_t handle, int32_t index) {
	LOG_FUNC();
	return WVR_GetTexture(handle, index);
}
void FWaveVRPlatformAndroid::ReleaseTextureQueue(WVR_TextureQueueHandle_t handle) {
	LOG_FUNC();
	WVR_ReleaseTextureQueue(handle);
}
bool FWaveVRPlatformAndroid::IsRenderFoveationSupport() {
	LOG_FUNC();
	return WVR_IsRenderFoveationSupport();
}
bool FWaveVRPlatformAndroid::IsRenderFoveationEnabled() {
	LOG_FUNC();
	return WVR_IsRenderFoveationEnabled();
}
WVR_Result FWaveVRPlatformAndroid::RenderFoveationMode(WVR_FoveationMode mode) {
	LOG_FUNC();
	return WVR_RenderFoveationMode(mode);
}

WVR_Result FWaveVRPlatformAndroid::SetFoveationConfig(const WVR_Eye eye, const WVR_RenderFoveationParams_t *foveatedParam) {
	LOG_FUNC();
	return WVR_SetFoveationConfig(eye, foveatedParam);
}

WVR_Result FWaveVRPlatformAndroid::GetFoveationDefaultConfig(const WVR_Eye eye, WVR_RenderFoveationParams_t *foveatedParam) {
	LOG_FUNC();
	return WVR_GetFoveationDefaultConfig(eye, foveatedParam);
}

bool FWaveVRPlatformAndroid::IsAdaptiveQualityEnabled() {
	LOG_FUNC();
	return WVR_IsAdaptiveQualityEnabled();
}
bool FWaveVRPlatformAndroid::EnableAdaptiveQuality(bool enable, uint32_t strategyFlags) {
	LOG_FUNC();
	return WVR_EnableAdaptiveQuality(enable, strategyFlags);
}

WVR_Result FWaveVRPlatformAndroid::EnableTimeWarpStabilizedMode(WVR_TimeWarpStabilizedMode mode) {
	LOG_FUNC();
	return WVR_EnableTimeWarpStabilizedMode(mode);
}

void FWaveVRPlatformAndroid::GetRenderTargetSize(uint32_t* width, uint32_t * height) {
	LOG_FUNC();
	WVR_GetRenderTargetSize(width, height);
}
void FWaveVRPlatformAndroid::GetClippingPlaneBoundary(WVR_Eye eye, float * left, float * right, float * top, float * bottom) {
	LOG_FUNC();
	WVR_GetClippingPlaneBoundary(eye, left, right, top, bottom);
}
void FWaveVRPlatformAndroid::PreRenderEye(WVR_Eye eye, const WVR_TextureParams_t *textureParam, const WVR_RenderFoveationParams_t* foveatedParam) {
	LOG_FUNC();
	WVR_PreRenderEye(eye, textureParam, foveatedParam);
}
WVR_Result FWaveVRPlatformAndroid::SetFrameSharpnessEnhancementLevel(float level) {
	LOG_FUNC();
	return WVR_SetFrameSharpnessEnhancementLevel(level);
}

/* Overlay */
WVR_OverlayError FWaveVRPlatformAndroid::GenOverlay(int32_t *overlayId) {
	LOG_FUNC();
	return WVR_GenOverlay(overlayId);
}
WVR_OverlayError FWaveVRPlatformAndroid::DelOverlay(int32_t overlayId) {
	LOG_FUNC();
	return WVR_DelOverlay(overlayId);
}
WVR_OverlayError FWaveVRPlatformAndroid::SetOverlayTextureId(int32_t overlayId, const WVR_OverlayTexture_t *texture) {
	LOG_FUNC();
	return WVR_SetOverlayTextureId(overlayId, texture);
}
WVR_OverlayError FWaveVRPlatformAndroid::SetOverlayFixedPosition(int32_t overlayId, const WVR_OverlayPosition_t *position) {
	LOG_FUNC();
	return WVR_SetOverlayFixedPosition(overlayId, position);
}
WVR_OverlayError FWaveVRPlatformAndroid::ShowOverlay(int32_t overlayId) {
	LOG_FUNC();
	return WVR_ShowOverlay(overlayId);
}
WVR_OverlayError FWaveVRPlatformAndroid::HideOverlay(int32_t overlayId) {
	LOG_FUNC();
	return WVR_HideOverlay(overlayId);
}
bool FWaveVRPlatformAndroid::IsOverlayValid(int32_t overlayId) {
	LOG_FUNC();
	return WVR_IsOverlayValid(overlayId);
}
bool FWaveVRPlatformAndroid::IsOverlayShow(int32_t overlayId) {
	LOG_FUNC();
	return WVR_IsOverlayShow(overlayId);
}

/* WVR internal API */
bool FWaveVRPlatformAndroid::IsATWActive() {
	LOG_FUNC();
	return WVR_IsATWActive();
}
void FWaveVRPlatformAndroid::SetATWActive(bool isActive, void *anativeWindow) {
	LOG_FUNC();
	WVR_SetATWActive(isActive, anativeWindow);
}
void FWaveVRPlatformAndroid::PauseATW() {
	LOG_FUNC();
	WVR_PauseATW();
}
void FWaveVRPlatformAndroid::ResumeATW() {
	LOG_FUNC();
	WVR_ResumeATW();
}
bool FWaveVRPlatformAndroid::SetColorGamut(ColorGamut gamut) {
	LOG_FUNC();
	if (WVR_SetColorGamutLocal == nullptr)
		return gamut == ColorGamut_Native;
	return WVR_SetColorGamutLocal(gamut);
}
WVR_Result FWaveVRPlatformAndroid::SetAMCMode(WVR_AMCMode mode) {
	LOG_FUNC();
	auto CVarTripleDepthBuffer = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.tripleDepthBuffer"));
	bool isTripleDepthBufferEnabled = (bool)CVarTripleDepthBuffer->GetValueOnAnyThread();
	if (!isTripleDepthBufferEnabled) {
		if (mode == WVR_AMCMode_Auto) {
			mode = WVR_AMCMode_Force_UMC;
		} else if (mode == WVR_AMCMode_Force_PMC) {
			mode = WVR_AMCMode_Off;
		}
	}
	return WVR_SetAMCMode(mode);
}
WVR_AMCMode FWaveVRPlatformAndroid::GetAMCMode() {
	LOG_FUNC();
	if (WVR_GetAMCModeLocal == nullptr)
		return WVR_AMCMode_Off;
	return WVR_GetAMCModeLocal();
}
WVR_AMCStatus FWaveVRPlatformAndroid::GetAMCStatus() {
	LOG_FUNC();
	if (WVR_GetAMCStatusLocal == nullptr)
		return WVR_AMCStatus_Off;
	return WVR_GetAMCStatusLocal();

}
void FWaveVRPlatformAndroid::SetRenderThreadId(int tid) {
	LOG_FUNC();
	WVR_SetRenderThreadId(tid);
}

void FWaveVRPlatformAndroid::SetAppEditorVersion(AppEditorVersion v) {
	LOG_FUNC();
	LOGI(FWaveVRPlatformAndroid, "SetAppEditorVersion");
	WVR_SetAppEditorVersion(v);
}

WVR_TextureQueueHandle_t FWaveVRPlatformAndroid::StoreRenderTextures(uint32_t texturesIDs[], int size, bool eEye, WVR_TextureTarget target) {
	LOG_FUNC();
	return WVR_StoreRenderTextures(texturesIDs, size, eEye, target);
}
void FWaveVRPlatformAndroid::GetStencilMesh(WVR_Eye eye, uint32_t* vertexCount, uint32_t* triangleCount, uint32_t floatArrayCount, float* vertexData, uint32_t intArrayCount, int* indexData) {
	LOG_FUNC();
	WVR_GetStencilMesh(eye, vertexCount, triangleCount, floatArrayCount, vertexData, intArrayCount, indexData);
}
void FWaveVRPlatformAndroid::SetFocusedController(WVR_DeviceType focusedController) {
	LOG_FUNC();
	WVR_SetFocusedController(focusedController);
}
WVR_DeviceType FWaveVRPlatformAndroid::GetFocusedController() {
	LOG_FUNC();
	return WVR_GetFocusedController();
}

bool FWaveVRPlatformAndroid::SetChecker(bool enable) {
	LOG_FUNC();
	return WVR_SetChecker(enable);
}

std::string FWaveVRPlatformAndroid::GetStringBySystemLanguage(std::string stringName) {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return stringName;
	}

	if ((mRWClazz == nullptr) || (mRWObj == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mRWClazz or mRWObj is nullptr");
		JavaENVFinish();
		return stringName;
	}

	jmethodID methodid = JavaENV->GetMethodID(mRWClazz, "getStringByName", "(Ljava/lang/String;)Ljava/lang/String;");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: getString Method cant be found T_T ");
		JavaENVFinish();
		return stringName;
	}

	jstring sn = JavaENV->NewStringUTF(stringName.c_str());
	jstring retStr = (jstring)JavaENV->CallObjectMethod(mRWObj, methodid, sn);
	JavaENV->DeleteLocalRef(sn);

	const char *str;
	str = JavaENV->GetStringUTFChars(retStr, 0);
	std::string resString(str);

	LOGI(FWaveVRPlatformAndroid, "GetStringBySystemLanguage, input = %s, result = %s", stringName.c_str(), str);

	JavaENV->ReleaseStringUTFChars(retStr, str);
	JavaENV->DeleteLocalRef(retStr);
	JavaENVFinish();

	return resString;
}

std::string FWaveVRPlatformAndroid::GetStringByLanguage(std::string stringName, std::string lang, std::string country) {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return stringName;
	}

	if ((mRWClazz == nullptr) || (mRWObj == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mRWClazz or mRWObj is nullptr");
		JavaENVFinish();
		return stringName;
	}

	jmethodID methodid = JavaENV->GetMethodID(mRWClazz, "getPreferredStringByName", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: getPreferredStringByName Method cant be found T_T ");
		JavaENVFinish();
		return stringName;
	}

	jstring sn1 = JavaENV->NewStringUTF(stringName.c_str());
	jstring sn2 = JavaENV->NewStringUTF(lang.c_str());
	jstring sn3 = JavaENV->NewStringUTF(country.c_str());
	jstring retStr = (jstring)JavaENV->CallObjectMethod(mRWObj, methodid, sn1, sn2, sn3);
	JavaENV->DeleteLocalRef(sn1);
	JavaENV->DeleteLocalRef(sn2);
	JavaENV->DeleteLocalRef(sn3);

	const char *str;
	str = JavaENV->GetStringUTFChars(retStr, 0);
	std::string resString(str);

	LOGI(FWaveVRPlatformAndroid, "getPreferredStringByName, input = %s, lang = %s, country = %s, result = %s", stringName.c_str(), lang.c_str(), country.c_str(), str);

	JavaENV->ReleaseStringUTFChars(retStr, str);
	JavaENV->DeleteLocalRef(retStr);
	JavaENVFinish();

	return resString;
}

std::string FWaveVRPlatformAndroid::GetSystemLanguage() {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return std::string(" ");
	}

	if ((mRWClazz == nullptr) || (mRWObj == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mRWClazz or mRWObj is nullptr");
		JavaENVFinish();
		return std::string(" ");
	}

	jmethodID methodid = JavaENV->GetMethodID(mRWClazz, "getSystemLanguage", "()Ljava/lang/String;");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: getSystemLanguage Method cant be found T_T ");
		JavaENVFinish();
		return std::string(" ");
	}

	jstring retStr = (jstring)JavaENV->CallObjectMethod(mRWObj, methodid);

	const char *str;
	str = JavaENV->GetStringUTFChars(retStr, 0);
	std::string resString(str);

	LOGI(FWaveVRPlatformAndroid, "getSystemLanguage, str = %s", str);

	JavaENV->ReleaseStringUTFChars(retStr, str);
	JavaENV->DeleteLocalRef(retStr);
	JavaENVFinish();

	return resString;
}

std::string FWaveVRPlatformAndroid::GetSystemCountry() {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return std::string(" ");
	}

	if ((mRWClazz == nullptr) || (mRWObj == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mRWClazz or mRWObj is nullptr");
		JavaENVFinish();
		return std::string(" ");
	}

	jmethodID methodid = JavaENV->GetMethodID(mRWClazz, "getSystemCountry", "()Ljava/lang/String;");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: getSystemCountry Method cant be found T_T ");
		JavaENVFinish();
		return std::string(" ");
	}

	jstring retStr = (jstring)JavaENV->CallObjectMethod(mRWObj, methodid);

	const char *str;
	str = JavaENV->GetStringUTFChars(retStr, 0);
	std::string resString(str);

	LOGI(FWaveVRPlatformAndroid, "getSystemCountry, str = %s", str);

	JavaENV->ReleaseStringUTFChars(retStr, str);
	JavaENV->DeleteLocalRef(retStr);
	JavaENVFinish();

	return resString;
}

std::string FWaveVRPlatformAndroid::GetOEMConfigRawData(std::string key) {
	std::string nullString("");
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return nullString;
	}

	if ((mOEMClass == nullptr) || (mOEMObject == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mOEMClass or mOEMObject is nullptr");
		JavaENVFinish();
		return nullString;
	}

	jmethodID methodid = JavaENV->GetMethodID(mOEMClass, "getJsonRawDataByKey", "(Ljava/lang/String;)Ljava/lang/String;");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: getString Method cant be found T_T ");
		JavaENVFinish();
		return nullString;
	}

	jstring sn = JavaENV->NewStringUTF(key.c_str());
	jstring retStr = (jstring)JavaENV->CallObjectMethod(mOEMObject, methodid, sn);
	JavaENV->DeleteLocalRef(sn);

	const char *str;
	str = JavaENV->GetStringUTFChars(retStr, 0);
	std::string resString(str);

	LOGI(FWaveVRPlatformAndroid, "GetOEMConfigRawData, key = %s, result = %s \n", key.c_str(), str);

	JavaENV->ReleaseStringUTFChars(retStr, str);
	JavaENV->DeleteLocalRef(retStr);
	JavaENVFinish();

	return resString;
}

std::string FWaveVRPlatformAndroid::DeployRenderModelAssets(int deviceIndex, std::string renderModelName) {
	std::string nullString("");
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return nullString;
	}

	if ((mFUClass == nullptr) || (mFUObject == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mFUClass or mFUObject is nullptr");
		JavaENVFinish();
		return nullString;
	}

	jmethodID methodid = JavaENV->GetMethodID(mFUClass, "doUnZIPAndDeploy", "(Ljava/lang/String;I)Ljava/lang/String;");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: doDeployControllerModel Method cant be found T_T ");
		JavaENVFinish();
		return nullString;
	}

	jstring jStr = JavaENV->NewStringUTF(renderModelName.c_str());
	jstring retStr = (jstring)JavaENV->CallObjectMethod(mFUObject, methodid, jStr, deviceIndex);
	JavaENV->DeleteLocalRef(jStr);

	const char *str;
	str = JavaENV->GetStringUTFChars(retStr, 0);
	std::string resString(str);

	LOGI(FWaveVRPlatformAndroid, "doDeployControllerModel, deploy path = %s", str);

	JavaENV->ReleaseStringUTFChars(retStr, str);
	JavaENV->DeleteLocalRef(retStr);

	JavaENVFinish();

	return resString;
}

std::string FWaveVRPlatformAndroid::GetRootRelativePath() {
	FString PathStr = "";
	TArray<FString> Folders;
	LOGI(FWaveVRPlatformAndroid, "GExternalFilePath: %s", TCHAR_TO_UTF8(*GExternalFilePath));

	GExternalFilePath.ParseIntoArray(Folders, TEXT("/"));
	for (FString Folder : Folders) {
		PathStr += FString("/..");
	}
	LOGI(FWaveVRPlatformAndroid, "GetRootRelativePath PathStr: %s", TCHAR_TO_UTF8(*PathStr));
	std::string retString(TCHAR_TO_ANSI(*PathStr));
	return retString;
}

bool FWaveVRPlatformAndroid::RequestPermissions(std::vector<std::string> permissions) {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return false;
	}

	if ((mPWclass == nullptr) || (mFUObject == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mPWclass or mPWObj is nullptr");
		JavaENVFinish();
		return false;
	}

	jmethodID methodid = JavaENV->GetMethodID(mPWclass, "requestPermissions", "([Ljava/lang/String;)Z");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "ERROR: requestPermissions Method cant be found T_T ");
		JavaENVFinish();
		return false;
	}
	int len = permissions.size();
	jstring str;
	jobjectArray permissionArray = 0;
	jclass strClass = JavaENV->FindClass("java/lang/String");
	permissionArray = JavaENV->NewObjectArray(len, strClass, 0);

	for (int i = 0; i < len; i++)
	{
		str = JavaENV->NewStringUTF(permissions[i].c_str());
		JavaENV->SetObjectArrayElement(permissionArray, i, str);
		JavaENV->DeleteLocalRef(str);
	}
	bool ret = JavaENV->CallBooleanMethod(mPWObj, methodid, permissionArray);

	LOGI(FWaveVRPlatformAndroid, "requestPermissions, ret = %d", ret);

	JavaENV->DeleteLocalRef(strClass);
	JavaENV->DeleteLocalRef(permissionArray);

	JavaENVFinish();
	return ret;
}

bool FWaveVRPlatformAndroid::RequestUsbPermission() {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return false;
	}

	if ((mPWclass == nullptr) || (mFUObject == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mPWclass or mPWObj is nullptr");
		JavaENVFinish();
		return false;
	}

	jmethodID methodid = JavaENV->GetMethodID(mPWclass, "requestUsbPermission", "()Z");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "ERROR: requestUsbPermission Method cant be found T_T ");
		JavaENVFinish();
		return false;
	}

	bool ret = JavaENV->CallBooleanMethod(mPWObj, methodid);
	LOGI(FWaveVRPlatformAndroid, "requestUsbPermission, ret = %d", ret);

	JavaENVFinish();
	return ret;
}

bool FWaveVRPlatformAndroid::IsPermissionGranted(std::string permission) {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return false;
	}

	if ((mPWclass == nullptr) || (mFUObject == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mPWclass or mPWObj is nullptr");
		JavaENVFinish();
		return false;
	}
	jmethodID methodid = JavaENV->GetMethodID(mPWclass, "isPermissionGranted", "(Ljava/lang/String;)Z");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "ERROR: isPermissionGranted Method cant be found T_T ");
		JavaENVFinish();
		return false;
	}

	jstring jStr = JavaENV->NewStringUTF(permission.c_str());

	bool ret = JavaENV->CallBooleanMethod(mPWObj, methodid, jStr);

	LOGE(FWaveVRPlatformAndroid, "isPermissionGranted, ret = %d", ret);
	JavaENV->DeleteLocalRef(jStr);

	JavaENVFinish();

	return ret;
}

bool FWaveVRPlatformAndroid::ShowDialogOnVRScene() {
	if (!JavaENVCheck()) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: AndroidJavaENVCheck fails ");
		return false;
	}

	if ((mPWclass == nullptr) || (mFUObject == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "ERROR: mPWclass or mPWObj is nullptr");
		JavaENVFinish();
		return false;
	}

	jmethodID methodid = JavaENV->GetMethodID(mPWclass, "showDialogOnVRScene", "()Z");

	if (!methodid)
	{
		LOGE(FWaveVRPlatformAndroid, "ERROR: showDialogOnVRScene Method cant be found T_T ");
		JavaENVFinish();
		return false;
	}

	bool ret = JavaENV->CallBooleanMethod(mPWObj, methodid);

	LOGE(FWaveVRPlatformAndroid, "showDialogOnVRScene, ret = %d", ret);

	JavaENVFinish();

	return ret;
}

/* Controller model shared from runtime */
WVR_Result FWaveVRPlatformAndroid::GetCurrentControllerModel(WVR_DeviceType ctrlerType, WVR_CtrlerModel_t **ctrlerModel, bool isOneBone) {
	LOG_FUNC();
	return WVR_GetCurrentControllerModel(ctrlerType, ctrlerModel, isOneBone);
}

void FWaveVRPlatformAndroid::ReleaseControllerModel(WVR_CtrlerModel_t **ctrlerModel) {
	LOG_FUNC();
	WVR_ReleaseControllerModel(ctrlerModel);
}

WVR_Result FWaveVRPlatformAndroid::GetCtrlerModelAnimNodeData(WVR_DeviceType ctrlerType, WVR_CtrlerModelAnimData_t **ctrlModelAnimData) {
	LOG_FUNC();
	return WVR_GetCtrlerModelAnimNodeData(ctrlerType, ctrlModelAnimData);
}

void FWaveVRPlatformAndroid::ReleaseCtrlerModelAnimNodeData(WVR_CtrlerModelAnimData_t **ctrlModelAnimData) {
	LOG_FUNC();
	WVR_ReleaseCtrlerModelAnimNodeData(ctrlModelAnimData);
}

bool FWaveVRPlatformAndroid::JavaENVCheck() {
	if ((JavaENV == nullptr) || (mJavaVM == nullptr)) {
		LOGE(FWaveVRPlatformAndroid, "%s", "Cant find Java class/method.");
		return false;
	}

	mJavaAttached = false;
	switch (mJavaVM->GetEnv((void**)&JavaENV, JNI_VERSION_1_6)) {
	case JNI_OK:
		break;
	case JNI_EDETACHED:
		if (mJavaVM->AttachCurrentThread(&JavaENV, nullptr) != JNI_OK) {
			LOGE(FWaveVRPlatformAndroid, "%s", "failed to attach current thread");
			return false;
		}
		mJavaAttached = true;
		break;
	case JNI_EVERSION:
		break;
	}

	return true;
}

void FWaveVRPlatformAndroid::JavaENVFinish() {
	if (mJavaAttached) {
		mJavaVM->DetachCurrentThread();
	}
}

bool FWaveVRPlatformAndroid::CamUtil_GetFrameBufferWithPoseState(uint8_t *pframebuffer, uint32_t frameBufferSize, WVR_PoseOriginModel origin, uint32_t predictInMs, WVR_PoseState_t *poseState) {
	LOG_FUNC();
	return GetFrameBufferWithPoseState(pframebuffer, frameBufferSize, origin, predictInMs, poseState);
}

void FWaveVRPlatformAndroid::CamUtil_ReleaseCameraTexture() {
	LOG_FUNC();
	return ReleaseAll_Unreal();
}

bool FWaveVRPlatformAndroid::CamUtil_DrawTextureWithBuffer(uint32_t textureid, WVR_CameraImageFormat imgFormat, uint8_t *frameBuffer, uint32_t size, uint32_t width, uint32_t height, bool enableCropping, bool clearClampRegion, bool noErrorContext) {
	LOG_FUNC();
	return DrawTextureWithBuffer_Unreal(textureid, imgFormat, frameBuffer, size, width, height, enableCropping, clearClampRegion, noErrorContext);
}

WVR_SubmitError FWaveVRPlatformAndroid::SubmitCompositionLayers(const WVR_LayerSetParams_t* param)
{
	LOG_FUNC();
	return WVR_SubmitCompositionLayers(param);
}

uint32_t FWaveVRPlatformAndroid::GetMaxCompositionLayerCount()
{
	LOG_FUNC();
	return WVR_GetMaxCompositionLayerCount();
}

#endif //PLATFORM_ANDROID
