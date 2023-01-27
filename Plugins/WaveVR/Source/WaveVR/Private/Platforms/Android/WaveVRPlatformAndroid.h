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
#include "Platforms/WaveVRAPIWrapper.h"

class WAVEVR_API FWaveVRPlatformAndroid : public FWaveVRAPIWrapper
{
#if PLATFORM_ANDROID
public:
	FWaveVRPlatformAndroid();
	~FWaveVRPlatformAndroid();

	virtual uint64_t GetSupportedFeatures() override;
	virtual WVR_InitError Init(WVR_AppType type) override;
	virtual void Quit() override;
	virtual void GetSyncPose(WVR_PoseOriginModel originModel, WVR_DevicePosePair_t* retPose, uint32_t PoseCount) override;
	virtual void GetPoseState(WVR_DeviceType type, WVR_PoseOriginModel originModel, uint32_t predictedMilliSec, WVR_PoseState_t *poseState) override;
	virtual void SetPosePredictEnabled(WVR_DeviceType type, bool enabled_position_predict, bool enabled_rotation_predict) override;
	virtual bool GetInputButtonState(WVR_DeviceType type, WVR_InputId id) override;
	virtual bool GetInputTouchState(WVR_DeviceType type, WVR_InputId id) override;
	virtual WVR_Axis_t GetInputAnalogAxis(WVR_DeviceType type, WVR_InputId id) override;
	virtual bool IsDeviceConnected(WVR_DeviceType type) override;
	virtual uint32_t GetParameters(WVR_DeviceType type, const char* param, char* ret, uint32_t bufferSize) override;
	virtual WVR_NumDoF GetDegreeOfFreedom(WVR_DeviceType type) override;
	virtual float GetDeviceBatteryPercentage(WVR_DeviceType type) override;
	virtual bool PollEventQueue(WVR_Event_t* event) override;

	virtual bool GetRenderProps(WVR_RenderProps_t* props) override;
	virtual bool SetInputRequest(WVR_DeviceType type, const WVR_InputAttribute* request, uint32_t size) override;
	//virtual bool SetInteractionMode(WVR_InteractionMode mode) override;
	virtual WVR_InteractionMode GetInteractionMode() override;
	virtual bool SetGazeTriggerType(WVR_GazeTriggerType type) override;
	virtual WVR_GazeTriggerType GetGazeTriggerType() override;
	virtual void SetNeckModelEnabled(bool enabled) override;
	virtual WVR_DeviceType GetDefaultControllerRole() override;
	virtual void SetArmSticky(bool stickyArm) override;
	virtual void SetArmModel(WVR_SimulationType type) override;
	virtual void InAppRecenter(WVR_RecenterType recenterType) override;
	virtual void SetArenaVisible(WVR_ArenaVisible config) override;
	virtual bool StartCamera(WVR_CameraInfo_t *info) override;
	virtual void StopCamera() override;
	virtual bool GetCameraFrameBuffer(uint8_t *pframebuffer, uint32_t frameBufferSize) override;
	virtual bool RequestScreenshot(uint32_t width, uint32_t height, WVR_ScreenshotMode mode, const char* filename) override;
	virtual uint32_t GetWaveRuntimeVersion() override;
	virtual bool GetInputMappingPair(WVR_DeviceType type, WVR_InputId destination, WVR_InputMappingPair* pair) override;
	virtual uint32_t GetInputMappingTable(WVR_DeviceType type, WVR_InputMappingPair* table, uint32_t size) override;
	virtual bool IsInputFocusCapturedBySystem() override;
	virtual void TriggerVibration(WVR_DeviceType type, WVR_InputId id = WVR_InputId_Max, uint32_t durationMicroSec = 1000000, uint32_t frequency = 1, WVR_Intensity intensity = WVR_Intensity_Normal) override;
	virtual void TriggerVibrationScale(WVR_DeviceType type, WVR_InputId inputId = WVR_InputId_Max, uint32_t durationMicroSec = 65535, uint32_t frequency = 1, float amplitude = 0.0f) override;
	virtual bool SetControllerPoseMode(WVR_DeviceType type, WVR_ControllerPoseMode mode) override;
	virtual bool GetControllerPoseMode(WVR_DeviceType type, WVR_ControllerPoseMode *mode) override;
	virtual bool GetControllerPoseModeOffset(WVR_DeviceType type, WVR_ControllerPoseMode mode, WVR_Vector3f_t *translation, WVR_Quatf_t *quaternion) override;
	virtual bool IsDeviceTableStatic(WVR_DeviceType type) override;
	virtual void SetParameters(WVR_DeviceType type, const char *pchValue) override;
#if 0 /* PTW SUPPORT */
	virtual void SetATWConfig(WVR_DepthOptions opts) override;
#endif
	virtual bool ShowPassthroughOverlay(bool show, bool delaySubmit = false, bool showIndicator = false) override;
	virtual bool IsPassthroughOverlayVisible() override;
	virtual WVR_Result SetPassthroughOverlayAlpha(const float alpha = 1.0f) override;
	virtual WVR_Result ShowPassthroughUnderlay(bool show) override;

	virtual WVR_Result ShowProjectedPassthrough(bool show) override;
	virtual WVR_Result SetProjectedPassthroughAlpha(float alpha) override;
	virtual WVR_Result SetProjectedPassthroughPose(const WVR_Pose_t* pose) override;
	virtual WVR_Result SetProjectedPassthroughMesh(float* vertexBuffer, uint32_t vertextCount, uint32_t* indices, uint32_t indexCount) override;


	/* Hand */
	virtual WVR_Result StartHandGesture(uint64_t demands) override;
	virtual void StopHandGesture() override;
	virtual WVR_Result GetHandGestureInfo(WVR_HandGestureInfo_t* info) override;
	virtual WVR_Result GetHandGestureData(WVR_HandGestureData *data) override;

	virtual WVR_Result StartHandTracking(WVR_HandTrackerType type) override;
	virtual void StopHandTracking(WVR_HandTrackerType type) override;
	virtual WVR_Result GetHandJointCount(WVR_HandTrackerType type, uint32_t* jointCount) override;
	virtual WVR_Result GetHandTrackerInfo(WVR_HandTrackerType type, WVR_HandTrackerInfo_t* info) override;
	virtual WVR_Result GetHandTrackingData(WVR_HandTrackerType trackerType,
		WVR_HandModelType modelType,
		WVR_PoseOriginModel originModel,
		WVR_HandTrackingData* skeleton,
		WVR_HandPoseData_t* pose = nullptr) override;
	virtual void WearWristTracker(bool wear) override;
	virtual bool IsWearingTracker() override;

	/** Tracker **/
	virtual WVR_Result StartTracker() override;
	virtual void StopTracker() override;
	virtual bool IsTrackerConnected(WVR_TrackerId trackerId) override;
	virtual WVR_TrackerRole GetTrackerRole(WVR_TrackerId trackerId) override;
	virtual WVR_Result GetTrackerCapabilities(WVR_TrackerId trackerId, WVR_TrackerCapabilities_t* capabilities) override;
	virtual WVR_Result GetTrackerPoseState(WVR_TrackerId trackerId, WVR_PoseOriginModel originModel, uint32_t predictedMilliSec, WVR_PoseState_t *poseState) override;
	virtual int32_t GetTrackerInputDeviceCapability(WVR_TrackerId trackerId, WVR_InputType inputType) override;
	virtual WVR_AnalogType GetTrackerInputDeviceAnalogType(WVR_TrackerId trackerId, WVR_InputId id) override;
	virtual bool GetTrackerInputButtonState(WVR_TrackerId trackerId, WVR_InputId id) override;
	virtual bool GetTrackerInputTouchState(WVR_TrackerId trackerId, WVR_InputId id) override;
	virtual WVR_Axis_t GetTrackerInputAnalogAxis(WVR_TrackerId trackerId, WVR_InputId id) override;
	virtual float GetTrackerBatteryLevel(WVR_TrackerId trackerId) override;
	virtual WVR_Result TriggerTrackerVibration(WVR_TrackerId trackerId, uint32_t durationMicroSec = 65535, uint32_t frequency = 0, float amplitude = 0.0f) override;
	virtual int32_t* GetTrackerExtendedData(WVR_TrackerId trackerId, int32_t* validSize) override;
	virtual WVR_Result RegisterTrackerInfoCallback(WVR_TrackerInfoNotify_t* notify) override;
	virtual WVR_Result UnregisterTrackerInfoCallback() override;

	/** Lip Expression **/
	virtual WVR_Result StartLipExp() override;
	virtual void StopLipExp() override;
	virtual WVR_Result GetLipExpData(float* value) override;

	/* Eye */
	virtual WVR_Result StartEyeTracking() override;
	virtual void StopEyeTracking() override;
	virtual WVR_Result GetEyeTracking(WVR_EyeTracking_t *data, WVR_CoordinateSystem space = WVR_CoordinateSystem_Global) override;

	/* Eye Expression */
	virtual WVR_Result StartEyeExp() override;
	virtual void StopEyeExp() override;
	virtual WVR_Result GetEyeExpData(WVR_EyeExp_t* value) override;

	virtual WVR_RenderError RenderInit(const WVR_RenderInitParams_t* param) override;
	virtual WVR_Matrix4f_t GetTransformFromEyeToHead(WVR_Eye eye, WVR_NumDoF dof = WVR_NumDoF_6DoF) override;
	virtual WVR_SubmitError SubmitFrame(WVR_Eye eye, const WVR_TextureParams_t *param, const WVR_PoseState_t* pose = NULL, WVR_SubmitExtend extendMethod = WVR_SubmitExtend_Default) override;
	virtual WVR_TextureQueueHandle_t ObtainTextureQueue(WVR_TextureTarget target, WVR_TextureFormat format, WVR_TextureType type, uint32_t width, uint32_t height, int32_t level) override;
	virtual uint32_t GetTextureQueueLength(WVR_TextureQueueHandle_t handle) override;
	virtual int32_t GetAvailableTextureIndex(WVR_TextureQueueHandle_t handle) override;
	virtual WVR_TextureParams_t GetTexture(WVR_TextureQueueHandle_t handle, int32_t index) override;
	virtual void ReleaseTextureQueue(WVR_TextureQueueHandle_t handle) override;
	virtual bool IsRenderFoveationSupport() override;
	virtual bool IsRenderFoveationEnabled() override;
	virtual WVR_Result RenderFoveationMode(WVR_FoveationMode mode) override;
	virtual WVR_Result SetFoveationConfig(const WVR_Eye eye, const WVR_RenderFoveationParams_t *foveatedParam) override;
	virtual WVR_Result GetFoveationDefaultConfig(const WVR_Eye eye, WVR_RenderFoveationParams_t *foveatedParam) override;
	virtual bool IsAdaptiveQualityEnabled() override;
	virtual bool EnableAdaptiveQuality(bool enable, uint32_t strategyFlags = WVR_QualityStrategy_Default) override;
	virtual WVR_Result EnableTimeWarpStabilizedMode(WVR_TimeWarpStabilizedMode mode) override;

	virtual void GetRenderTargetSize(uint32_t* width, uint32_t * height) override;
	virtual void GetClippingPlaneBoundary(WVR_Eye eye, float * left, float * right, float * top, float * bottom) override;
	virtual void PreRenderEye(WVR_Eye eye, const WVR_TextureParams_t *textureParam, const WVR_RenderFoveationParams_t* foveatedParam = NULL) override;
	virtual WVR_Result SetFrameSharpnessEnhancementLevel(float level) override;

	/* Overlay */
	virtual WVR_OverlayError GenOverlay(int32_t *overlayId) override;
	virtual WVR_OverlayError DelOverlay(int32_t overlayId) override;
	virtual WVR_OverlayError SetOverlayTextureId(int32_t overlayId, const WVR_OverlayTexture_t *texture) override;
	virtual WVR_OverlayError SetOverlayFixedPosition(int32_t overlayId, const WVR_OverlayPosition_t *position) override;
	virtual WVR_OverlayError ShowOverlay(int32_t overlayId) override;
	virtual WVR_OverlayError HideOverlay(int32_t overlayId) override;
	virtual bool IsOverlayValid(int32_t overlayId) override;
	virtual bool IsOverlayShow(int32_t overlayId) override;

	/* WVR internal API */
	virtual bool IsATWActive() override;
	virtual void SetATWActive(bool isActive, void *anativeWindow = nullptr) override;
	virtual void PauseATW() override;  // New Api to replace SetATWActive(false)
	virtual void ResumeATW() override; // New Api to replace SetATWActive(true)
	virtual bool SetColorGamut(ColorGamut gamut) override;
	virtual WVR_Result SetAMCMode(WVR_AMCMode mode) override;
	virtual WVR_AMCMode GetAMCMode() override;
	virtual WVR_AMCStatus GetAMCStatus() override;
	virtual void SetRenderThreadId(int tid) override;
	virtual void SetAppEditorVersion(AppEditorVersion v) override;
	virtual WVR_TextureQueueHandle_t StoreRenderTextures(uint32_t texturesIDs[], int size, bool eEye, WVR_TextureTarget target) override;
	virtual void GetStencilMesh(WVR_Eye eye, uint32_t* vertexCount, uint32_t* triangleCount, uint32_t floatArrayCount, float* vertexData, uint32_t intArrayCount, int* indexData) override;
	virtual void SetFocusedController(WVR_DeviceType focusedController) override;
	virtual WVR_DeviceType GetFocusedController() override;

	/* WVR internal API - Resource Wrapper */
	virtual std::string GetStringBySystemLanguage(std::string str) override;
	virtual std::string GetStringByLanguage(std::string str, std::string lang, std::string country) override;
	virtual std::string GetSystemLanguage() override;
	virtual std::string GetSystemCountry() override;

	/* WVR internal API - OEM Config*/
	virtual std::string GetOEMConfigRawData(std::string key) override;

	/* WVR internal API - Controller Loader*/
	virtual std::string DeployRenderModelAssets(int deviceIndex, std::string renderModelName) override;
	virtual std::string GetRootRelativePath() override;

	/* WVR internal API - Permission Manager */
	virtual bool RequestPermissions(std::vector<std::string> permissions) override;
	virtual bool RequestUsbPermission() override;
	virtual bool IsPermissionGranted(std::string permission) override;
	virtual bool ShowDialogOnVRScene() override;

	/* WVR internal API - Camera Utility*/
	virtual bool CamUtil_GetFrameBufferWithPoseState(uint8_t *pframebuffer, uint32_t frameBufferSize, WVR_PoseOriginModel origin, uint32_t predictInMs, WVR_PoseState_t *poseState) override;
	virtual void CamUtil_ReleaseCameraTexture() override;
	virtual bool CamUtil_DrawTextureWithBuffer(uint32_t textureid, WVR_CameraImageFormat imgFormat, uint8_t *frameBuffer, uint32_t size, uint32_t width, uint32_t height, bool enableCropping, bool clearClampRegion, bool noErrorContext) override;

	/* WVR internal API - Controller model shared from runtime */
	virtual WVR_Result GetCurrentControllerModel(WVR_DeviceType ctrlerType, WVR_CtrlerModel_t **ctrlerModel, bool isOneBone = true) override;
	virtual void ReleaseControllerModel(WVR_CtrlerModel_t **ctrlerModel) override;

	virtual WVR_Result GetCtrlerModelAnimNodeData(WVR_DeviceType ctrlerType, WVR_CtrlerModelAnimData_t **ctrlModelAnimData) override;
	virtual void ReleaseCtrlerModelAnimNodeData(WVR_CtrlerModelAnimData_t **ctrlModelAnimData) override;

	/* WVR internal API - Multi-Layer */
	virtual WVR_SubmitError SubmitCompositionLayers(const WVR_LayerSetParams_t* param) override;
	virtual uint32_t GetMaxCompositionLayerCount() override;
	virtual bool SetChecker(bool enable) override;

private:
	void * mHandle;
	bool JavaENVCheck();
	void JavaENVFinish();
	bool mJavaAttached;
#endif
};
