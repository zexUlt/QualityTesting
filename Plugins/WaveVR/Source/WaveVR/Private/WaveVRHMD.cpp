// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRHMD.h"

// Engine
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "RendererPrivate.h"
#include "ScenePrivate.h"
#include "Slate/SceneViewport.h"
#include "PostProcess/PostProcessHMD.h"
#include "BatteryStatusEvent.h"
#include "CtrlrSwipeEvent.h"
#include "IpdUpdateEvent.h"
#include "RequestResultObject.h"
#include "PipelineStateCache.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "XRThreadUtils.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

// WaveVR
#include "Platforms/WaveVRLogWrapper.h"
#include "commit_info.h"
#include "WaveVREventCommon.h"
#include "WaveVRSplash.h"
#include "WaveVRRender.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/Windows/WaveVRPlatformWindows.h"
#include "Platforms/Android/WaveVRPlatformAndroid.h"
#include "Platforms/Editor/WaveVRDirectPreview.h"
#include "IWaveVRPlugin.h"
#include "PoseManagerImp.h"
#include "WaveVRUtils.h"
#include "WaveVRBlueprintFunctionLibrary.h"
#include "WaveVRMultiLayer.h"
#include "Hand/WaveVRHandGestureComponent.h"
#include "EngineUtils.h"

using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(WVRHMD, Display, All);

#if PLATFORM_ANDROID
#include "Android/AndroidWindow.h"
#include "Android/AndroidEGL.h"
#endif

#ifdef DP_DEBUG
static int x1 = 0;
static int x2 = 0;
#endif

#if (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG) && !WITH_EDITOR
#define WVR_SCOPED_NAMED_EVENT(name, color) SCOPED_NAMED_EVENT(name, color)
#else
#define WVR_SCOPED_NAMED_EVENT(name, color)
#endif


//---------------------------------------------------
// WaveVR Plugin Implementation
//---------------------------------------------------

class FWaveVRPlugin : public IWaveVRPlugin
{
	/** IHeadMountedDisplayModule implementation */
	virtual TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > CreateTrackingSystem() override;

	FString GetModuleKeyName() const {
		LOG_FUNC();
		return FString(TEXT("WaveVR"));
	}

public:
	FWaveVRPlugin() {
		LOG_FUNC();
#if PLATFORM_ANDROID
		LOGI(WVRHMD, "Commit Info : %s", WAVEVR_COMMIT_INFO);
#else // PLATFORM_WINDOWS || WITH_EDITOR
		FString str(WAVEVR_COMMIT_INFO);
		LOGI(WVRHMD, "Commit Info : %s", *str);
#endif
	}

	//TODO: Test whether it works or not if been removed
	virtual bool IsHMDConnected() override
	{
		LOG_FUNC();
		return true;
	}

	// This code should be here.  The FWaveVRPlugin must be the one who decide the platform, not decide in the wrapper.
	FWaveVRAPIWrapper * CreateWaveVRInstance()
	{
		LOGI(WVRHMD, "CreateWaveVRInstance()");
		FWaveVRAPIWrapper * instance = nullptr;
#if PLATFORM_ANDROID
		instance = new FWaveVRPlatformAndroid();
#elif WITH_EDITOR
		instance = FWaveVRHMD::DirectPreview = new WaveVRDirectPreview();
#elif PLATFORM_WINDOWS
		instance = new FWaveVRPlatformWindows();
#endif
		if (instance == nullptr)
			instance = new FWaveVRAPIWrapper();
		FWaveVRAPIWrapper::SetInstance(instance);

		return instance;
	}

	virtual void StartupModule() override
	{
		LOGI(WVRHMD, "StartupModule()+");

		IHeadMountedDisplayModule::StartupModule();
		// If use DirectPreview

		CreateWaveVRInstance();
		auto PlatFormContext = WVR();
		if (PlatFormContext != nullptr) {
			PlatFormContext->LoadLibraries();
		}

		// TODO: Disable RHI thread forcibly. Remove if RHI thread progress is ready.
		auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.OpenGL.AllowRHIThread"));
		if (CVar)
		{
			CVar->Set(false);
			LOGI(WVRHMD, "Set r.OpenGL.AllowRHIThread as %d", CVar->GetBool());
		}

		LOGI(WVRHMD, "StartupModule()-");
	}

	virtual void ShutdownModule() override
	{
		LOGI(WVRHMD, "ShutdownModule()");
		IHeadMountedDisplayModule::ShutdownModule();
		auto PlatFormContext = WVR();
		if (PlatFormContext != nullptr) {
			PlatFormContext->UnLoadLibraries();
		}
	}

	virtual bool IsStandaloneStereoOnlyDevice() override
	{
#if PLATFORM_ANDROID
		return true;
#else
		return false;
#endif
	}
};

IMPLEMENT_MODULE( FWaveVRPlugin, WaveVR )

TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > FWaveVRPlugin::CreateTrackingSystem()
{
	TSharedRef< FWaveVRHMD, ESPMode::ThreadSafe > WaveVRHMD = FSceneViewExtensions::NewExtension<FWaveVRHMD>();
	FWaveVRHMD::SetARSystem(WaveVRHMD);

	return WaveVRHMD;
}

//---------------------------------------------------
// WaveVR IHeadMountedDisplay Implementation
//---------------------------------------------------

FWaveVRHMD * FWaveVRHMD::Instance = nullptr;

WaveVRDirectPreview * FWaveVRHMD::DirectPreview = nullptr;

TSharedPtr<IARSystemSupport, ESPMode::ThreadSafe> FWaveVRHMD::ArSystem = nullptr;

bool FWaveVRHMD::IsHMDEnabled() const
{
	LOG_FUNC();
	// This function will not be used in game, but editor.  If need preview, return true.
	return bHmdEnabled;
}

void FWaveVRHMD::EnableHMD(bool enable)
{
	LOG_FUNC();
	// Only the console command will call this function.  Therefore the bHmdEnabled is initially true.
	bHmdEnabled = enable;

	if (!bHmdEnabled)
	{
		EnableStereo(false);
	}
}

bool FWaveVRHMD::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
	LOG_FUNC();

	MonitorDesc.MonitorName = "";
	MonitorDesc.MonitorId = 0;
	MonitorDesc.DesktopX = 0;
	MonitorDesc.DesktopY = 0;
	MonitorDesc.ResolutionX = 0;
	MonitorDesc.ResolutionY = 0;

#if PLATFORM_ANDROID
	FPlatformRect Rect = FAndroidWindow::GetScreenRect();
	MonitorDesc.ResolutionX = Rect.Right - Rect.Left;
	MonitorDesc.ResolutionY = Rect.Bottom - Rect.Top;
	return true;
#elif PLATFORM_WINDOWS && !WITH_EDITOR
	MonitorDesc.ResolutionX = 1024;  // TODO Correct it.  This is a test code.
	MonitorDesc.ResolutionY = 1024;  // TODO Correct it.  This is a test code.
	return true;
#endif
	return false;
}

//TODO: re-check
void FWaveVRHMD::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
	LOG_FUNC();
	//Assigned zero means we use projection matrix.
	OutHFOVInDegrees = 0.0f;
	OutVFOVInDegrees = 0.0f;
}

bool FWaveVRHMD::DoesSupportPositionalTracking() const
{
	LOG_FUNC();
	return PoseMngr->GetSupportedNumOfDoF(WVR_DeviceType_HMD) == EWVR_DOF::DOF_6;
}

bool FWaveVRHMD::HasValidTrackingPosition()
{
	LOG_FUNC();
	//Support 6 Dof do not means set 6 Dof now. ref: PoseManagerImp::SetNumOfDoF
	return DoesSupportPositionalTracking() && PoseMngr->IsDevicePoseValid(WVR_DeviceType_HMD);
}

struct Wave_TrackedDevice {
	WVR_DeviceType DeviceType;
	EXRTrackedDeviceType EXRType;
};

Wave_TrackedDevice TrackedObjects[WVR_DEVICE_COUNT_LEVEL_1] = {
	{WVR_DeviceType_HMD              /*1*/, EXRTrackedDeviceType::HeadMountedDisplay},
	{WVR_DeviceType_Controller_Right /*2*/, EXRTrackedDeviceType::Controller},
	{WVR_DeviceType_Controller_Left  /*3*/, EXRTrackedDeviceType::Controller},
};

bool FWaveVRHMD::EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type /*EXRTrackedDeviceType::Any*/)
{
	LOG_FUNC();
	for (auto TrackedDevice: TrackedObjects){
		if (TrackedDevice.EXRType == Type || Type == EXRTrackedDeviceType::Any)
			OutDevices.Add(static_cast<int32>(TrackedDevice.DeviceType));
	}
	return true;
}



// IXRTrackingSystem: This method must be callable both on the render thread and the game thread.
bool FWaveVRHMD::GetCurrentPose(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition) {
	LOG_FUNC();
	WVR_SCOPED_NAMED_EVENT(GetCurrentPose, FColor::Orange);

	//if (PLATFORM_WINDOWS) // TODO Correct it.  This is a test code.
	//{
	//	OutOrientation = FQuat::Identity;
	//	OutPosition = FVector::ZeroVector;
	//	return true;
	//}

#if WITH_EDITOR
	if (GIsEditor && !WaveVRDirectPreview::IsDirectPreview()) return false;
#endif

	WVR_DeviceType ID = static_cast<WVR_DeviceType>(DeviceId);
	if (DeviceId == IXRTrackingSystem::HMDDeviceId) { ID = WVR_DeviceType_HMD; } // 0 is the default (first) HMD which UE assigned.

	if (IsInGameThread()) {
		OutOrientation = FrameData->orientation;
		OutPosition = FrameData->position;
		return true;
	} else if (IsInRenderingThread()) {
		bool ret = false;
		if (FrameDataRT->bSupportLateUpdate && FrameDataRT->bNeedLateUpdateInRT) {
			FrameDataRT->bNeedLateUpdateInRT = false;
			PoseMngr->LateUpdate_RenderThread(FrameDataRT);
		}
		OutOrientation = FrameDataRT->orientation;
		OutPosition = FrameDataRT->position;
		return true;
	}

	return false;
}

bool FWaveVRHMD::GetRelativeEyePose(int32 DeviceId, EStereoscopicPass Eye, FQuat& OutOrientation, FVector& OutPosition)
{
	LOG_FUNC();
	OutOrientation = FQuat::Identity;
	OutPosition = FVector::ZeroVector;
	//if (PLATFORM_WINDOWS) // TODO Correct it.  This is a test code.
	//	return true;

	if (DeviceId == IXRTrackingSystem::HMDDeviceId && (Eye == eSSP_LEFT_EYE || Eye == eSSP_RIGHT_EYE)) {
		WVR_Eye hmdEye = (Eye == eSSP_LEFT_EYE) ? WVR_Eye_Left : WVR_Eye_Right;
		WVR_Matrix4f_t headFromEye = WVR()->GetTransformFromEyeToHead(hmdEye/*, WVR_NumDoF_6DoF*/);  // TODO need input DoF

		OutPosition = FVector(-headFromEye.m[2][3], headFromEye.m[0][3], headFromEye.m[1][3]) * GetWorldToMetersScale();

		FQuat Orientation(wvr::utils::ToFMatrix(headFromEye));
		OutOrientation.X = -Orientation.Z;
		OutOrientation.Y = Orientation.X;
		OutOrientation.Z = Orientation.Y;
		OutOrientation.W = -Orientation.W;
		//LOGI(WVRHMD, "GetRelativeEyePose(%d, %d, Quat(%f, %f, %f, %f), Pos(%f, %f, %f))", DeviceId, Eye, OutOrientation.X, OutOrientation.Y, OutOrientation.Z, OutOrientation.W, OutPosition.X, OutPosition.Y, OutPosition.Z);
		return true;
	} else {
		return false;
	}
	//return FXRTrackingSystemBase::GetRelativeEyePose(DeviceId, Eye, OutOrientation, OutPosition);
}

void FWaveVRHMD::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
	LOG_FUNC();
	//Do not allow user to set IPD
	LOGI(WVRHMD, "SetInterpupillaryDistance %.5f, but nothing happened.", NewInterpupillaryDistance);
}

float FWaveVRHMD::GetInterpupillaryDistance() const
{
	LOG_FUNC();
	return CurrentIPD;
}

bool FWaveVRHMD::GetHMDDistortionEnabled(EShadingPath ShadingPath) const {
	LOG_FUNC();
	return bUseUnrealDistortion;
}

void FWaveVRHMD::SetPixelDensity(const float NewDensity) {
	LOG_FUNC();

	mRender.SetPixelDensity(NewDensity);
}

float FWaveVRHMD::GetPixelDenity() const {
	LOG_FUNC();
	return mRender.GetPixelDensity();
}

// This API want the default size, not the scaled size.
FIntPoint FWaveVRHMD::GetIdealRenderTargetSize() const {
	LOG_FUNC();
	FIntPoint IdealRenderTargetSize;
	uint32 width = mRender.GetSingleEyePixelWidth(), height = mRender.GetSingleEyePixelHeight();
	IdealRenderTargetSize.X = mRender.GetMultiView() ? width : width * 2;
	IdealRenderTargetSize.Y = height;
	return IdealRenderTargetSize;
}

bool FWaveVRHMD::IsChromaAbCorrectionEnabled() const
{
	LOG_FUNC();
	return true;
}

void FWaveVRHMD::UpdateScreenSettings(const class FViewport* InViewport) {
	LOG_FUNC();
	// Cant do it because const.
	//InViewport->GetViewportFrame()->ResizeFrame(mRender.GetSingleEyePixelWidth(), mRender.GetSingleEyePixelHeight(), EWindowMode::WindowedFullscreen);

	// Can this line stop rendering scene?
	//InViewport->SetGameRenderingEnabled(false);
}


bool FWaveVRHMD::IsHeadTrackingAllowed() const
{
	LOG_FUNC();
	return FHeadMountedDisplayBase::IsHeadTrackingAllowed();
}

void FWaveVRHMD::OnBeginPlay(FWorldContext& InWorldContext)
{
	LOG_FUNC();
	FirstGameFrame = true;

	WaveVRDirectPreviewSettings::SettingsInfo();

	if (WaveVRDirectPreview::IsVRPreview() && !bSIM_Available) //OnBeginPlay will continuously execute twice
	{
		if (DirectPreview != nullptr && DirectPreview->HookVRPreview()) {
			LOGD(WVRHMD, "DirectPreview device is hooked.");
			bSIM_Available = true;
			ResetProjectionMats();

			uint32 width = 1024, height = 1024;
			WVR()->GetRenderTargetSize(&width, &height);
			mRender.SetSingleEyePixelSize(width, height);
			mRender.SetTextureFormat(PF_R8G8B8A8);
			mRender.Apply();

			SetDeviceSupportedMaxFPS();

		} else {
			LOGW(WVRHMD, "DirectPreview hook failed!");
		}
	}
}

void FWaveVRHMD::OnEndPlay(FWorldContext& InWorldContext)
{
	LOG_FUNC();
	EnableStereo(false);
	if (WaveVRDirectPreview::IsVRPreview())
	{
		// Close it if available
		if (bSIM_Available) {
			WVR()->Quit();
			bSIM_Available = false;
		}
	}
}

void FWaveVRHMD::NextFrameData() {
	// swap data
	OldFrameData = FrameData;
	FrameData = FFrameData::NewInstance();

	// If position is invalidate, we need the old pose.  Use latest pose in RT.
	FFrameData::Copy(OldFrameDataRT, FrameData);

	FrameData->bSupportLateUpdate = FrameData->bNeedLateUpdateInRT = lateUpdateConfig.bEnabled;
	FrameData->bDoUpdateInGT = lateUpdateConfig.bDoUpdateInGT;
	FrameData->predictTimeInGT = lateUpdateConfig.predictTimeInGT;
	FrameData->frameNumber = GFrameNumber;
	FrameData->meterToWorldUnit = GetWorldToMetersScale();  // For example, 1 meter multiply 100 to convert to world units.
	FrameData->Origin = PoseMngr->GetTrackingOriginModelInternal();
}

bool FWaveVRHMD::OnStartGameFrame(FWorldContext& WorldContext)
{
	LOG_FUNC();
	WVR_SCOPED_NAMED_EVENT(OnStartGameFrame, FColor::Purple);

#if WITH_EDITOR
	if (WorldContext.WorldType == EWorldType::PIE && (WaveVRDirectPreview::IsVRPreview() && IsDirectPreview())) {
#ifdef DP_DEBUG
		LOGW(WVRHMD, "OnStartGameFrame %d", x1++);
		LOGW(WVRHMD, "FlushRenderingCommands");
#endif
		FlushRenderingCommands();
#else
	if (mRender.IsInitialized()) {
#endif
		NextFrameData();
		PoseMngr->UpdatePoses(FrameData);
	}

	if (!AppliedAdaptiveQualityProjectSettings && IsRenderInitialized()) {
		AppliedAdaptiveQualityProjectSettings = true;
		LOGD(WVRHMD, "Activate cached AdaptiveQuality : enabled(%u) StrategyFlags(%u) (consider project settings and override by developers)", bAdaptiveQuality, AdaptiveQualityStrategyFlags);
		WVR()->EnableAdaptiveQuality(bAdaptiveQuality, AdaptiveQualityStrategyFlags);
	}

	RefreshTrackingToWorldTransform(WorldContext);
	pollEvent();
	CheckSystemFocus();
	if (pSimulator)
		pSimulator->TickData();
	if (pHandPose)
		pHandPose->TickHandData();
	if (pEyeManager)
		pEyeManager->TickEyeData();
	if (pTracker)
		pTracker->TickTrackerData();
	if (pLipExp)
		pLipExp->TickLipExpData();
	if (pEyeExp)
		pEyeExp->TickEyeExpData();

	if (GNearClippingPlane != NearClippingPlane) {
		bNeedResetProjectionMatrix = true;
		NearClippingPlane = GNearClippingPlane;
	}
	if (bNeedResetProjectionMatrix)
		ResetProjectionMats();

	SceneStatusInfo(FirstGameFrame);
	FirstGameFrame = false;

	CachedTrackingToWorld = ComputeTrackingToWorldTransform(WorldContext);

	mWaveAR->UpdateData();

#if WITH_EDITOR
	WVR()->Tick();
#endif

#if (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG)
	if (SimulatedLoadingGameThread > 0 && SimulatedLoadingGameThread <= 100000)
	{
		WVR_SCOPED_NAMED_EVENT(GTSleep, FColor::Purple);
		FPlatformProcess::Sleep(SimulatedLoadingGameThread / 1000000.0f);
	}
#endif

	return true;
}

bool FWaveVRHMD::OnEndGameFrame(FWorldContext& WorldContext)
{
	WVR_SCOPED_NAMED_EVENT(OnEndGameFrame, FColor::Purple);

	LOG_FUNC();
	//LOGD(WVRHMD, "OnEndGameFrame");

	FrameDataPtr FrameDataCopy = FFrameData::NewInstance();
	FFrameData::Copy(FrameData, FrameDataCopy);

	// Send a copy of FrameData to render thread.
	ExecuteOnRenderThread_DoNotWait([this, FrameDataCopy](FRHICommandListImmediate& RHICmdList)
	{
		WVR_SCOPED_NAMED_EVENT(CopyFrameDataToRT, FColor::Purple);
		OldFrameDataRT = FrameDataRT;
		FrameDataRT = FrameDataCopy;
	}
	);

	return false;
}

void FWaveVRHMD::OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
	WVR_SCOPED_NAMED_EVENT(OnBeginRenderingRT, FColor::Purple);

	LOG_FUNC();

#if WITH_EDITOR
	if (DirectPreview != nullptr && GetInstance()->IsDirectPreview()) {
#ifdef DP_DEBUG
		LOGW(WVRHMD, "OnBeginRendering_RenderThread %d", x2++);
#endif
		FRHITexture2D* TexRef2D = ViewFamily.RenderTarget->GetRenderTargetTexture()->GetTexture2D();
		uint32 LolStride = 0;
		char* TextureDataPtr = (char*)RHICmdList.LockTexture2D(TexRef2D, 0, EResourceLockMode::RLM_ReadOnly, LolStride, false);
		DirectPreview->sendRTTexture(RHICmdList, TexRef2D->GetNativeResource());
		RHICmdList.UnlockTexture2D(TexRef2D, 0, false);
	}
#endif

	// TODO We should let simulator run the mRender to test windows path.
	if (GIsEditor) return;

#if (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG)
	if (SimulatedLoadingRenderThread > 0 && SimulatedLoadingRenderThread <= 100000)
	{
		WVR_SCOPED_NAMED_EVENT(RTSleep, FColor::Purple);
		FPlatformProcess::Sleep(SimulatedLoadingRenderThread / 1000000.0f);
	}
#endif

	mRender.OnBeginRendering_RenderThread(RHICmdList, ViewFamily);
}

void FWaveVRHMD::OnBeginRendering_GameThread()
{
	WVR_SCOPED_NAMED_EVENT(OnBeginRenderingGT, FColor::Purple);
	LOG_FUNC();
}

bool FWaveVRHMD::DoesSupportLateUpdate() const {
	LOG_FUNC();
	//LOGD(WVRHMD, "LateUpdate=%d, DoUpdateInGT=%d, predict=%f",
	//	lateUpdateConfig.bEnabled, lateUpdateConfig.bDoUpdateInGT, lateUpdateConfig.predictTimeInGT);
	return lateUpdateConfig.bEnabled;
}

void FWaveVRHMD::EnableLateUpdate(bool enable, bool doUpdateInGT, float predictTimeInGT) {
	predictTimeInGT = FMath::Clamp<float>(predictTimeInGT, 0, 100.0f);
	LOGD(WVRHMD, "EnableLateUpdate(%d, %d, %f)", (int) enable, (int)doUpdateInGT, predictTimeInGT);
	lateUpdateConfig.bEnabled = enable;
	lateUpdateConfig.bDoUpdateInGT = doUpdateInGT;
	lateUpdateConfig.predictTimeInGT = predictTimeInGT;

	// Trigger RenderMask to disable or enable
	UIpdUpdateEvent::onIpdUpdateNative.Broadcast();
}

void FWaveVRHMD::OnLateUpdateApplied_RenderThread(FRHICommandListImmediate& RHICmdList, const FTransform& NewRelativeTransform)
{
	WVR_SCOPED_NAMED_EVENT(OnLateUpdateApplied, FColor::Purple);
	LOG_FUNC();
	FDefaultStereoLayers::UpdateHmdTransform(NewRelativeTransform);
}

void FWaveVRHMD::SetTrackingOrigin(EHMDTrackingOrigin::Type InOrigin)
{
	LOG_FUNC();
	PoseMngr->SetTrackingOriginPoses(InOrigin);
}

EHMDTrackingOrigin::Type FWaveVRHMD::GetTrackingOrigin() const
{
	LOG_FUNC();
	return PoseMngr->GetTrackingOriginPoses();
}

void FWaveVRHMD::pollEvent() {
	LOG_FUNC();
	bool ret = false;
	do
	{
		WVR_Event_t vrevent;
		ret = WVR()->PollEventQueue(&vrevent);
		if (ret)
		{
			processVREvent(vrevent);
		}
		else
		{
			UpdateConnectionsWhenNoEvent();
		}
	} while (ret);
}

void FWaveVRHMD::processVREvent(WVR_Event_t vrEvent)
{
	LOG_FUNC();
	switch (vrEvent.common.type)
	{
	case WVR_EventType_DeviceConnected:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_DeviceConnected %d", (uint8)vrEvent.device.deviceType);
		toUpdateConnectionsWhenNoEvent = true;
		break;
	case WVR_EventType_DeviceDisconnected:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_DeviceDisconnected %d", (uint8)vrEvent.device.deviceType);
		toUpdateConnectionsWhenNoEvent = true;
		break;
	case WVR_EventType_InputDevMappingChanged:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_InputDevMappingChanged %d", (uint8)vrEvent.device.deviceType);
		UpdateInputMappingTable(vrEvent.device.deviceType);
		break;
	case WVR_EventType_BatteryStatusUpdate:
		LOGD(WVRHMD, "processVREvent() WVR_EventType: WVR_EventType battery status updated");
		UBatteryStatusEvent::onBatteryStatusUpdateNative.Broadcast();
		break;
	case WVR_EventType_IpdChanged:
		WVR_RenderProps props;
		if (WVR()->GetRenderProps(&props)) {
			float ipd = props.ipdMeter;
			LOGI(WVRHMD, "processVREvent() WVR_EventType: WVR_EventType IPD updated (%f)", ipd);
			CurrentIPD = ipd;
		}
		else {
			LOGD(WVRHMD, "Get render properties error! Not success!");
		}
		ResetProjectionMats();
		UIpdUpdateEvent::onIpdUpdateNative.Broadcast();
		break;
	case WVR_EventType_LeftToRightSwipe:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_LeftToRightSwipe, device %d", (uint8)vrEvent.device.deviceType);
		UCtrlrSwipeEvent::onCtrlrSwipeLtoRUpdateNative.Broadcast();
		break;
	case WVR_EventType_RightToLeftSwipe:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_RightToLeftSwipe, device %d", (uint8)vrEvent.device.deviceType);
		UCtrlrSwipeEvent::onCtrlrSwipeRtoLUpdateNative.Broadcast();
		break;
	case WVR_EventType_UpToDownSwipe:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_UpToDownSwipe, device %d", (uint8)vrEvent.device.deviceType);
		UCtrlrSwipeEvent::onCtrlrSwipeUtoDUpdateNative.Broadcast();
		break;
	case WVR_EventType_DownToUpSwipe:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_DownToUpSwipe, device %d", (uint8)vrEvent.device.deviceType);
		UCtrlrSwipeEvent::onCtrlrSwipeDtoUUpdateNative.Broadcast();
		break;
	case WVR_EventType_RecenterSuccess:
		LOGD(WVRHMD, "processVREvent() WVR_EventType: WVR_EventType HMD Recenter");
		break;
	case WVR_EventType_DeviceRoleChanged:
		CheckLeftHandedMode();
		LOGI(WVRHMD, "processVREvent() WVR_EventType_DeviceRoleChanged() bIsLeftHanded: %d", (uint8)bIsLeftHanded);
		toUpdateConnectionsWhenNoEvent = true;
		break;
	case WVR_EventType_TrackingModeChanged:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackingModeChanged");
		UWaveVREventCommon::OnTrackingModeChangeNative.Broadcast();
		break;
	case WVR_EventType_ControllerPoseModeChanged:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_ControllerPoseModeChanged");
		UpdatePoseModeAndBroadcast(vrEvent.device.deviceType);
		break;
	case WVR_EventType_ControllerPoseModeOffsetReady:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_ControllerPoseModeOffsetReady");
		UpdatePoseModeData();
		break;
	// ------------------- Button State begins ----------------
	case WVR_EventType_ButtonPressed:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_ButtonPressed");
		UpdateButtonPressStates(vrEvent.device.deviceType, vrEvent.input.inputId, true);
		break;
	case WVR_EventType_ButtonUnpressed:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_ButtonUnpressed");
		UpdateButtonPressStates(vrEvent.device.deviceType, vrEvent.input.inputId, false);
		break;
	case WVR_EventType_TouchTapped:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TouchTapped");
		UpdateButtonTouchStates(vrEvent.device.deviceType, vrEvent.input.inputId, true);
		break;
	case WVR_EventType_TouchUntapped:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TouchUntapped");
		UpdateButtonTouchStates(vrEvent.device.deviceType, vrEvent.input.inputId, false);
		break;
	// ------------------- Button State ends ----------------

	case WVR_EventType_DeviceTableStaticLocked:
	{
		WVR_DeviceType dt = vrEvent.device.deviceType;
		LOGD(WVRHMD, "processVREvent() WVR_EventType_DeviceTableStaticLocked %d", (uint8)dt);
		if (s_TableStatic.Contains(dt))
		{
			s_TableStatic[dt] = FWaveVRAPIWrapper::GetInstance()->IsDeviceTableStatic(dt);
			LOGD(WVRHMD, "processVREvent() WVR_EventType_DeviceTableStaticLocked device %d static %d", (uint8)dt, (uint8)s_TableStatic[dt]);
		}
		break;
	}
	case WVR_EventType_DeviceTableStaticUnlocked:
	{
		WVR_DeviceType dt = vrEvent.device.deviceType;
		LOGD(WVRHMD, "processVREvent() WVR_EventType_DeviceTableStaticUnlocked %d", (uint8)dt);
		if (s_TableStatic.Contains(dt))
		{
			s_TableStatic[dt] = FWaveVRAPIWrapper::GetInstance()->IsDeviceTableStatic(dt);
			LOGD(WVRHMD, "processVREvent() WVR_EventType_DeviceTableStaticUnlocked device %d static %d", (uint8)dt, (uint8)s_TableStatic[dt]);
		}
		break;
	}

	case WVR_EventType_InteractionModeChanged:
		UpdateInteractionMode();
		LOGD(WVRHMD, "processVREvent() WVR_EventType_InteractionModeChanged, m_InteractionMode = %d", (uint8)m_InteractionMode);
		break;
	case WVR_EventType_GazeTriggerTypeChanged:
		UpdateGazeType();
		LOGD(WVRHMD, "processVREvent() WVR_EventType_GazeTriggerTypeChanged, m_GazeType = %d", (uint8)m_GazeType);
		break;

	// ----------------- Hand begins -----------------
	case WVR_EventType_Hand_EnhanceStable:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_Hand_EnhanceStable");
		if (pHandPose)
			pHandPose->CheckPoseFusion();
		break;
	// ----------------- Hand ends -----------------

	// ----------------- Tracker begins -----------------
	case WVR_EventType_TrackerConnected:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerConnected, tracker: %d", (uint8)vrEvent.tracker.trackerId);
		if (pTracker)
			pTracker->OnTrackerConnection((uint8)vrEvent.tracker.trackerId, true);
		break;
	case WVR_EventType_TrackerDisconnected:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerDisconnected, tracker: %d", (uint8)vrEvent.tracker.trackerId);
		if (pTracker)
			pTracker->OnTrackerConnection((uint8)vrEvent.tracker.trackerId, false);
		break;
	case WVR_EventType_TrackerBatteryLevelUpdate:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerBatteryLevelUpdate");
		if (pTracker)
			pTracker->OnTrackerBatteryLevelUpdate((uint8)vrEvent.tracker.trackerId);
		break;
	case WVR_EventType_TrackerButtonPressed:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerButtonPressed, tracker: %d, button: %d", (uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId);
		if (pTracker)
			pTracker->OnButtonPress((uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId, true);
		break;
	case WVR_EventType_TrackerButtonUnpressed:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerButtonUnpressed, tracker: %d, button: %d", (uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId);
		if (pTracker)
			pTracker->OnButtonPress((uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId, false);
		break;
	case WVR_EventType_TrackerTouchTapped:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerTouchTapped, tracker: %d, button: %d", (uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId);
		if (pTracker)
			pTracker->OnButtonTouch((uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId, true);
		break;
	case WVR_EventType_TrackerTouchUntapped:
		LOGD(WVRHMD, "processVREvent() WVR_EventType_TrackerTouchUntapped, tracker: %d, button: %d", (uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId);
		if (pTracker)
			pTracker->OnButtonTouch((uint8)vrEvent.tracker.trackerId, (uint8)vrEvent.trackerInput.inputId, false);
		break;
	// ----------------- Tracker ends -----------------
	default:
		LOGD(WVRHMD, "processVREvent() %d", (uint8)vrEvent.common.type);
		break;
	}
	UWaveVREventCommon::OnAllEventNative.Broadcast((int32)(vrEvent.common.type));
}

//Call from UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(float Yaw, EOrientPositionSelector::Type Options)
void FWaveVRHMD::ResetOrientationAndPosition(float yaw) {
	LOG_FUNC();
	WVR()->InAppRecenter(WVR_RecenterType_YawAndPosition);
}

//Call from UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(float Yaw, EOrientPositionSelector::Type Options)
void FWaveVRHMD::ResetOrientation(float Yaw) {
	LOG_FUNC();
	WVR()->InAppRecenter(WVR_RecenterType_YawOnly);
}

//Call from UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(float Yaw, EOrientPositionSelector::Type Options)
void FWaveVRHMD::ResetPosition() {
	LOG_FUNC();
	WVR()->InAppRecenter(WVR_RecenterType_Position);
}

void FWaveVRHMD::SetBaseRotation(const FRotator& BaseRot) {
	LOG_FUNC();
}

FRotator FWaveVRHMD::GetBaseRotation() const {
	LOG_FUNC();
	return FRotator::ZeroRotator;
}

void FWaveVRHMD::SetBaseOrientation(const FQuat& BaseOrient) {
	LOG_FUNC();
}

FQuat FWaveVRHMD::GetBaseOrientation() const {
	LOG_FUNC();
	return FQuat::Identity;
}

#pragma region
bool FWaveVRHMD::IsLeftHandedMode()
{
	return bIsLeftHanded;
}
void FWaveVRHMD::CheckLeftHandedMode()
{
	bool left_handed = WVR()->GetDefaultControllerRole() == WVR_DeviceType::WVR_DeviceType_Controller_Left ? true : false;
	if (bIsLeftHanded != left_handed)
	{
		bIsLeftHanded = left_handed;
		LOGD(WVRHMD, "WaveVR CheckLeftHandedMode() left-handed? %d", (uint8)bIsLeftHanded);
		UWaveVREventCommon::OnControllerRoleChangeNative.Broadcast();
	}
}
#pragma endregion Left-handed mode
#pragma region
bool FWaveVRHMD::GetButtonPressState(WVR_DeviceType type, WVR_InputId id)
{
	switch (type)
	{
	case WVR_DeviceType::WVR_DeviceType_HMD:
		return s_ButtonPressedHmd[(uint8)id];
	case WVR_DeviceType::WVR_DeviceType_Controller_Left:
		return s_ButtonPressedLeft[(uint8)id];
	case WVR_DeviceType::WVR_DeviceType_Controller_Right:
		return s_ButtonPressedRight[(uint8)id];
	default:
		break;
	}
	return false;
}
bool FWaveVRHMD::GetButtonTouchState(WVR_DeviceType type, WVR_InputId id)
{
	switch (type)
	{
	case WVR_DeviceType::WVR_DeviceType_HMD:
		return s_ButtonTouchedHmd[(uint8)id];
	case WVR_DeviceType::WVR_DeviceType_Controller_Left:
		return s_ButtonTouchedLeft[(uint8)id];
	case WVR_DeviceType::WVR_DeviceType_Controller_Right:
		return s_ButtonTouchedRight[(uint8)id];
	default:
		break;
	}
	return false;
}
void FWaveVRHMD::UpdateButtonStates()
{
	for (int i = 0; i < kButtonCount; i++)
	{
		s_ButtonPressedHmd[i] = WVR()->GetInputButtonState(WVR_DeviceType::WVR_DeviceType_HMD, static_cast<WVR_InputId>(i));
		LOGD(WVRHMD, "UpdateButtonStates() HMD button %d pressed? %d", i, (int)s_ButtonPressedHmd[i]);
		s_ButtonTouchedHmd[i] = WVR()->GetInputTouchState(WVR_DeviceType::WVR_DeviceType_HMD, static_cast<WVR_InputId>(i));
		LOGD(WVRHMD, "UpdateButtonStates() HMD button %d touched? %d", i, (int)s_ButtonTouchedHmd[i]);

		s_ButtonPressedLeft[i] = WVR()->GetInputButtonState(WVR_DeviceType::WVR_DeviceType_Controller_Left, static_cast<WVR_InputId>(i));
		LOGD(WVRHMD, "UpdateButtonStates() Left button %d pressed? %d", i, (int)s_ButtonPressedLeft[i]);
		s_ButtonTouchedLeft[i] = WVR()->GetInputTouchState(WVR_DeviceType::WVR_DeviceType_Controller_Left, static_cast<WVR_InputId>(i));
		LOGD(WVRHMD, "UpdateButtonStates() Left button %d touched? %d", i, (int)s_ButtonTouchedLeft[i]);

		s_ButtonPressedRight[i] = WVR()->GetInputButtonState(WVR_DeviceType::WVR_DeviceType_Controller_Right, static_cast<WVR_InputId>(i));
		LOGD(WVRHMD, "UpdateButtonStates() Right button %d pressed? %d", i, (int)s_ButtonPressedRight[i]);
		s_ButtonTouchedRight[i] = WVR()->GetInputTouchState(WVR_DeviceType::WVR_DeviceType_Controller_Right, static_cast<WVR_InputId>(i));
		LOGD(WVRHMD, "UpdateButtonStates() Right button %d touched? %d", i, (int)s_ButtonTouchedRight[i]);
	}
}
void FWaveVRHMD::ResetButtonStates(WVR_DeviceType type)
{
	switch (type)
	{
	case WVR_DeviceType::WVR_DeviceType_HMD:
	{
		for (int i = 0; i < kButtonCount; i++)
		{
			s_ButtonPressedHmd[i] = false;
			s_ButtonTouchedHmd[i] = false;
		}
	}
	break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Left:
	{
		for (int i = 0; i < kButtonCount; i++)
		{
			s_ButtonPressedLeft[i] = false;
			s_ButtonTouchedLeft[i] = false;
		}
	}
	break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Right:
	{
		for (int i = 0; i < kButtonCount; i++)
		{
			s_ButtonPressedRight[i] = false;
			s_ButtonTouchedRight[i] = false;
		}
	}
	break;
	default:
		break;
	}
}
void FWaveVRHMD::UpdateButtonPressStates(WVR_DeviceType type, WVR_InputId id, bool press)
{
	if (type == WVR_DeviceType::WVR_DeviceType_HMD)
	{
		s_ButtonPressedHmd[(uint8)id] = press;
		LOGD(WVRHMD, "UpdateButtonPressStates() HMD, button %d, press %d", (uint8)id, (uint8)press);
		UWaveVREventCommon::OnAllButtonPressNative_HMD.Broadcast((uint8)id, press);
	}
	if (type == WVR_DeviceType::WVR_DeviceType_Controller_Left)
	{
		s_ButtonPressedLeft[(uint8)id] = press;
		LOGD(WVRHMD, "UpdateButtonPressStates() Left, button %d, press %d", (uint8)id, (uint8)press);
		// UWaveVREventCommon::OnAllButtonPressNative_Left.Broadcast((uint8)id, press); // Ignore here. Called in WaveVRInput.
	}
	if (type == WVR_DeviceType::WVR_DeviceType_Controller_Right)
	{
		s_ButtonPressedRight[(uint8)id] = press;
		LOGD(WVRHMD, "UpdateButtonPressStates() Right, button %d, press %d", (uint8)id, (uint8)press);
		// UWaveVREventCommon::OnAllButtonPressNative_Right.Broadcast((uint8)id, press); // Ignore here. Called in WaveVRInput.
	}
}
void FWaveVRHMD::UpdateButtonTouchStates(WVR_DeviceType type, WVR_InputId id, bool touch)
{
	if (type == WVR_DeviceType::WVR_DeviceType_HMD)
	{
		s_ButtonTouchedHmd[(uint8)id] = touch;
		LOGD(WVRHMD, "UpdateButtonTouchStates() HMD, button %d, touch %d", (uint8)id, (uint8)touch);
		UWaveVREventCommon::OnAllButtonTouchNative_HMD.Broadcast((uint8)id, touch);
	}
	if (type == WVR_DeviceType::WVR_DeviceType_Controller_Left)
	{
		s_ButtonTouchedLeft[(uint8)id] = touch;
		LOGD(WVRHMD, "WaveVR UpdateButtonTouchStates() Left, button %d, touch %d", (uint8)id, (uint8)touch);
		// UWaveVREventCommon::OnAllButtonTouchNative_Left.Broadcast((uint8)id, touch); // Ignore here. Called in WaveVRInput.
	}
	if (type == WVR_DeviceType::WVR_DeviceType_Controller_Right)
	{
		s_ButtonTouchedRight[(uint8)id] = touch;
		LOGD(WVRHMD, "WaveVR UpdateButtonTouchStates() Right, button %d, touch %d", (uint8)id, (uint8)touch);
		// UWaveVREventCommon::OnAllButtonTouchNative_Right.Broadcast((uint8)id, touch); // Ignore here. Called in WaveVRInput.
	}
}
#pragma endregion Button
#pragma region
void FWaveVRHMD::SetInputRequest(WVR_DeviceType device, const WVR_InputAttribute_t* inputAttributes, uint32_t size) {
	if (device == WVR_DeviceType::WVR_DeviceType_Invalid)
		return;

	LOGD(WVRHMD, "SetInputRequest() %d", device);
	if (WVR()->SetInputRequest(device, inputAttributes, size))
	{
		UpdateInputMappingTable(device);
	}
}
bool FWaveVRHMD::IsButtonAvailable(WVR_DeviceType device, WVR_InputId button)
{
	if (device == WVR_DeviceType_HMD)
		return usableButtons_hmd[(uint8)button];
	if (device == WVR_DeviceType_Controller_Right)
		return usableButtons_right[(uint8)button];
	if (device == WVR_DeviceType_Controller_Left)
		return usableButtons_left[(uint8)button];

	return false;
}
void FWaveVRHMD::UpdateInputMappingTable(WVR_DeviceType device)
{
	switch (device)
	{
	case WVR_DeviceType::WVR_DeviceType_HMD:
		inputTableHMDSize = WVR()->GetInputMappingTable(device, inputTableHMD, (uint32_t)WVR_InputId_Max);
		LOGD(WVRHMD, "UpdateInputMappingTable() HMD size: %d", inputTableHMDSize);
		for (int _i = 0; _i < (int)inputTableHMDSize; _i++)
		{
			if (inputTableHMD[_i].source.capability != 0)
			{
				usableButtons_hmd[(uint8)inputTableHMD[_i].destination.id] = true;
				LOGD(WVRHMD, "UpdateInputMappingTable() HMD button %d (capability: %d) is mapping to input ID %d.",
					(uint8)inputTableHMD[_i].source.id, (uint8)inputTableHMD[_i].source.capability, (uint8)inputTableHMD[_i].destination.id);
			}
			else
			{
				LOGD(WVRHMD, "UpdateInputMappingTable() HMD source button %d has invalid capability.", (uint8)inputTableHMD[_i].source.id);
			}
		}
		mappingTableHash[WVR_DeviceType::WVR_DeviceType_HMD] = GFrameCounter;
		break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Right:
		inputTableRightSize = WVR()->GetInputMappingTable(device, inputTableRight, (uint32_t)WVR_InputId_Max);
		LOGD(WVRHMD, "UpdateInputMappingTable() Right size: %d", inputTableRightSize);
		for (int _i = 0; _i < (int)inputTableRightSize; _i++)
		{
			if (inputTableRight[_i].source.capability != 0)
			{
				usableButtons_right[(uint8)inputTableRight[_i].destination.id] = true;
				LOGD(WVRHMD, "UpdateInputMappingTable() Right button %d (capability: %d) is mapping to input ID %d.",
					(uint8)inputTableRight[_i].source.id, (uint8)inputTableRight[_i].source.capability, (uint8)inputTableRight[_i].destination.id);
			}
			else
			{
				LOGD(WVRHMD, "UpdateInputMappingTable() Right source button %d has invalid capability.", (uint8)inputTableRight[_i].source.id);
			}
		}
		mappingTableHash[WVR_DeviceType::WVR_DeviceType_Controller_Right] = GFrameCounter;
		break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Left:
		inputTableLeftSize = WVR()->GetInputMappingTable(device, inputTableLeft, (uint32_t)WVR_InputId_Max);
		LOGD(WVRHMD, "UpdateInputMappingTable() Left size: %d", inputTableLeftSize);
		for (int _i = 0; _i < (int)inputTableLeftSize; _i++)
		{
			if (inputTableLeft[_i].source.capability != 0)
			{
				usableButtons_left[(uint8)inputTableLeft[_i].destination.id] = true;
				LOGD(WVRHMD, "UpdateInputMappingTable() Left button %d (capability: %d) is mapping to input ID %d.",
					(uint8)inputTableLeft[_i].source.id, (uint8)inputTableLeft[_i].source.capability, (uint8)inputTableLeft[_i].destination.id);
			}
			else
			{
				LOGD(WVRHMD, "UpdateInputMappingTable() Left source button %d has invalid capability.", (uint8)inputTableLeft[_i].source.id);
			}
		}
		mappingTableHash[WVR_DeviceType::WVR_DeviceType_Controller_Left] = GFrameCounter;
		break;
	default:
		break;
	}
}
bool FWaveVRHMD::GetInputMappingPair(WVR_DeviceType device, WVR_InputId &destination)
{
	// Default true in editor mode, destination will be equivallent to source.
	bool result = false;
	int index = 0;

	switch (device)
	{
	case WVR_DeviceType::WVR_DeviceType_HMD:
		if (inputTableHMDSize > 0)
		{
			for (index = 0; index < (int)inputTableHMDSize; index++)
			{
				if (inputTableHMD[index].destination.id == destination)
				{
					destination = inputTableHMD[index].source.id;
					result = true;
				}
			}
		}
		break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Right:
		if (inputTableRightSize > 0)
		{
			for (index = 0; index < (int)inputTableRightSize; index++)
			{
				if (inputTableRight[index].destination.id == destination)
				{
					destination = inputTableRight[index].source.id;
					result = true;
				}
			}
		}
		break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Left:
		if (inputTableLeftSize > 0)
		{
			for (index = 0; index < (int)inputTableLeftSize; index++)
			{
				if (inputTableLeft[index].destination.id == destination)
				{
					destination = inputTableLeft[index].source.id;
					result = true;
				}
			}
		}
		break;
	default:
		break;
	}

	return result;
}
#pragma endregion Key Mapping
#pragma region
bool FWaveVRHMD::IsDeviceConnected(WVR_DeviceType device) const {
	if (device == WVR_DeviceType::WVR_DeviceType_Invalid)
		return false;
	return s_DeviceConnected[(int)device];
}
void FWaveVRHMD::UpdateConnectionsWhenNoEvent()
{
	if (!toUpdateConnectionsWhenNoEvent)
		return;

	bool conn = false;
	for (int i = 1; i < kWVRDeviceCount; i++)	// Starts from 1 (WVR_DeviceType_HMD)
	{
		WVR_DeviceType type = (WVR_DeviceType)i;
		conn = WVR()->IsDeviceConnected(type);
		if (s_DeviceConnected[i] != conn)
		{
			s_DeviceConnected[i] = conn;
			if (s_DeviceConnected[i])
			{
				switch (type)
				{
				case WVR_DeviceType::WVR_DeviceType_HMD:
					SetInputRequest(WVR_DeviceType::WVR_DeviceType_HMD, InputAttributes_HMD, InputAttributes_HMD_Count);
					break;
				case WVR_DeviceType::WVR_DeviceType_Controller_Left:
					SetInputRequest(WVR_DeviceType::WVR_DeviceType_Controller_Left, InputAttributes_Controller, InputAttributes_Controller_Count);
					break;
				case WVR_DeviceType::WVR_DeviceType_Controller_Right:
					SetInputRequest(WVR_DeviceType::WVR_DeviceType_Controller_Right, InputAttributes_Controller, InputAttributes_Controller_Count);
					break;
				default:
					break;
				}
			}
			else
			{
				ResetButtonStates(type);
			}

			if (s_TableStatic.Contains(type))
			{
				s_TableStatic[type] = false;
				LOGD(WVRHMD, "UpdateConnectionsWhenNoEvent() device %d connected %d, set table static to false.", (uint8)type, (uint8)s_DeviceConnected[i]);
			}

			UWaveVREventCommon::OnConnectionChangeNative.Broadcast((uint8)i, s_DeviceConnected[i]);
		}

		LOGD(WVRHMD, "UpdateConnectionsWhenNoEvent() Device %d connected %d", i, (uint8)s_DeviceConnected[i]);
	}

	toUpdateConnectionsWhenNoEvent = false;
}
#pragma endregion Connection
#pragma region
bool FWaveVRHMD::GetSimulationPose(FVector& OutPosition, FRotator& OutOrientation, WVR_DeviceType type)
{
	if (pSimulator == nullptr)
		return false;

	return pSimulator->GetSimulationPose(OutPosition, OutOrientation, type);
}
bool FWaveVRHMD::GetSimulationPose(FVector& OutPosition, FQuat& OutOrientation, WVR_DeviceType type)
{
	if (pSimulator == nullptr)
		return false;

	return pSimulator->GetSimulationPose(OutPosition, OutOrientation, type);
}
bool FWaveVRHMD::GetSimulationPressState(WVR_DeviceType device, WVR_InputId id)
{
	if (pSimulator == nullptr)
		return false;

	return pSimulator->GetSimulationPressState(device, id);
}
#pragma endregion Simulation Pose
#pragma region
WVR_InteractionMode FWaveVRHMD::GetInteractionMode()
{
	return m_InteractionMode;
}
/*bool FWaveVRHMD::SetInteractionMode(WVR_InteractionMode mode)
{
	bool ret = WVR()->SetInteractionMode(mode);
	if (ret)
	{
		UpdateInteractionMode();
		LOGD(WVRHMD, "SetInteractionMode() m_InteractionMode: %d", (uint8)m_InteractionMode);
	}
	return ret;
}*/
void FWaveVRHMD::UpdateInteractionMode()
{
	m_InteractionMode = WVR()->GetInteractionMode();
	LOGD(WVRHMD, "UpdateInteractionMode() m_InteractionMode: %d", (uint8)m_InteractionMode);
}
WVR_GazeTriggerType FWaveVRHMD::GetGazeTriggerType()
{
	return m_GazeType;
}
bool FWaveVRHMD::SetGazeTriggerType(WVR_GazeTriggerType type)
{
	bool ret = WVR()->SetGazeTriggerType(type);
	LOGD(WVRHMD, "SetGazeTriggerType() type: %d result %d", (uint8)type, (uint8)ret);
	if (ret)
		UpdateGazeType();
	return ret;
}
void FWaveVRHMD::UpdateGazeType()
{
	m_GazeType = WVR()->GetGazeTriggerType();
}
#pragma endregion Interaction Mode
#pragma region
void FWaveVRHMD::SetFocusedController(WVR_DeviceType focus)
{
	LOGD(WVRHMD, "SetFocusedController() %d", (uint8)focus);
	if (focus != WVR_DeviceType::WVR_DeviceType_Controller_Right && focus != WVR_DeviceType::WVR_DeviceType_Controller_Left)
		return;

	FWaveVRAPIWrapper::GetInstance()->SetFocusedController(focus);
	UpdateFocusedController();
	LOGD(WVRHMD, "SetFocusedController() focused controller: %d", (uint8)m_FocusedController);
}
WVR_DeviceType FWaveVRHMD::GetFocusedController()
{
	return m_FocusedController;
}
void FWaveVRHMD::UpdateFocusedController()
{
	m_FocusedController = FWaveVRAPIWrapper::GetInstance()->GetFocusedController();
}
#pragma endregion Focus Controller
#pragma region
bool FWaveVRHMD::IsDeviceTableStatic(WVR_DeviceType type)
{
	if (s_TableStatic.Contains(type)) { return s_TableStatic[type]; }
	return false;
}
void FWaveVRHMD::UpdateTableStatic()
{
	for (TPair< WVR_DeviceType, bool >& pair : s_TableStatic)
	{
		pair.Value = FWaveVRAPIWrapper::GetInstance()->IsDeviceTableStatic(pair.Key);
	}
}
#pragma endregion Simultaneous Interaction


void FWaveVRHMD::CheckSystemFocus() {
	// TODO IsInputFocusCapuredBySystem() will block!
	bool focus = WVR()->IsInputFocusCapturedBySystem();
	if (bFocusCapturedBySystem != focus)
	{
		bFocusCapturedBySystem = focus;
		LOGD(WVRHMD, "CheckSystemFocus() bFocusCapturedBySystem: %d", (uint8)bFocusCapturedBySystem);

		// --------------------- Actions taken on QuickMenu closed begins ---------------------
		if (!bFocusCapturedBySystem)
		{
			LOGD(WVRHMD, "CheckSystemFocus() Gets focus - 1. Updates the focused controller.");
			UpdateFocusedController();
			LOGD(WVRHMD, "CheckSystemFocus() m_FocusedController: %d", (uint8)m_FocusedController);

			LOGD(WVRHMD, "CheckSystemFocus() Gets focus - 2. Updates all button states.");
			UpdateButtonStates();

			if (inputTableHMDSize == 0)
			{
				LOGD(WVRHMD, "CheckSystemFocus() Gets focus - 3. Updates inputTable HMD.");
				UpdateInputMappingTable(WVR_DeviceType::WVR_DeviceType_HMD);
			}
			if (inputTableRightSize == 0)
			{
				LOGD(WVRHMD, "CheckSystemFocus() Gets focus - 3. Updates inputTable Right.");
				UpdateInputMappingTable(WVR_DeviceType::WVR_DeviceType_Controller_Right);
			}
			if (inputTableLeftSize == 0)
			{
				LOGD(WVRHMD, "CheckSystemFocus() Gets focus - 3. Updates inputTable Left.");
				UpdateInputMappingTable(WVR_DeviceType::WVR_DeviceType_Controller_Left);
			}
		}
		// --------------------- Actions taken on QuickMenu closed ends ---------------------
	}
}

bool FWaveVRHMD::IsFocusCapturedBySystem() const {
	return bFocusCapturedBySystem;
}

bool FWaveVRHMD::IsStereoEnabledInternal() const
{
	// Internal function will not use the LOG_FUNC()
	return bStereoEnabled && bHmdEnabled;
}

bool FWaveVRHMD::IsStereoEnabled() const
{
	LOG_FUNC();
	return bStereoEnabled && bHmdEnabled;
}

float FWaveVRHMD::GetWorldToMetersScale() const
{
	LOG_FUNC();
	if (IsInGameThread() && GWorld != nullptr)
	{
		// For example, One world unit need multiply 100 to become 1 meter.
		float wtm = GWorld->GetWorldSettings()->WorldToMeters;
		//LOGI(WVRHMD, "GWorld->GetWorldSettings()->WorldToMeters = %f", wtm);
		return wtm;
	}
	return 100.0f;
}

bool FWaveVRHMD::EnableStereo(bool bStereo)
{
	LOG_FUNC();
	// Only Enabled if the develop check the 'Start in VR' from project settings.  Therefore the bStereoEnabled could be initiallized true.
	LOGI(WVRHMD, "EnableStereo = %d", bStereo);

	bStereoEnabled = bStereo;
	return bStereoEnabled;
}

void FWaveVRHMD::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{
	LOG_FUNC();
	//LOGI(WVRHMD, "AdjustViewRect eye %d (%d, %d, %d, %d) before", (int)StereoPass, X, Y, SizeX, SizeY);
#if WITH_EDITOR
	if (GIsEditor) {
		// We should not use these input values.  How to get the screen size?
		SizeX = SizeX / 2;
		SizeY = SizeY;

		// Put in center
		X = (StereoPass == eSSP_RIGHT_EYE ? SizeX : 0);
		Y = 0;
		return;
	}
#endif

	if (!mRender.IsCustomPresentSet()) {
		LOGD(WVRHMD, "mRender is not ready for VR.");
		return;
	}

	// Case below: zoom in when pd < 1, zoom out when pd > 1
	// * FMath::Sqrt(mRender.GetPixelDensity()) can support up to 1.4x but only center have content.
	// / FMath::Sqrt(mRender.GetPixelDensity()) not match center and looks strange
	// * mRender.GetPixelDensity() can't match up to 1.1 and also 0.6. Can put in center.
	// Use scaledWidth can fit all case.
	uint32 width = mRender.GetSingleEyeScaledPixelWidth(), height = mRender.GetSingleEyeScaledPixelHeight();

	if (GSupportsMobileMultiView) {
		SizeX = width;
		SizeY = height;
		X = 0;
		Y = 0;
	} else {
		SizeX = width;
		SizeY = height;
		X = (StereoPass == eSSP_RIGHT_EYE ? width : 0);
		Y = 0;
	}

	//LOGI(WVRHMD, "AdjustViewRect eye %d (%d, %d, %d, %d) after", (int)StereoPass, X, Y, SizeX, SizeY);
}

// Need ScreenPercentage is true
void FWaveVRHMD::SetFinalViewRect(class FRHICommandListImmediate& RHICmdList, const enum EStereoscopicPass StereoPass, const FIntRect& FinalViewRect) {
	LOG_FUNC();
	//LOGI(WVRHMD, "SetFinalViewRect StereoPass(%d) FinalViewRect MinX(%d),MaxX(%d), MinY(%d),MaxY(%d)", (int)StereoPass, FinalViewRect.Min.X, FinalViewRect.Max.X, FinalViewRect.Min.Y, FinalViewRect.Max.Y);
}

// Will call to GetRelativeEyePose
void FWaveVRHMD::CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation)
{
	LOG_FUNC();
	FHeadMountedDisplayBase::CalculateStereoViewOffset(StereoPassType, ViewRotation, WorldToMeters, ViewLocation);
}

FMatrix FWaveVRHMD::GetStereoProjectionMatrix(const enum EStereoscopicPass StereoPassType) const
{
	LOG_FUNC();
	if (!IsStereoEnabledInternal())
		return FMatrix::Identity;

	if (StereoPassType == eSSP_LEFT_EYE) {
		return LeftProjection;
	} else if (StereoPassType == eSSP_RIGHT_EYE) {
		return RightProjection;
	} else if (StereoPassType == eSSP_FULL) {
		return CenterProjection;
	}

	return FMatrix::Identity;
}

void FWaveVRHMD::InitCanvasFromView(FSceneView* InView, UCanvas* Canvas)
{
	LOG_FUNC();
	FHeadMountedDisplayBase::InitCanvasFromView(InView, Canvas);
}

FWaveVRHMD::FWaveVRHMD(const FAutoRegister& AutoRegister)
	: FHeadMountedDisplayBase(nullptr)
	, FDefaultStereoLayers(AutoRegister, this)
	, bUseUnrealDistortion(GIsEditor)

	, bIsLeftHanded(false)

	, inputTableHMDSize(0)
	, inputTableRightSize(0)
	, inputTableLeftSize(0)
	, toUpdateConnectionsWhenNoEvent(false)

	, bResumed(false)
	, supportedFeatures(0)

	, DistortionPointsX(40)
	, DistortionPointsY(40)
	, NumVerts(0)
	, NumTris(0)
	, NumIndices(0)
	, DistortionMeshIndices(nullptr)
	, DistortionMeshVerticesLeftEye(nullptr)
	, DistortionMeshVerticesRightEye(nullptr)

	, bHmdEnabled(true)
	, bStereoEnabled(false)
	, CurrentIPD(0.064f)

	, NearClippingPlane(10.0f)
	, FarClippingPlane(0.0f)
	, bNeedResetProjectionMatrix(true)

	, mRender(this)
	, PoseMngr(nullptr)
	, mWaveAR(FWaveAR::GetInstance())

	, bFocusCapturedBySystem(false)
	, bSIM_Available(false)
	, FirstGameFrame(true)
	, bAdaptiveQuality(false)
	, AdaptiveQualityMode(0)
	, AdaptiveQualityStrategyFlags(WVR_QualityStrategy_Default)
	, AppliedAdaptiveQualityProjectSettings(false)

	, lateUpdateConfig()

	, SimulatedLoadingGameThread(0)
	, SimulatedLoadingRenderThread(0)
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);

	Startup();

#define LOCAL_EOL ", "  // " \n"
	LOGI(WVRHMD, "Initialized variable:" LOCAL_EOL
		"bUseUnrealDistortion %d" LOCAL_EOL
		"bHmdEnabled %d" LOCAL_EOL
		"bStereoEnabled %d" LOCAL_EOL
	, bUseUnrealDistortion, bHmdEnabled, bStereoEnabled);
#undef LOCAL_EOL
}

FWaveVRHMD::~FWaveVRHMD()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	Shutdown();
}

void ReportWVRError(WVR_InitError error)
{
	switch (error)
	{
		case WVR_InitError_None:
			break;
		case WVR_InitError_NotInitialized:
			LOGD(WVRHMD, "WaveVR: Not initialized");
			break;
		case WVR_InitError_Unknown:
			LOGD(WVRHMD, "WaveVR: Unknown error during initializing");
			break;
		default:
			//LOGD(WVRHMD, "WaveVR: InitError default case");
			break;
	}
}

void ReportRenderError(WVR_RenderError render_error)
{
	switch (render_error)
	{
		case WVR_RenderError_None:
			LOGD(WVRHMD, "WaveVR: RenderError: None");
			break;
		case WVR_RenderError_RuntimeInitFailed:
			LOGD(WVRHMD, "WaveVR: RenderError: RuntimeInitFailed");
			break;
		case WVR_RenderError_ContextSetupFailed:
			LOGD(WVRHMD, "WaveVR: RenderError: ContextSetupFailed");
			break;
		case WVR_RenderError_DisplaySetupFailed:
			LOGD(WVRHMD, "WaveVR: RenderError: DisplaySetupFailed");
			break;
		case WVR_RenderError_LibNotSupported:
			LOGD(WVRHMD, "WaveVR: RenderError: LibNotSupported");
			break;
		case WVR_RenderError_NullPtr:
			LOGD(WVRHMD, "WaveVR: RenderError: NullPtr");
			break;
		default:
			//LOGD(WVRHMD, "WaveVR: RenderError default case");
			break;
	}
}

void FWaveVRHMD::SetDeviceSupportedMaxFPS()
{
	LOG_FUNC();
	WVR_RenderProps props;
	if (WVR()->GetRenderProps(&props)) {
		float fps = props.refreshRate;
		LOGI(WVRHMD, "Set FreshRate as %f", fps);
		GEngine->SetMaxFPS(fps);  // If set to 500, the tick frequency will become 500 if possible.
		//IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
		//CVar->Set(fps);
	}
	else {
		LOGW(WVRHMD, "Get device FreshRate error! Not success!");
	}
}

// This is only used in FWaveVRHMD constructor
bool FWaveVRHMD::Startup()
{
	LOG_FUNC();

	FrameData = FFrameData::NewInstance();
	FrameDataRT = FFrameData::NewInstance();
	OldFrameData = FFrameData::NewInstance();
	OldFrameDataRT = FFrameData::NewInstance();

	// load WaveVR project settings from ini
	ApplyCVarSettingsFromIni(TEXT("/Script/WaveVREditor.WaveVRSettings"), *GEngineIni, ECVF_SetByProjectSetting);

	// Apply AdaptiveQuality params from project settings
	SetupAdaptiveQuality();

	WVR_InitError error = WVR()->Init(WVR_AppType_VRContent);

	if (error != WVR_InitError_None)
	{
		ReportWVRError(error);
		WVR()->Quit();
		LOGD(WVRHMD, "WVR_Quit");
		return false;
	}
#if !UE_BUILD_SHIPPING
	WVR()->SetChecker(true);
#endif

	PoseMngr = PoseManagerImp::GetInstance();
	supportedFeatures = WVR()->GetSupportedFeatures();
	LOGI(WVRHMD, "Startup() supportedFeatures: %d", (int)supportedFeatures);

	pSimulator = new PoseSimulator();
	pHandPose = new WaveVRHandPose();
	pHandPose->InitHandData();
	pEyeManager = new WaveVREyeManager();
	pEyeManager->InitEyeData();
	pTracker = new WaveVRTrackerImpl();
	pTracker->InitTrackerData();
	pLipExp = new WaveVRLipExpImpl();
	pLipExp->InitLipExpData();
	pEyeExp = new WaveVREyeExpImpl();
	pEyeExp->InitEyeExpData();

	WVR()->SetArmModel(WVR_SimulationType::WVR_SimulationType_Auto);

	mappingTableHash.Empty();
	mappingTableHash.Add(WVR_DeviceType::WVR_DeviceType_HMD, 0);
	mappingTableHash.Add(WVR_DeviceType::WVR_DeviceType_Controller_Left, 0);
	mappingTableHash.Add(WVR_DeviceType::WVR_DeviceType_Controller_Right, 0);

	s_TableStatic.Empty();
	s_TableStatic.Add(WVR_DeviceType::WVR_DeviceType_Controller_Left, false);
	s_TableStatic.Add(WVR_DeviceType::WVR_DeviceType_Controller_Right, false);

	// --------------------- Actions taken on Start & Resume begins ---------------------
	//pollEvent(); //get device connect info.
	CheckLeftHandedMode();
	LOGI(WVRHMD, "Startup() bIsLeftHanded: %d", (uint8)bIsLeftHanded);
	toUpdateConnectionsWhenNoEvent = true;
	UpdateConnectionsWhenNoEvent();
	UpdateButtonStates();
	UpdateInteractionMode();
	LOGI(WVRHMD, "Startup() m_InteractionMode: %d", (uint8)m_InteractionMode);
	UpdateGazeType();
	LOGI(WVRHMD, "Startup() m_GazeType: %d", (uint8)m_GazeType);
	UpdateFocusedController();
	LOGI(WVRHMD, "Startup() m_FocusedController: %d", (uint8)m_FocusedController);
	UpdateTableStatic();
	for (const TPair< WVR_DeviceType, bool >& pair : s_TableStatic)
	{
		LOGI(WVRHMD, "Startup() device %d static %d", (uint8)pair.Key, (uint8)pair.Value);
	}
	// --------------------- Actions taken on Start & Resume ends ---------------------


#if WITH_EDITOR
	if (!GIsEditor)
#endif
	{
		FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &FWaveVRHMD::ApplicationWillEnterBackgroundDelegate);
		//calls to this delegate are often (always?) paired with a call to ApplicationWillEnterBackgroundDelegate(), but cover the possibility that only this delegate is called
		FCoreDelegates::ApplicationWillDeactivateDelegate.AddRaw(this, &FWaveVRHMD::ApplicationWillDeactivateDelegate);
		FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &FWaveVRHMD::ApplicationHasEnteredForegroundDelegate);
		//calls to this delegate are often (always?) paired with a call to ApplicationHasEnteredForegroundDelegate(), but cover the possibility that only this delegate is called
		FCoreDelegates::ApplicationHasReactivatedDelegate.AddRaw(this, &FWaveVRHMD::ApplicationHasReactivatedDelegate);
		FCoreDelegates::OnFEngineLoopInitComplete.AddRaw(this, &FWaveVRHMD::OnFEngineLoopInitComplete);
		//Handle PreLoadMap and PostLoadMap
		FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &FWaveVRHMD::OnPreLoadMap);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FWaveVRHMD::OnPostLoadMap);
	}

	NextFrameData();
	if (FrameData->bSupportLateUpdate)
	{
		// LateUpdate will need first frame's pose.
		FrameData->bDoUpdateInGT = true;
		PoseMngr->UpdatePoses(FrameData);
	}

	NearClippingPlane = GNearClippingPlane;
	ResetProjectionMats();

	if (bUseUnrealDistortion)
		SetNumOfDistortionPoints(40, 40);

	//UGameUserSettings* Settings = GetGameUserSettings();
	//if (Settings != nullptr)
	//{
	//	LOGD(WVRHMD, "RequestResolutionChange")
	//	{
	//		uint32 width = 0, height = 0;
	//		Settings->RequestResolutionChange(width * 2, height, EWindowMode::Type::Windowed, false);
	//	}
	//}

	uint32 width = 1024, height = 1024;
	WVR()->GetRenderTargetSize(&width, &height);
	mRender.SetSingleEyePixelSize(width, height);
	mRender.SetTextureFormat(PF_R8G8B8A8);
	// If we force update, the width height will be correct at begining.
	// However the Unreal's MobileMultiViewSceneDepth will become half size.
	// Thus we keep it incorrect...
	mRender.SetPixelDensity(1.0f, true /* forceUpdate */);
	// mRender.SetPixelDensity(1.0f, false /* forceUpdate */);
	mRender.Apply();

	Instance = this;

#if WITH_EDITOR
	if (!GIsEditor)
#endif
	{
		WaveVRSplash = MakeShareable(new FWaveVRSplash(&mRender));
		WaveVRSplash->Init();
	}

	{
		UpdatePoseModeData();
		//InitPoseModeAsPanel
		UWaveVRBlueprintFunctionLibrary::SetControllerPoseMode(EWVR_Hand::Hand_Controller_Right, EWVR_ControllerPoseMode::ControllerPoseMode_Panel);
		UWaveVRBlueprintFunctionLibrary::SetControllerPoseMode(EWVR_Hand::Hand_Controller_Left, EWVR_ControllerPoseMode::ControllerPoseMode_Panel);
	}

	return true;
}

void FWaveVRHMD::OnPause()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	if (!bResumed)
		return;
	bResumed = false;
	mRender.OnPause();
}

void FWaveVRHMD::OnResume()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	if (bResumed)
		return;
	bResumed = true;
	UWaveVREventCommon::OnResumeNative.Broadcast();

	// --------------------- Actions taken on Start & Resume begins ---------------------
	CheckLeftHandedMode();
	LOGI(WVRHMD, "OnResume() bIsLeftHanded: %d", (uint8)bIsLeftHanded);
	toUpdateConnectionsWhenNoEvent = true;
	UpdateConnectionsWhenNoEvent();
	UpdateButtonStates();
	UpdateInteractionMode();
	LOGI(WVRHMD, "OnResume() m_InteractionMode: %d", (uint8)m_InteractionMode);
	UpdateGazeType();
	LOGI(WVRHMD, "OnResume() m_GazeType: %d", (uint8)m_GazeType);
	UpdateFocusedController();
	LOGI(WVRHMD, "OnResume() m_FocusedController: %d", (uint8)m_FocusedController);
	UpdateTableStatic();
	for (const TPair< WVR_DeviceType, bool >& pair : s_TableStatic)
	{
		LOGI(WVRHMD, "OnResume() device %d static %d", (uint8)pair.Key, (uint8)pair.Value);
	}

	if (pHandPose)
	{
		LOGI(WVRHMD, "OnResume() checks hand pose fusion.");
		pHandPose->CheckPoseFusion();
	}
	// --------------------- Actions taken on Start & Resume ends ---------------------

	mRender.OnResume();

	FDebugCanvasHandler* DebugCanvasHandler = FDebugCanvasHandler::GetInstance();
	if(DebugCanvasHandler != nullptr) {
		DebugCanvasHandler->ResetDebugCanvasOverlay();
	}
}

void FWaveVRHMD::ApplicationWillEnterBackgroundDelegate()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	OnPause();
}

void FWaveVRHMD::ApplicationWillDeactivateDelegate()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	OnPause();
}

void FWaveVRHMD::ApplicationHasReactivatedDelegate()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	OnResume();
}

void FWaveVRHMD::ApplicationHasEnteredForegroundDelegate()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	OnResume();
}

void FWaveVRHMD::OnFEngineLoopInitComplete()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	SetDeviceSupportedMaxFPS();
}

void FWaveVRHMD::OnPreLoadMap(const FString&)
{
	LOG_FUNC_IF(1);
	if (IsRenderInitialized()) {
		LOGI(WVRHMD, "Force disable AdaptiveQuality to increase performance OnPreLoadMap.");
		FWaveVRAPIWrapper::GetInstance()->EnableAdaptiveQuality(false);
	}
}

void FWaveVRHMD::OnPostLoadMap(UWorld*)
{
	LOG_FUNC_IF(1);
	if (IsRenderInitialized()) {
		LOGI(WVRHMD, "Restore AdaptiveQuality status OnPostLoadMap.");
		FWaveVRAPIWrapper::GetInstance()->EnableAdaptiveQuality(bAdaptiveQuality, AdaptiveQualityStrategyFlags);
	}
}

void FWaveVRHMD::Shutdown()
{
	LOG_FUNC_IF(WAVEVR_LOG_ENTRY_LIFECYCLE);
	if (!GIsEditor)
	{
		mRender.Shutdown();
		WVR()->Quit();
	} else {

		if (DistortionMeshVerticesLeftEye != nullptr)
			delete[] DistortionMeshVerticesLeftEye;
		DistortionMeshVerticesLeftEye = nullptr;

		if (DistortionMeshVerticesRightEye != nullptr)
			delete[] DistortionMeshVerticesRightEye;
		DistortionMeshVerticesRightEye = nullptr;

		if (DistortionMeshIndices != nullptr)
			delete[] DistortionMeshIndices;
		DistortionMeshIndices = nullptr;

		if (bSIM_Available)
			WVR()->Quit();
	}

	Instance = nullptr;
}

static FMatrix MakeProjection(float Left, float Right, float Top, float Bottom, float ZNear, float ZFar = 0) {
	LOGI(WVRHMD, "MakeProjection (%f, %f, %f, %f, %f, %f)", Left, Right, Top, Bottom, ZNear, ZFar);

	float SumRL = (Right + Left);
	float SumTB = (Top + Bottom);
	float SubRL = Right - Left;
	float SubTB = Top - Bottom;

	if (ZFar > 0 && ZNear > 0 && ZFar > ZNear) {
		// Reversed Z [1, 0].  Use this function if ZFar is finite.
		return FMatrix(
			FPlane(2.0f / SubRL, 0.0f, 0.0f, 0.0f),
			FPlane(0.0f, 2.0f / SubTB, 0.0f, 0.0f),
			FPlane(SumRL / -SubRL, SumTB / -SubTB, ZNear / (ZNear - ZFar), 1.0f),
			FPlane(0.0f, 0.0f, ZFar * ZNear / (ZFar - ZNear), 0.0f)
		);
	}
	else
	{
		// Reversed Z [1, 0].  Use this function if ZFar is infinite.
		return FMatrix(
			FPlane(2.0f / SubRL, 0.0f, 0.0f, 0.0f),
			FPlane(0.0f, 2.0f / SubTB, 0.0f, 0.0f),
			FPlane(SumRL / -SubRL, SumTB / -SubTB, 0.0f, 1.0f),
			FPlane(0.0f, 0.0f, ZNear, 0.0f)
		);
	}
}

void FWaveVRHMD::ResetProjectionMats()
{
	LOG_FUNC();
	bNeedResetProjectionMatrix = false;

	float boundaries[3][4];
	WVR()->GetClippingPlaneBoundary(WVR_Eye_Left, &(boundaries[0][0]), &(boundaries[0][1]), &(boundaries[0][2]), &(boundaries[0][3]));
	WVR()->GetClippingPlaneBoundary(WVR_Eye_Right, &(boundaries[1][0]), &(boundaries[1][1]), &(boundaries[1][2]), &(boundaries[1][3]));

	// Center proj. use the most wide config.
	boundaries[2][0] = FMath::Min(boundaries[0][0], boundaries[1][0]);
	boundaries[2][1] = FMath::Max(boundaries[0][1], boundaries[1][1]);
	boundaries[2][2] = FMath::Max(boundaries[0][2], boundaries[1][2]);
	boundaries[2][3] = FMath::Min(boundaries[0][3], boundaries[1][3]);

	LeftProjection = MakeProjection(boundaries[0][0], boundaries[0][1], boundaries[0][2], boundaries[0][3], NearClippingPlane, FarClippingPlane);
	RightProjection = MakeProjection(boundaries[1][0], boundaries[1][1], boundaries[1][2], boundaries[1][3], NearClippingPlane, FarClippingPlane);
	CenterProjection = MakeProjection(boundaries[2][0], boundaries[2][1], boundaries[2][2], boundaries[2][3], NearClippingPlane, FarClippingPlane);
}

void FWaveVRHMD::SetClippingPlanes(float NCP, float FCP) {
	LOG_FUNC();
	NearClippingPlane = FMath::Max(NCP, 1.0f);
	FarClippingPlane = FMath::Max(FCP, 0.0f);

	GNearClippingPlane = NearClippingPlane;

	bNeedResetProjectionMatrix = true;

	// Do we need reset it immediatly?
	ResetProjectionMats();
}

bool FWaveVRHMD::IsDirectPreview() {
	LOG_FUNC();
	return bSIM_Available;
}

bool FWaveVRHMD::IsRenderFoveationSupport() {
	return mRender.IsRenderFoveationSupport();
}

bool FWaveVRHMD::IsRenderFoveationEnabled() {
	return mRender.IsRenderFoveationEnabled();
}

void FWaveVRHMD::SetFoveationMode(WVR_FoveationMode Mode) {
	mRender.SetFoveationMode(Mode);
}

void FWaveVRHMD::SetFoveationParams(EStereoscopicPass Eye, WVR_RenderFoveationParams_t& FoveatParams) {
	mRender.SetFoveationParams(Eye, FoveatParams);
}

void FWaveVRHMD::GetFoveationParams(EStereoscopicPass Eye, WVR_RenderFoveationParams_t& FoveatParams) {
	mRender.GetFoveationParams(Eye, FoveatParams);
}

bool FWaveVRHMD::IsSplashShowing() {
	if (mRender.IsInitialized() && mRender.IsCustomPresentSet() && WaveVRSplash.IsValid()) {
		return WaveVRSplash->IsShown();
	}
	return false;
}

void FWaveVRHMD::SetAdaptiveQualityState(bool enabled, uint32_t strategyFlags) {
	LOG_FUNC();
	bAdaptiveQuality = enabled;
	AdaptiveQualityStrategyFlags = strategyFlags;
}

void FWaveVRHMD::SetupAdaptiveQuality() {
	auto CVarAQEnable = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.AdaptiveQuality"));
	auto CVarAQMode = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.AdaptiveQuality.mode"));
	auto CVarAQEnableSendQualityEvent = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.AdaptiveQuality.sendQualityEvent"));
	auto CVarAQEnableAutoFoveation = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.AdaptiveQuality.autoFoveation"));
	bAdaptiveQuality = CVarAQEnable && CVarAQEnable->GetValueOnAnyThread() != 0;

	if (bAdaptiveQuality) {
		if (CVarAQMode) {
			AdaptiveQualityMode = CVarAQMode->GetValueOnAnyThread();
			if (CVarAQEnableSendQualityEvent && CVarAQEnableSendQualityEvent->GetValueOnAnyThread() != 0) {
				AdaptiveQualityStrategyFlags |= WVR_QualityStrategy_SendQualityEvent;
			}
			if (CVarAQEnableAutoFoveation && CVarAQEnableAutoFoveation->GetValueOnAnyThread() != 0) {
				AdaptiveQualityStrategyFlags |= WVR_QualityStrategy_AutoFoveation;
			}
			enum { Quality_Oriented, Performance_Oriented, Customization };
			if (AdaptiveQualityMode == Performance_Oriented){
				AdaptiveQualityStrategyFlags |= WVR_QualityStrategy_Reserved_1;
				AdaptiveQualityStrategyFlags |= WVR_QualityStrategy_AutoAMC;
			} else if (AdaptiveQualityMode == Quality_Oriented){
				AdaptiveQualityStrategyFlags |= WVR_QualityStrategy_Reserved_2;
			}
#if !UE_BUILD_SHIPPING
			//Verify if meet the conditions of AdaptiveQualityMode or not.
			bool MeetAQModeCondition = (AdaptiveQualityStrategyFlags&WVR_QualityStrategy_SendQualityEvent) && (AdaptiveQualityStrategyFlags&WVR_QualityStrategy_AutoFoveation);
			if ( (AdaptiveQualityMode == Performance_Oriented || AdaptiveQualityMode == Quality_Oriented) && !MeetAQModeCondition){
				LOGW(WVRHMD, "AdaptiveQuality did not meet the correct conditions");
			}
#endif //!UE_BUILD_SHIPPING
			LOGD(WVRHMD, "Setup AdaptiveQuality by Project Settings : AdaptiveQuality enabled(%u) with AdaptiveQualityMode(%u) and StrategyFlags(%u)", bAdaptiveQuality, AdaptiveQualityMode, AdaptiveQualityStrategyFlags);
		}
	} else {
		LOGW(WVRHMD, "AdaptiveQuality is disabled by project settings");
	}
}

void FWaveVRHMD::UpdatePoseModeData()
{
	LOG_FUNC();
	PoseModeOffsets.Empty();
	WVR_DeviceType hands[2] = {WVR_DeviceType_Controller_Right, WVR_DeviceType_Controller_Left };
	WVR_ControllerPoseMode modes[4] = {WVR_ControllerPoseMode_Raw, WVR_ControllerPoseMode_Trigger, WVR_ControllerPoseMode_Panel, WVR_ControllerPoseMode_Handle};

	//Update Offsets.
	for(auto type : hands) {
		for(auto mode : modes) {
			WVR_Vector3f_t translation;
			WVR_Quatf_t quaternion;
			FWaveVRAPIWrapper::GetInstance()->GetControllerPoseModeOffset(type, mode, &translation, &quaternion);
			LOGD(WVRHMD, "UpdatePoseModeData type(%d) mode(%d) translation(v1,v2,v3) = (%f, %f, %f) quaternion(w,x,y,z) = (%f, %f, %f, %f)"
				, type, mode, translation.v[0], translation.v[1], translation.v[2], quaternion.w, quaternion.x, quaternion.y, quaternion.z);
			FVector Translation = FVector(-translation.v[2], translation.v[0], translation.v[1]) * 100; // meter to centimeter.
			FQuat Quaternion= FQuat(-quaternion.z, quaternion.x, quaternion.y, -quaternion.w);
			FTransform Transform = FTransform(Quaternion, Translation, FVector::OneVector);
			PoseModeOffsets.Add(Transform);
		}
	}
}

void FWaveVRHMD::UpdatePoseModeAndBroadcast(WVR_DeviceType Type)
{
	LOG_FUNC();
	WVR_ControllerPoseMode WVRMode = WVR_ControllerPoseMode::WVR_ControllerPoseMode_Raw;
	FWaveVRAPIWrapper::GetInstance()->GetControllerPoseMode(Type, &WVRMode);
	uint8_t device_int = static_cast<uint8_t>(Type);
	uint8_t mode_int = static_cast<uint8_t>(WVRMode);
	int index = (device_int-2)*4 + mode_int;

	CachedPoseModes[device_int-2] = mode_int;
	UWaveVREventCommon::OnControllerPoseModeChangedNative.Broadcast(device_int, mode_int, PoseModeOffsets[index]);

	LOGD(WVRHMD, "Update mode and broadcast CachedPoseModes[%u] = %u", device_int-2, mode_int);
}

void FWaveVRHMD::PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
	LOG_FUNC();

	if (multiLayerManager)
	{
		multiLayerManager->UpdateTextures(RHICmdList);
		multiLayerManager->SubmitLayers();
	}
}

IStereoLayers::FLayerDesc FWaveVRHMD::GetDebugCanvasLayerDesc(FTextureRHIRef Texture)
{
	LOG_FUNC();
	IStereoLayers::FLayerDesc StereoLayerDesc;
	StereoLayerDesc.Id = 1;
	if (!isMultiLayerSupported)
	{
		StereoLayerDesc.Transform = FTransform(FRotator(180, 0, 0), FVector(100.f, 0, 0), FVector::OneVector);
		StereoLayerDesc.UVRect = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
	}
	else
	{
		StereoLayerDesc.Transform = FTransform(FRotator(0, 0, 0), FVector(100.f, 0, 0), FVector::OneVector);
		StereoLayerDesc.UVRect = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
	}
	StereoLayerDesc.QuadSize = FVector2D(120.f, 120.f);
	StereoLayerDesc.PositionType = IStereoLayers::ELayerType::FaceLocked;
	StereoLayerDesc.SetShape<FQuadLayer>();
	StereoLayerDesc.Texture = Texture;
	StereoLayerDesc.Priority = 10;
	StereoLayerDesc.Flags = IStereoLayers::ELayerFlags::LAYER_FLAG_TEX_CONTINUOUS_UPDATE;
	StereoLayerDesc.Flags |= IStereoLayers::ELayerFlags::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO;
	return StereoLayerDesc;
}

bool FWaveVRHMD::IsRenderInitialized() {
	return mRender.IsInitialized();
}

void FWaveVRHMD::RenderSetSubmitWithPose(bool enable, WVR_PoseState_t * pose)
{
	mRender.SetSubmitWithPose(enable, pose);
}


void FWaveVRHMD::SimulateCPULoading(unsigned int gameThreadLoading, unsigned int renderThreadLoading) {
#if (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG)
	LOGW(WVRHMD, "Using Sleep to simulate CPU loading: GT %7.3fms, RT %7.3fms", gameThreadLoading / 1000.0f, renderThreadLoading / 1000.0f);
	SimulatedLoadingGameThread = FMath::Clamp<unsigned int>(gameThreadLoading, 0, 100000);
	SimulatedLoadingRenderThread = FMath::Clamp<unsigned int>(renderThreadLoading, 0, 100000);
#endif
}

void FWaveVRHMD::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	LOG_FUNC();
}

#pragma region MultiLayer
void FWaveVRHMD::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	LOG_FUNC();

	// Code below can help not to render before VR get ready content.
	// EngingeShowFlags.StereoRendering will set true before we get ready.
#if !PLATFORM_ANDROID
	// In Editor, the Render.IsInitialized() will be always false, and the editor's view will be black.
	if (false)
#endif
	{
		if (!mRender.IsInitialized()) {
			InViewFamily.EngineShowFlags.StereoRendering = false;
			InViewFamily.EngineShowFlags.Rendering = false;
		}
	}

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::SetupViewFamily(InViewFamily);
	}

}

void FWaveVRHMD::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::PreRenderViewFamily_RenderThread(RHICmdList, InViewFamily);
	}
}
void FWaveVRHMD::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	LOG_FUNC();

}

void FWaveVRHMD::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::PostRenderView_RenderThread(RHICmdList, InView);
	}
	else 
	{
		FDebugCanvasHandler* DebugCanvasHandler = FDebugCanvasHandler::GetInstance();
		if (DebugCanvasHandler != nullptr) {
			DebugCanvasHandler->PostRenderView_RenderThread(InView);
		}
	}
}

uint32  FWaveVRHMD::CreateLayer(const IStereoLayers::FLayerDesc& InLayerDesc)
{
	LOG_FUNC();
	MultiLayerSupportCheck();
	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::CreateLayer(InLayerDesc);
	}

	if (!multiLayerManager)
	{
		multiLayerManager = FWaveVRMultiLayerManager::GetInstance();
	}

	if (multiLayerManager)
	{
		if (multiLayerManager->GetCurrentLayerCount() < maxLayerCount)
		{
			LOGD(WVRHMD, "FWaveVRHMD::CreateLayer\nInLayerDesc: Quad Size X:%f Y:%f Priority: %d", InLayerDesc.QuadSize.X, InLayerDesc.QuadSize.Y, InLayerDesc.Priority);
			return multiLayerManager->CreateLayer(InLayerDesc);
		}
		else
		{
			if (InLayerDesc.Id == 1)
			{
				return multiLayerManager->CreateLayer(InLayerDesc);
			}

			LOGD(WVRHMD, "Max Layer Count Reached, not creating new layers.");
			return 0;
		}
	}

	LOGD(WVRHMD, "FWaveVRHMD::CreateLayer : Multi layer manager instance not found");
	return 0;
}

void  FWaveVRHMD::DestroyLayer(uint32 LayerId)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::DestroyLayer(LayerId);
	}

	if (multiLayerManager)
	{
		multiLayerManager->MarkLayerToDestroy(LayerId);
	}
	else
	{
		LOGD(WVRHMD, "FWaveVRHMD::DestroyLayer : Multi layer manager instance not found");
	}
}

void  FWaveVRHMD::SetLayerDesc(uint32 LayerId, const IStereoLayers::FLayerDesc& InLayerDesc)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::SetLayerDesc(LayerId, InLayerDesc);
	}

	if (multiLayerManager)
	{
		LOGD(WVRHMD, "FWaveVRHMD::SetLayerDesc\nInLayerDesc: Quad Size X:%f Y:%f Priority: %d", InLayerDesc.QuadSize.X, InLayerDesc.QuadSize.Y, InLayerDesc.Priority);
		return multiLayerManager->SetLayerDesc(LayerId, InLayerDesc);
	}
}

bool  FWaveVRHMD::GetLayerDesc(uint32 LayerId, IStereoLayers::FLayerDesc& OutLayerDesc)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::GetLayerDesc(LayerId, OutLayerDesc);
	}

	if (multiLayerManager)
	{
		LOGD(WVRHMD, "FWaveVRHMD::GetLayerDesc");
		return multiLayerManager->GetLayerDesc(LayerId, OutLayerDesc);
	}

	return false;
}

void  FWaveVRHMD::MarkTextureForUpdate(uint32 LayerId)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::MarkTextureForUpdate(LayerId);
	}
}

void  FWaveVRHMD::GetAllocatedTexture(uint32 LayerId, FTextureRHIRef &Texture, FTextureRHIRef &LeftTexture)
{
	LOG_FUNC();

	if (!isMultiLayerSupported)
	{
		//LOGD(WVRHMD, "Platform does not support multilayers, fall back to default UE4 behaviour");
		return FDefaultStereoLayers::GetAllocatedTexture(LayerId, Texture, LeftTexture);
	}
}

bool FWaveVRHMD::MultiLayerSupportCheck()
{
	maxLayerCount = WVR()->GetMaxCompositionLayerCount();
	LOGD(WVRHMD, "FWaveVRHMD::MultiLayerSupportCheck, Max Layer Count is %u", maxLayerCount);

	if (maxLayerCount <= 0)
	{
		isMultiLayerSupported = false;
	}
	else
	{
		isMultiLayerSupported = true;
	}

	return isMultiLayerSupported;
}

#pragma endregion

bool FWaveVRHMD::bVRTransitionIsShowning = false;
bool FWaveVRHMD::bIsVRTransitionInUse = false;

void FWaveVRHMD::SetVRTransitionState(bool state)
{
	LOG_FUNC();
        if (bIsVRTransitionInUse == false)
                bIsVRTransitionInUse = true;
        bVRTransitionIsShowning = state;
        if (bVRTransitionIsShowning) {
                LOGD(WVRHMD, "SetVRTransitionState(true)");
        } else {
                LOGD(WVRHMD, "SetVRTransitionState(false)");
        }
}

bool FWaveVRHMD::IsVRTransitionShowing()
{
	LOG_FUNC();
        bool result = false;
        if (bIsVRTransitionInUse) {
                result = bVRTransitionIsShowning;
        } else {
                result = !UWaveVRBlueprintFunctionLibrary::IsDevicePoseValid(EWVR_DeviceType::DeviceType_HMD); //SVR
        }
        return result;
}
void FWaveVRHMD::SetFrameSharpnessEnhancementLevel(float level)
{
	LOG_FUNC();
	mRender.SetFrameSharpnessEnhancementLevel(level);
}
