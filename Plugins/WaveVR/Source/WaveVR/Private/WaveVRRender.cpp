// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRRender.h"
#include "Logging/LogMacros.h"
#include "RHI.h"
#include "RHIUtilities.h"
#include "OpenGLDrv.h"
#include "OpenGLResources.h"
#include "XRThreadUtils.h"
#include "Widgets/SViewport.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

#include "WaveVRHMD.h"
#include "WaveVRTexturePool.h"
#include "WaveVRUtils.h"
#include "WaveVRRenderDoc.h"

using namespace Wave::Render;

#ifdef WVR_SCOPED_NAMED_EVENT
#undef WVR_SCOPED_NAMED_EVENT
#endif
#if (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG) && !WITH_EDITOR
#define WVR_SCOPED_NAMED_EVENT(name, color) SCOPED_NAMED_EVENT(name, color)
#else
#define WVR_SCOPED_NAMED_EVENT(name, color)
#endif

//#if PLATFORM_ANDROID
//#define WAVEVR_LOG_SHOW_ALL_ENTRY 1
//#endif

#if defined(WAVEVR_LOG_SHOW_ALL_ENTRY) && (PLATFORM_ANDROID || PLATFORM_WINDOWS)
#if WAVEVR_LOG_SHOW_ALL_ENTRY && !WITH_EDITOR
#ifdef LOG_FUNC
#undef LOG_FUNC
#endif
#define LOG_FUNC() LOGD(WVRRender, "%s", WVR_FUNCTION_STRING) // ;FDebug::DumpStackTraceToLog();
#endif
#define LOG_STACKTRACE() FDebug::DumpStackTraceToLog()
#endif //PLATFORM_ANDROID

#ifndef PTR_TO_INT
#if PLATFORM_ANDROID && PLATFORM_ANDROID_ARM //(32bit)
#define PTR_TO_INT(x) (int)x
#define INT_TO_PTR(x) (void*)x
#else //PLATFORM_ANDROID_ARM64 || PLATFORM_WINDOWS || WITH_EDITOR  /*Other 64bit*/
#define PTR_TO_INT(x) \
  (int)(long long)(x)
#define INT_TO_PTR(x) \
  (long long)(x)
#endif
#endif

/**
  MSAA and MultiView configuration TruthTable
  T for work, S for work and msaa4x, F for all fail.

  Multiview
    0  1   <- use Unreal queue
  0 F  F   Note: WVR api didn't accept/submit the multiview resources created from Unreal.
  1 S  S
  ^- create from resource

  No Multiview
    0  1   <- use Unreal queue
  0 S  S   Note: (0,0) is not smooth.  Maybe the queue order is not follow SVR's order.
  1 S  T
  ^- create from resource
 **/
#if 0  // Set 1 for internal test.  It will override the plugin settings.
#define WVRRENDER_USE_UNREAL_TEXTURE_QUEUE(input) false
#define WVRRENDER_CREATE_FROM_RESOURCE(input) true
#else
#define WVRRENDER_USE_UNREAL_TEXTURE_QUEUE(input) input
#define WVRRENDER_CREATE_FROM_RESOURCE(input) input
#endif

DEFINE_LOG_CATEGORY_STATIC(WVRRender, Display, All);

#if PLATFORM_ANDROID
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android_native_app_glue.h>
extern struct android_app* GNativeAndroidApp;
#endif // PLATFORM_ANDROID

typedef void (* PFNGLDEBUGMESSAGEINSERTLOCALPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
static PFNGLDEBUGMESSAGEINSERTLOCALPROC glDebugMessageInsertExt = nullptr;
static void PrepareGLExtAPI() {
#if PLATFORM_ANDROID
	if (glDebugMessageInsertExt == nullptr) {
		glDebugMessageInsertExt = (PFNGLDEBUGMESSAGEINSERTLOCALPROC)eglGetProcAddress("glDebugMessageInsert");
	}
#endif // PLATFORM_ANDROID
}

void ReportRenderError(WVR_RenderError render_error);

/****************************************************
 *
 * FWaveVRFXRRenderBridge
 *
 ****************************************************/

FWaveVRFXRRenderBridge::FWaveVRFXRRenderBridge(FWaveVRRender * render) :
	FXRRenderBridge(), mRender(render)
{
	LOG_FUNC();
}

FWaveVRFXRRenderBridge::~FWaveVRFXRRenderBridge()
{
	LOG_FUNC();
}

bool FWaveVRFXRRenderBridge::NeedsNativePresent()
{
	LOG_FUNC();
	return true; //D3D12 only now, not mobile.
}

bool FWaveVRFXRRenderBridge::Present(int& SyncInterval)
{
	LOG_FUNC();
	check(IsInRenderingThread());

	// Disable Unreal's VSync
	SyncInterval = 0;

	mRender->OnFinishRendering_RenderThread();

	// Not to swap
	return false;
}

void FWaveVRFXRRenderBridge::OnBackBufferResize()
{
	LOG_FUNC();
}

/****************************************************
 *
 * FWaveVRRender
 *
 ****************************************************/

FWaveVRRender::FWaveVRRender(FWaveVRHMD * hmd) :
	FXRRenderTargetManager(),
	mCustomPresent(nullptr),
	mTextureManager(this, hmd),
	mHMD(hmd),

	// WaveVR State
	bInitialized(false),

	// WaveVR Settings
	msaaLevel(1),
	width(720),  // A special value for not been confused to the correct size from wvr runtime.
	height(720),
	scaledWidth(720),
	scaledHeight(720),
	pixelDensity(1.0f),
	pixelDensityChanged(false),
	pixelFormat(PF_R8G8B8A8),
	isFoveatedRenderingSupported(false),
	isFoveatedRenderingEnabled(false),
	mCurrentFoveationMode(WVR_FoveationMode_Default),
	isMultiViewEnabled(false),
	isMultiViewDirectEnabled(false),
	defaultQueueSize(3),
	isSRGB(false),
	mAMCMode(WVR_AMCMode_Off),
	isFadeOutEnabled(false),
	isFrameSharpnessEnhancementEnabled(false),
	mFrameSharpnessEnhancementLevel(0.5f),

	bSubmitWithPose(false),
	bSubmitWithInteralPose(false),

	// Unreal state
	needReAllocateRenderTargetTexture(false),
	needReAllocateDepthTexture(false)

{
	LOG_FUNC();

	if (PLATFORM_WINDOWS) {
		isMultiViewEnabled = false;
		isMultiViewDirectEnabled = false;
		useUnrealTextureQueue = WVRRENDER_USE_UNREAL_TEXTURE_QUEUE(true);
		createFromResource = WVRRENDER_CREATE_FROM_RESOURCE(false);
	} else if (PLATFORM_ANDROID) {
		isSRGB = IsMobileColorsRGB();
		msaaLevel = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.MobileMSAA"))->GetValueOnAnyThread();
		auto CVarMobileMultiView = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
		isMultiViewEnabled = (CVarMobileMultiView && CVarMobileMultiView->GetValueOnAnyThread() != 0);
		auto CVarMobileMultiViewDirect = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
		isMultiViewDirectEnabled = (CVarMobileMultiViewDirect && CVarMobileMultiViewDirect->GetValueOnAnyThread() != 0);
		LOGD(WVRRender, "isSRGB %d, isMobileMultiViewEnabled %d, isMultiViewDirectEnabled %d, MobileMSAALevel %d", isSRGB, isMultiViewEnabled, isMultiViewDirectEnabled, msaaLevel);

		// Enable MultiView
		GSupportsMobileMultiView = isMultiViewEnabled;

		// According to our experiment, which is noted above.  Different cases need different configurations.
		if (isMultiViewEnabled) {
			useUnrealTextureQueue = WVRRENDER_USE_UNREAL_TEXTURE_QUEUE(true);
			createFromResource = WVRRENDER_CREATE_FROM_RESOURCE(true);
		} else {
			useUnrealTextureQueue = WVRRENDER_USE_UNREAL_TEXTURE_QUEUE(true);
			createFromResource = WVRRENDER_CREATE_FROM_RESOURCE(false); /*true in MSAA is not workable*/
		}
	}
}

FWaveVRRender::~FWaveVRRender()
{
	LOG_FUNC();
}

void FWaveVRRender::RenderInit()
{
	LOG_FUNC();
	if (IsInRenderingThread())
	{
		RenderInit_RenderThread();
	}
	else
	{
		FWaveVRRender * pRender = this;
		ENQUEUE_RENDER_COMMAND(RenderInit) (
			[pRender](FRHICommandListImmediate& RHICmdList)
			{
				pRender->RenderInit_RenderThread();
			});
		FlushRenderingCommands();
	}
}

void FWaveVRRender::UpdateConsoleVariable()
{
#if 0
	auto CVarColorGamutPreference0 = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.ColorGamut.preference0"));
	colorGamutPreferences[0] = (int)CVarColorGamutPreference0->GetValueOnAnyThread();
	
	auto CVarColorGamutPreference1 = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.ColorGamut.preference1"));
	colorGamutPreferences[1] = (int)CVarColorGamutPreference1->GetValueOnAnyThread();

	auto CVarColorGamutPreference2 = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.ColorGamut.preference2"));
	colorGamutPreferences[2] = (int)CVarColorGamutPreference2->GetValueOnAnyThread();

	auto CVarColorGamutPreference3 = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.ColorGamut.preference3"));
	colorGamutPreferences[3] = (int)CVarColorGamutPreference3->GetValueOnAnyThread();
#else
	for (int i = 0; i < 4; i++)
		colorGamutPreferences[i] = 0;
#endif

#if 0
	auto CVarTripleDepthBuffer = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.tripleDepthBuffer"));
	isTripleDepthBufferEnabled = (bool)CVarTripleDepthBuffer->GetValueOnAnyThread();
#else
	isTripleDepthBufferEnabled = false;
#endif

	// Console Variable
	auto CVarRMEnable = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.RenderMask.enable"));
	isRenderMaskEnabled = CVarRMEnable->GetValueOnAnyThread() != 0;

	//Init Eye Tracking param
	auto CVarETEnable = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.EyeTracking.enable"));
	isEyeTrackingEnabled = CVarETEnable->GetValueOnAnyThread() != 0;

	//Init Foveation Param
	auto CVarFRMode = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.FoveatedRendering.mode"));
	mCurrentFoveationMode = (WVR_FoveationMode)CVarFRMode->GetValueOnAnyThread();

	auto CVarFRPeripheralFOV = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.FoveatedRendering.peripheralFOV"));
	int32 frPeripheralFOV = CVarFRPeripheralFOV->GetValueOnAnyThread();

	auto CVarFRPeripheralQuality = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.FoveatedRendering.peripheralQuality"));
	WVR_PeripheralQuality frPeripheralQuality = (WVR_PeripheralQuality)CVarFRPeripheralQuality->GetValueOnAnyThread();

	EnableModeFoveationParams[0].focalX = EnableModeFoveationParams[0].focalY = 0.0f;
	EnableModeFoveationParams[0].fovealFov = frPeripheralFOV;
	EnableModeFoveationParams[0].periQuality = frPeripheralQuality;
	EnableModeFoveationParams[1].focalX = EnableModeFoveationParams[1].focalY = 0.0f;
	EnableModeFoveationParams[1].fovealFov = frPeripheralFOV;
	EnableModeFoveationParams[1].periQuality = frPeripheralQuality;

	// DEBUG
	//LOGI(WVRRender, "FoveatedRendering: Left Eye focalX (%f) focalY (%f) FOV(%f) periQuality(%d)",
	//	EnableModeFoveationParams[0].focalX, EnableModeFoveationParams[0].focalY,
	//	EnableModeFoveationParams[0].fovealFov, EnableModeFoveationParams[0].periQuality);

	//LOGI(WVRRender, "FoveatedRendering: Right Eye focalX (%f) focalY (%f) FOV(%f) periQuality(%d)",
	//	EnableModeFoveationParams[1].focalX, EnableModeFoveationParams[1].focalY,
	//	EnableModeFoveationParams[1].fovealFov, EnableModeFoveationParams[1].periQuality);

	auto CVarAMCModeLocal = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.AMC.mode"));
	if (CVarAMCModeLocal)
		mAMCMode = (WVR_AMCMode)CVarAMCModeLocal->GetValueOnAnyThread();

	auto CVarTimeWarpStabilizedMode = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.TimeWarpStabilizedMode"));
	mTimeWarpStabilizedMode = (WVR_TimeWarpStabilizedMode) CVarTimeWarpStabilizedMode->GetValueOnAnyThread();

	auto CVarFadeOut = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.FadeOut"));
	isFadeOutEnabled = (CVarFadeOut && CVarFadeOut->GetValueOnAnyThread() != 0);

	auto CVarFrameSharpnessEnhancement = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.FrameSharpnessEnhancement"));
	isFrameSharpnessEnhancementEnabled = (CVarFrameSharpnessEnhancement && CVarFrameSharpnessEnhancement->GetValueOnAnyThread() != 0);
}


void FWaveVRRender::RenderInit_RenderThread()
{
	LOG_FUNC();
	check(IsInRenderingThread());
	check(!IsInGameThread());

//#if WITH_EDITOR
//	if (GIsEditor) return;
//#endif

	UpdateConsoleVariable();

	{
		uint64_t render_config = WVR_RenderConfig_Default | isSRGB ? WVR_RenderConfig_sRGB : 0;

		if (mAMCMode == WVR_AMCMode::WVR_AMCMode_Force_UMC || mAMCMode == WVR_AMCMode::WVR_AMCMode_Auto)
			render_config |= WVR_RenderConfig_Initialize_UMC;

		if (isFadeOutEnabled)
			render_config |= WVR_RenderConfig_Initialize_FadeOut;

		if (isFrameSharpnessEnhancementEnabled)
			render_config |= WVR_RenderConfig_Initialize_FrameSharpnessEnhancement;

#if UE_BUILD_SHIPPING //Add GL_No_Error because shipping build change the elg config attribute since UE4.23.
		render_config |= WVR_RenderConfig_GL_No_Error;
		LOGI(WVRRender, "UE do shipping build if you see this line, additionally add WVR_RenderConfig_GL_No_Error render config while RenderInit");
#endif
		// We has the display config from device service, so we don't specify it.
		WVR_RenderInitParams_t param = {WVR_GraphicsApiType_OpenGL, render_config};
		WVR_RenderError render_error = WVR_RenderError_LibNotSupported;

#if 0
		// Must set display profile before RenderInit.
		const int count = sizeof(colorGamutPreferences) / sizeof(*colorGamutPreferences);
		for (int i = 0; i < count; i++) {
			ColorGamut colorGamut = (ColorGamut)colorGamutPreferences[i];
			if ((colorGamut >= ColorGamut_Native && colorGamut <= ColorGamut_DisplayP3) &&
				WVR()->SetColorGamut(colorGamut)) {
				LOGI(WVRRender, "Set display color gamut as: %d.", colorGamut);
				break;
			} else {
				LOGI(WVRRender, "Try set display color gamut: %d failed.", colorGamut);
			}
		}
#endif

		render_error = WVR()->RenderInit(&param);
		if (render_error != WVR_RenderError_None) {
			ReportRenderError(render_error);
			LOGI(WVRRender, "WVR Render Init Failed");
			return;
		}
		else {
			LOGI(WVRRender, "WVR Render Init Success");
		}

		WVR()->SetAppEditorVersion(AppEditorVersion::UE_426);

		WVR()->EnableTimeWarpStabilizedMode(mTimeWarpStabilizedMode);

		WVR()->SetFrameSharpnessEnhancementLevel(mFrameSharpnessEnhancementLevel);

		PrepareGLExtAPI();

		if (GIsGameThreadIdInitialized)
			// GameThread is the GL Context's main thread.  All texture create in GameThread.
			WVR()->SetRenderThreadId(GGameThreadId);

		{
			// Get default queue size.
			auto queue = WVR()->ObtainTextureQueue(isMultiViewEnabled ? WVR_TextureTarget_2D_ARRAY : WVR_TextureTarget_2D, WVR_TextureFormat_RGBA, WVR_TextureType_UnsignedByte, 256, 256, 0);
			defaultQueueSize = WVR()->GetTextureQueueLength(queue);
			WVR()->ReleaseTextureQueue(queue);

			check(defaultQueueSize);
		}

		LOGD(WVRRender, "AMCMode %d", (int)mAMCMode);
		WVR()->SetAMCMode(mAMCMode);
	}

	// Default
	WVR()->RenderFoveationMode(mCurrentFoveationMode);
	if (mCurrentFoveationMode == WVR_FoveationMode::WVR_FoveationMode_Enable) {
		SetFoveationParams(EStereoscopicPass::eSSP_LEFT_EYE, EnableModeFoveationParams[0]);
		SetFoveationParams(EStereoscopicPass::eSSP_RIGHT_EYE, EnableModeFoveationParams[1]);
	}
	WVR()->GetFoveationDefaultConfig(WVR_Eye::WVR_Eye_Left, &DefaultModeFoveationParams[0]);
	WVR()->GetFoveationDefaultConfig(WVR_Eye::WVR_Eye_Right, &DefaultModeFoveationParams[1]);
	isFoveatedRenderingSupported = WVR()->IsRenderFoveationSupport();
	isFoveatedRenderingEnabled = WVR()->IsRenderFoveationEnabled();

	WVR()->GetRenderTargetSize(&width, &height);  // We did this in HMD before.  Doesn't matter to do it again.
	LOGI(WVRRender, "WVR()->GetRenderTargetSize %u %u", width, height);

	// Because width and height is changed, need recalculate scaledWidth and scaledHeight.
	CalculatePixelDensityAndSize(pixelDensity);
	//pixelDensityChanged = true;  // First time, no need set this flag.

	// Initial TextureManager
	FWaveVRRenderTextureInfo info = {0};
	// common
	info.flags = ETextureCreateFlags(TexCreate_RenderTargetable | TexCreate_ShaderResource | (isSRGB ? TexCreate_SRGB : 0));
	info.numSamples = msaaLevel;
	info.format = pixelFormat;  // PF_B8G8R8A8

	info.width = width; // SingleEye Width
	info.height = height; // SingleEye Height

	if (isMultiViewEnabled)
	{
		info.arraySize = 2;
		info.target = GL_TEXTURE_2D_ARRAY;
	}
	else
	{
		info.arraySize = 1;
		info.target = /*msaaLevel > 1 ? GL_TEXTURE_2D_MULTISAMPLE :*/ GL_TEXTURE_2D; // MSAA need GL_TEXTURE_2D
	}

	info.useUnrealTextureQueue = useUnrealTextureQueue;
	info.createFromResource = createFromResource;

	info.wvrTextureQueue = nullptr;  // Created in CreateColorTexturePool.
	info.capacity = defaultQueueSize;

	info.pixelDensity = pixelDensity;
	info.scaledWidth = scaledWidth;
	info.scaledHeight = scaledHeight;

	int widthScaler = isMultiViewEnabled ? 1 : 2;
	info.renderWidth = scaledWidth * widthScaler;
	info.renderHeight = scaledHeight;

	mTextureManager.CreateTexturePool(info);

	bInitialized = true;
}


/** IStereoRenderTargetManager interface **/

bool FWaveVRRender::ShouldUseSeparateRenderTarget() const
{
	LOG_FUNC();
	return bInitialized;
}

void FWaveVRRender::UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, SViewport* ViewportWidget)
{
	LOG_FUNC();
	WVR_SCOPED_NAMED_EVENT(UpdateViewport, FColor::Purple);

	//LOGW(WVRRender, "UpdateViewport(Viewport.targetSizeXY=%u,%u, VW=%p)", Viewport.GetRenderTargetTextureSizeXY().X, Viewport.GetRenderTargetTextureSizeXY().Y, ViewportWidget);
//#if WITH_EDITOR
//	if (GIsEditor) goto end;
//#endif

	if (!bUseSeparateRenderTarget || !bInitialized) {
#if !WITH_EDITOR  // too noisy
		LOGW(WVRRender, "bUseSeparateRenderTarget=%d, bInitialized=%d",
			bUseSeparateRenderTarget, bInitialized);
#endif
		goto end;
	}

	if (needReAllocateRenderTargetTexture) {
		LOGD(WVRRender, "needReAllocateRenderTargetTexture, no next texture.");
		goto end;
	}

	// Can this line help resize?
	//ViewportWidget->SetRenderDirectlyToWindow(true);

	// Get next texture
	mTextureManager.Next(Viewport);

end:
	// Must not remove this line:  It call to GetActiveRenderBridge_GameThread and set bridget into the viewport.
	FXRRenderTargetManager::UpdateViewport(bUseSeparateRenderTarget, Viewport, ViewportWidget);
}

uint32 FWaveVRRender::GetNumberOfBufferedFrames() const {
	LOG_FUNC();
	return useUnrealTextureQueue ? defaultQueueSize : 1;
}

void FWaveVRRender::CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY)
{
	LOG_FUNC();
	if (PLATFORM_ANDROID) {
		InOutSizeX = isMultiViewEnabled ? GetSingleEyeScaledPixelWidth() : GetSingleEyeScaledPixelWidth() * 2;
		InOutSizeY = GetSingleEyeScaledPixelHeight();
	}
	else if (PLATFORM_WINDOWS || WITH_EDITOR) {
		InOutSizeX = GetSingleEyeScaledPixelWidth() * 2;
		InOutSizeY = GetSingleEyeScaledPixelHeight();
	}

	// This line try to get size of HMD->GetIdealRenderTargetSize() * HMD->GetPixelDensity().  We can answer it directly.
	//FXRRenderTargetManager::CalculateRenderTargetSize(Viewport, InOutSizeX, InOutSizeY);

	//LOGD(WVRRender, "CalculateRenderTargetSize w=%u, h=%u", InOutSizeX, InOutSizeY);
}

// This will run in game thread
bool FWaveVRRender::NeedReAllocateViewportRenderTarget(const class FViewport& Viewport)
{
	LOG_FUNC();
	WVR_SCOPED_NAMED_EVENT(NeedReAllocateViewportRenderTarget, FColor::Purple);

//#if WITH_EDITOR
//	if (GIsEditor) return false;
//#endif

	// This line will call CalculateRenderTargetSize to check PixelDensity.
	bool sizeNotMatch = FXRRenderTargetManager::NeedReAllocateViewportRenderTarget(Viewport);

	if ((needReAllocateRenderTargetTexture && bInitialized) || sizeNotMatch) {
		LOGD(WVRRender, "NeedReAllocateViewportRenderTarget return n=%d, i=%d, s=%d, pd=%f", needReAllocateRenderTargetTexture, bInitialized, sizeNotMatch, pixelDensity);
		return true;
	}
	else
	{
		return false;
	}
}

bool FWaveVRRender::NeedReAllocateDepthTexture(const TRefCountPtr<struct IPooledRenderTarget>& DepthTarget)
{
	//LOG_FUNC();
#if WITH_EDITOR
	if (GIsEditor) return false;
#endif
	WVR_SCOPED_NAMED_EVENT(NeedReAllocateDepthTexture, FColor::Purple);

	if (needReAllocateDepthTexture && bInitialized) {
		LOGD(WVRRender, "NeedReAllocateDepthTexture return n=%d, i=%d", needReAllocateDepthTexture, bInitialized);
		return true;
	}
	else
	{
		return false;
	}
}

class FXRRenderBridge* FWaveVRRender::GetActiveRenderBridge_GameThread(bool bUseSeparateRenderTarget)
{
	LOG_FUNC();
//#if WITH_EDITOR
//	if (GIsEditor) return nullptr;
//#endif

	check(IsInGameThread());

	if (bUseSeparateRenderTarget)
	{
		bAlreadySetCustomPresent = true;
		if (mCustomPresent == nullptr)
			mCustomPresent = new FWaveVRFXRRenderBridge(this);
		return mCustomPresent;
	}
	else
	{
		bAlreadySetCustomPresent = false;
		return nullptr;
	}
}


// Called by SceneViewport.cpp: FSceneViewport::InitDynamicRHI
// NumSamples will always be 1, and we need get it from console variable by ourself.
bool FWaveVRRender::AllocateRenderTargetTexture(
	uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags,
	ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
#if WITH_EDITOR
	if (GIsEditor) return false;
#endif

	WVR_SCOPED_NAMED_EVENT(AllocateRenderTargetTexture, FColor::Purple);

	//LOG_FUNC();
	LOGI(WVRRender, "AllocateRenderTargetTexture(index=%d width=%u height=%u mips=%u samples=%u flags=%u targetFlags=%u, format=%u)",
		Index, SizeX, SizeY, NumMips, NumSamples, Flags, TargetableTextureFlags, Format);

	if (!bInitialized) {
		LOGW(WVRRender, "Not initialized");
		return false;
	}

	int widthScalar = GetMultiView() ? 1 : 2;

	FWaveVRRenderTextureInfo info = {0};
	info.width = SizeX;
	info.height = SizeY;
	info.format = Format;
	info.mips = NumMips;
	info.flags = Flags;
	info.extraFlags = TargetableTextureFlags;
	info.numSamples = NumSamples;  // In WaveVRTexturPool, real created texture will follow msaaLevel.
	info.createFromResource = false;
	info.pixelDensity = pixelDensity;
	info.scaledWidth = scaledWidth;
	info.scaledHeight = scaledHeight;
	info.renderWidth = scaledWidth * widthScalar;
	info.renderHeight = scaledHeight;

	if (!needReAllocateRenderTargetTexture) {
		LOGI(WVRRender, "Unreal asked AllocateRenderTargetTexture()");
	} else {
		needReAllocateRenderTargetTexture = false;
		if (pixelDensityChanged) {
			pixelDensityChanged = false;
			mTextureManager.UpdatePixelDensity(info);
		}
	}

	auto ret = mTextureManager.AllocateColorTexture(info, Index);
	if (ret) {
		LOGI(WVRRender, "Color Texture[%d] Allocated", Index);
		OutTargetableTexture = OutShaderResourceTexture = ret;
	}

	return (ret != nullptr);
}

// May be not necessary
bool FWaveVRRender::AllocateDepthTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
#if WITH_EDITOR
	if (GIsEditor) return false;
#endif

	WVR_SCOPED_NAMED_EVENT(AllocateDepthTexture, FColor::Purple);

	//LOG_FUNC();
	LOGI(WVRRender, "AllocateDepthTexture(index=%d PD=%f width=%u height=%u mips=%u samples=%u flags=%u targetFlags=%u, format=%u)",
		Index, mHMD->GetPixelDenity(), SizeX, SizeY, NumMips, NumSamples, Flags, TargetableTextureFlags, Format);

	// Force not to generate depth texture.  Unreal will ask scene depth by this function.
	// However the depth texture generate here is only for non-multiview.
	if (NumSamples > 0)
		return false;

	if (!needReAllocateDepthTexture)
		LOGI(WVRRender, "Unreal asked AllocateDepthTexture()");

	if (!bInitialized) {
		LOGW(WVRRender, "Not initialized");
		return false;
	}

	// If developer didn't want triple depth buffer, we don't have to generate depth.
	// PMC will not be supported when triple depth buffer disabled.
	if (!isTripleDepthBufferEnabled) {
		LOGI(WVRRender, "TripleDepthBuffer is disabled");
		return false;
	}

	if (needReAllocateDepthTexture && bInitialized)
		needReAllocateDepthTexture = false;

	if (NumSamples > 1)
		return false;

	int widthScalar = GetMultiView() ? 1 : 2;

	if (!IsMatchPixelDensityScaledSize(SizeX / widthScalar, SizeY, pixelDensity, scaledWidth, scaledHeight)) {
		LOGW(WVRRender, "Not to generate depth texture because size not match");
		return false;
	}

	FWaveVRRenderTextureInfo info = {0};
	info.width = SizeX;
	info.height = SizeY;
	info.format = Format;
	info.mips = NumMips;
	info.flags = Flags;
	info.extraFlags = TargetableTextureFlags;
	info.numSamples = NumSamples;
	info.createFromResource = false;
	info.pixelDensity = pixelDensity;
	info.scaledWidth = scaledWidth;
	info.scaledHeight = scaledHeight;
	info.renderWidth = scaledWidth * widthScalar;
	info.renderHeight = scaledHeight;

	auto ret = mTextureManager.AllocateDepthTexture(info, Index);
	if (ret) {
		LOGI(WVRRender, "Depth Texture[%d]=%p Allocated", Index, ret->GetTexture2D());
		OutTargetableTexture = OutShaderResourceTexture = ret;
	}

	return ret.GetReference() != nullptr;
}

/* FWaveVRRender Get and Set */

void FWaveVRRender::SetSingleEyePixelSize(uint32 w, uint32 h)
{
	LOG_FUNC();
	if (width != w || height != h)
		isDirty = true;

	check(w != 0 && h != 0)

	LOGI(WVRRender, "SetSingleEyePixelSize(%u, %u)", w, h);
	width = w;
	height = h;
}

void FWaveVRRender::SetTextureFormat(EPixelFormat format)
{
	LOG_FUNC();
	if (pixelFormat != format)
		isDirty = true;
	pixelFormat = format;
}

void FWaveVRRender::SetMultiSampleLevel(int InMSAALevel)
{
	LOG_FUNC();
	if (msaaLevel != InMSAALevel)
		isDirty = true;
	msaaLevel = InMSAALevel;
}

void FWaveVRRender::SetMultiView(bool enable)
{
	LOG_FUNC();
	if (isMultiViewEnabled != enable)
		isDirty = true;
	isMultiViewEnabled = enable;
}

void FWaveVRRender::SetFoveationMode(WVR_FoveationMode Mode)
{
	LOG_FUNC();
	if (IsInRenderingThread()) {
		if (mCurrentFoveationMode != Mode && WVR()->RenderFoveationMode(Mode) == WVR_Result::WVR_Success){
			mCurrentFoveationMode = Mode;
			isFoveatedRenderingEnabled = WVR()->IsRenderFoveationEnabled();
		}
		LOGI(WVRRender, "Set FoveationMode(%d) Disable:0 Enable:1 Default:2 ", mCurrentFoveationMode);
	} else {
		FWaveVRRender * pRender = this;
		WVR_FoveationMode InMode = Mode;
		ENQUEUE_RENDER_COMMAND(SetFoveationMode) (
			[pRender, InMode](FRHICommandListImmediate& RHICmdList)
			{
				if (pRender->mCurrentFoveationMode != InMode && WVR()->RenderFoveationMode(InMode) == WVR_Result::WVR_Success){
					pRender->mCurrentFoveationMode = InMode;
					pRender->isFoveatedRenderingEnabled = WVR()->IsRenderFoveationEnabled();
				}
				LOGI(WVRRender, "Set FoveationMode(%d) Disable:0 Enable:1 Default:2 ", pRender->mCurrentFoveationMode);
			});
	}
}

void FWaveVRRender::SetFoveationParams(EStereoscopicPass Eye, const WVR_RenderFoveationParams_t& foveationParams)
{
	LOG_FUNC();
	if (!isFoveatedRenderingSupported)
		LOGD(WVRRender, "SetFoveationParams but Foveation is not supported on this device!");

	if (!isFoveatedRenderingEnabled)
		LOGD(WVRRender, "SetFoveationParams but Foveation is disabled now!");

	WVR_Eye WVREye = WVR_Eye::WVR_Eye_Left;
	if (Eye == EStereoscopicPass::eSSP_LEFT_EYE) {
		EnableModeFoveationParams[0] = foveationParams;
	}
	else {
		EnableModeFoveationParams[1] = foveationParams;
		WVREye = WVR_Eye::WVR_Eye_Right;
	}

	if (IsInRenderingThread()) {
		WVR()->SetFoveationConfig(WVREye, &foveationParams);
	} else {
		FWaveVRRender * pRender = this;
		WVR_Eye InWVREye = WVREye;
		ENQUEUE_RENDER_COMMAND(SetFoveationParams) (
			[pRender, InWVREye](FRHICommandListImmediate& RHICmdList)
			{
				bool bIndex = 0; //Left is 0
				if (InWVREye == WVR_Eye::WVR_Eye_Right)
					bIndex = 1;

				WVR()->SetFoveationConfig(InWVREye, &pRender->EnableModeFoveationParams[bIndex]);
			});
	}

	LOGI(WVRRender, "Set eye(%d) focalX (%f) focalY (%f) FOV(%f) periQuality(%d)",
		Eye, foveationParams.focalX, foveationParams.focalY,
		foveationParams.fovealFov, foveationParams.periQuality);
}

void FWaveVRRender::SetFrameSharpnessEnhancementLevel(float level)
{
	LOG_FUNC();

	if (level > 1.0) {
		LOGD(WVRRender, "The set level of SetFrameSharpnessEnhancementLevel is greater than maximum value 1.0, set it as 1.0");
		level = 1.0;
	} else if(level < 0.0) {
		LOGD(WVRRender, "The set level of SetFrameSharpnessEnhancementLevel is less than the minmum vlaue 0.0, set it as 0.0");
		level = 0.0;
	}
	mFrameSharpnessEnhancementLevel = level;

	if(bInitialized) {
		if(IsInRenderingThread()) {
			WVR()->SetFrameSharpnessEnhancementLevel(level);
		} else if (IsInGameThread()) {
			FWaveVRRender * pRender = this;
			float InLevel = level;
			ENQUEUE_RENDER_COMMAND(SetFrameSharpnessEnhancementLevel) (
				[pRender, InLevel](FRHICommandListImmediate& RHICmdList)
				{
					WVR()->SetFrameSharpnessEnhancementLevel(InLevel);
				});
		} else {
			LOGE(WVRRender, "Should check runting thread of SetFrameSharpnessEnhancementLevel.");
		}
	} else {
		LOGD(WVRRender, "SetFrameSharpnessEnhancementLevel. The level(%f) will be cached and take effect right after render init.", mFrameSharpnessEnhancementLevel);
	}
}

void FWaveVRRender::SetSubmitWithPose(bool enable, const WVR_PoseState_t * pose) {
	if (enable) {
		if (pose != nullptr) {
			bSubmitWithInteralPose = false;
			mPoseUsedOnSubmit = *pose;
		}
		else {
			bSubmitWithInteralPose = true;
		}
	}
	bSubmitWithPose = enable;
}

float FWaveVRRender::CalculatePixelDensityAndSize(float pd, int recursiveCount) {
	uint32 tempWidth = FMath::CeilToInt(width * pd);
	uint32 tempHeight = FMath::CeilToInt(height * pd);

	// For depth render texture, it require 4's complement
	scaledWidth = tempWidth + tempWidth % 4;  // 4's complement
	scaledHeight = tempHeight + tempHeight % 4;  // 4's complement
	LOGD(WVRRender, "CalculatePixelDensityAndSize(%f) sw=%u sh=%u nsw=%u nsh=%u", pd, tempWidth, tempHeight, scaledWidth, scaledHeight);

	// Hope it will finished before 100 times for all the case.  I have tested the pd=1.11111 on focuse plus, it need 10 times recursive to be finished.
	if (recursiveCount > 100)  
		return pd;

	if (tempWidth != scaledWidth || tempHeight != scaledHeight) {
		float UpdatedPixelDensity = FMath::Max(((float)scaledWidth) / width, ((float)scaledHeight) / height);
		return CalculatePixelDensityAndSize(UpdatedPixelDensity, ++recursiveCount);
	}
	return pd;
}

void FWaveVRRender::SetPixelDensity(float newPixelDensity, bool forceUpdate) {
	newPixelDensity = FMath::Clamp(newPixelDensity, 0.1f, 2.0f);

	// The texture need increase at least 32 pixels when size is 2048.
	bool almostEqual = FMath::IsNearlyEqual(newPixelDensity, pixelDensity, 32.0f / 2048);
	if (!forceUpdate && almostEqual) {
		LOGI(WVRRender, "SetPixelDensity() PD %f->%f, needReAllocate:%d", pixelDensity, newPixelDensity, needReAllocateRenderTargetTexture);
		return;
	}

	float oldPixelDensity = pixelDensity;
	pixelDensity = CalculatePixelDensityAndSize(newPixelDensity);

	if (bInitialized) {
		pixelDensityChanged = true;
		needReAllocateRenderTargetTexture = true; // must be true before manual create texture for new pixel density
		needReAllocateDepthTexture = true;
	}
	LOGI(WVRRender, "SetPixelDensity() PD %f->%f, needReAllocate:%d", oldPixelDensity, pixelDensity, needReAllocateRenderTargetTexture);
}

// do boolean calculation "(width * pd) == scaledW && (height * pd) == scaledH"
bool FWaveVRRender::IsMatchPixelDensityScaledSize(uint32_t width, uint32_t height, float pd, uint32_t scaledW, uint32_t scaledH) {
	uint32 tempWidth = FMath::CeilToInt(width * pd);
	uint32 tempHeight = FMath::CeilToInt(height * pd);

	// For depth render texture, it require 4's complement
	uint32 scaledWidth = tempWidth + tempWidth % 4;  // 4's complement
	uint32 scaledHeight = tempHeight + tempHeight % 4;  // 4's complement

	return (scaledWidth == scaledW) && (scaledHeight == scaledH);
}

int FWaveVRRender::GetMultiSampleLevel() const {
	return msaaLevel;
}

bool FWaveVRRender::GetMultiView() const {
	return isMultiViewEnabled;
}

bool FWaveVRRender::IsRenderFoveationSupport() const {
	LOG_FUNC();
	return isFoveatedRenderingSupported;
}

bool FWaveVRRender::IsRenderFoveationEnabled() const {
	LOG_FUNC();
	return isFoveatedRenderingEnabled;
}

void FWaveVRRender::GetFoveationParams(EStereoscopicPass Eye, WVR_RenderFoveationParams_t& foveationParams) const {
	LOG_FUNC();

	if(mCurrentFoveationMode == WVR_FoveationMode::WVR_FoveationMode_Default) {
		if (Eye == EStereoscopicPass::eSSP_LEFT_EYE) {
			foveationParams = DefaultModeFoveationParams[0];
		} else {
			foveationParams = DefaultModeFoveationParams[1];
		}
		LOGI(WVRRender, "Get DefaultModeConfig");
	} else {
		if (Eye == EStereoscopicPass::eSSP_LEFT_EYE) {
			foveationParams = EnableModeFoveationParams[0];
		} else {
			foveationParams = EnableModeFoveationParams[1];
		}
		LOGI(WVRRender, "Get EnableModeConfig");
	}

	LOGI(WVRRender, "Get eye(%d) focalX (%f) focalY (%f) FOV(%f) periQuality(%d)",
		Eye, foveationParams.focalX, foveationParams.focalY,
		foveationParams.fovealFov, foveationParams.periQuality);
}

void FWaveVRRender::GetSingleEyePixelSize(uint32 &w, uint32 &h) const {
	w = width;
	h = height;
}

uint32 FWaveVRRender::GetSingleEyePixelWidth() const {
	return width;
}

uint32 FWaveVRRender::GetSingleEyePixelHeight() const {
	return height;
}

void FWaveVRRender::GetSingleEyeScaledPixelSize(uint32 &w, uint32 &h) const {
	w = scaledWidth;
	h = scaledHeight;
}

uint32 FWaveVRRender::GetSingleEyeScaledPixelWidth() const {
	return scaledWidth;
}

uint32 FWaveVRRender::GetSingleEyeScaledPixelHeight() const {
	return scaledHeight;
}

EPixelFormat FWaveVRRender::GetTextureFormat() const {
	return pixelFormat;
}

float FWaveVRRender::GetPixelDensity() const {
	return pixelDensity;
}


void FWaveVRRender::Apply()
{
	LOG_FUNC();
	if (!isDirty)
		return;

	isDirty = false;

	// Wait NeedReAllocate
	needReAllocateRenderTargetTexture = true;
	//needReAllocateDepthTexture = true;
}

// Unreal FMatrix::M[raw][col]
static void DebugMatrix(const FMatrix& m) {
	LOGD(WVRRender,
			"/ %6f, %6f, %6f, %6f \\\n"
			"| %6f, %6f, %6f, %6f |\n"
			"| %6f, %6f, %6f, %6f |\n"
		   "\\ %6f, %6f, %6f, %6f /\n",
			m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3],
			m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3],
			m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
			m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]
		);
}

// Called by HMD
void FWaveVRRender::OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
	LOG_FUNC();
	Wave::RenderDoc::FRenderDoc::GetInstance().OnFrameBegin();

	// The command will send to RenderThread
	if (!bInitialized)
		RenderInit();

	const FSceneView& sceneViewLeft = ViewFamily.GetStereoEyeView(EStereoscopicPass::eSSP_LEFT_EYE);
	const FSceneView& sceneViewRight = ViewFamily.GetStereoEyeView(EStereoscopicPass::eSSP_RIGHT_EYE);

	//DebugMatrix(sceneViewLeft.ProjectionMatrixUnadjustedForRHI);
	//DebugMatrix(sceneViewRight.ProjectionMatrixUnadjustedForRHI);

	wvrProjections[0] = wvr::utils::ToWVRMatrix(sceneViewLeft.ProjectionMatrixUnadjustedForRHI);
	wvrProjections[1] = wvr::utils::ToWVRMatrix(sceneViewRight.ProjectionMatrixUnadjustedForRHI);

	WVR_TextureParams_t paramsL = mTextureManager.GetSubmitParams(WVR_Eye_Left);
	WVR()->PreRenderEye(WVR_Eye_Left, &paramsL);
	if (!isMultiViewEnabled)
	{
		WVR_TextureParams_t paramsR = mTextureManager.GetSubmitParams(WVR_Eye_Right);
		WVR()->PreRenderEye(WVR_Eye_Right, &paramsR);
	}
}

// Present
void FWaveVRRender::OnFinishRendering_RenderThread()
{
	LOG_FUNC();

	if (needReAllocateRenderTargetTexture) {
		LOGD(WVRRender, "needReAllocateRenderTargetTexture, not to submit.");
		return;
	}

	SubmitFrame_RenderThread();
}

// May be also invoked by WaveVRSplash.
void FWaveVRRender::SubmitFrame_RenderThread() {
	LOG_FUNC();
	WVR_SCOPED_NAMED_EVENT(SubmitFrame_RenderThread, FColor::Orange);
	// Submit

	//LOGV(WVRRender, "WVR_SubmitFrame(param.id=%p .target=%d)", params.id, params.target);

	auto posePtr = bSubmitWithPose ?
		(bSubmitWithInteralPose ? &(mHMD->FrameDataRT->poses.wvrPoses[0].pose) : &mPoseUsedOnSubmit) :
		nullptr;

	WVR_SubmitExtend submitExtendFlags = WVR_SubmitExtend_PartialTexture; // (WVR_SubmitExtend)(WVR_SubmitExtend_PartialTexture | WVR_SubmitExtend_SystemReserved1);
	if (isMultiViewEnabled) {
		WVR_TextureParams_t paramsL = mTextureManager.GetSubmitParams(WVR_Eye_Left);
		paramsL.projectionMatrix = wvrProjections;
		//LOGD(WVRRender, "SubmitFrame(e=%d, c=%d, d=%d)", 0, PTR_TO_INT(paramsL.id), PTR_TO_INT(paramsL.depth));
		WVR()->SubmitFrame(WVR_Eye_Left, &paramsL, posePtr, submitExtendFlags);
	} else {
		WVR_TextureParams_t paramsL = mTextureManager.GetSubmitParams(WVR_Eye_Left);
		WVR_TextureParams_t paramsR = mTextureManager.GetSubmitParams(WVR_Eye_Right);
		paramsL.projectionMatrix = wvrProjections;
		paramsR.projectionMatrix = wvrProjections + 1;
		//LOGD(WVRRender, "SubmitFrame(e=%d, c=%d, d=%d)", 0, PTR_TO_INT(paramsL.id), PTR_TO_INT(paramsL.depth));
		WVR()->SubmitFrame(WVR_Eye_Left, &paramsL, posePtr, submitExtendFlags);
		//LOGD(WVRRender, "SubmitFrame(e=%d, c=%d, d=%d)", 1, PTR_TO_INT(paramsR.id), PTR_TO_INT(paramsR.depth));
		WVR()->SubmitFrame(WVR_Eye_Right, &paramsR, posePtr, submitExtendFlags);
	}
#if PLATFORM_ANDROID
	if (glDebugMessageInsertExt != nullptr)
		glDebugMessageInsertExt(GL_DEBUG_SOURCE_APPLICATION_KHR, GL_DEBUG_TYPE_MARKER_KHR, 0, GL_DEBUG_SEVERITY_HIGH_KHR, -1, "vr-marker,frame_end,type,application");
#endif  // PLATFORM_ANDROID
}

void FWaveVRRender::OnResume()
{
	LOG_FUNC();
#if PLATFORM_ANDROID //For GNativeAndroidApp
	if (IsInRenderingThread()) {
		WVR()->SetATWActive(true, GNativeAndroidApp->window);
		WVR()->SetRenderThreadId(GGameThreadId);
	} else {
		FWaveVRRender * pRender = this;
		ENQUEUE_RENDER_COMMAND(OnResume) (
			[pRender](FRHICommandListImmediate& RHICmdList)
			{
				WVR()->SetATWActive(true, GNativeAndroidApp->window);
				WVR()->SetRenderThreadId(GGameThreadId);
			});
	}
#endif
}

void FWaveVRRender::OnPause()
{
	LOG_FUNC();
	//needReAllocateRenderTargetTexture = true;

	if (IsInRenderingThread()) {
		WVR()->PauseATW();
		// No, we don't release textures when suspend.  This will cause a resume problem.
		//mTextureManager.CleanTextures();
	} else {
		FWaveVRRender * pRender = this;
		ENQUEUE_RENDER_COMMAND(Reset)(
			[pRender](FRHICommandListImmediate& RHICmdList)
			{
				WVR()->PauseATW();
				// No, we don't release textures when suspend.  This will cause a resume problem.
				// mColorTexturePool will be null
				//mTextureManager.CleanTextures();
			});
	}
}

void FWaveVRRender::Shutdown()
{
	LOG_FUNC();
	bInitialized = false;
	needReAllocateRenderTargetTexture = true;

	FWaveVRRender * pRender = this;
	ENQUEUE_RENDER_COMMAND(Shutdown) (
		[pRender](FRHICommandListImmediate& RHICmdList)
		{
			WVR()->PauseATW();
			// No, we don't release textures when shutdown.  TextureManager will handle it.
			//mTextureManager.CleanTextures();
		});
}
