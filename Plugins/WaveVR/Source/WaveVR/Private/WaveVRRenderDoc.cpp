#include "WaveVRRenderDoc.h"

// C
#if PLATFORM_ANDROID
#include <dlfcn.h>
#endif

#include "Platforms/WaveVRLogWrapper.h"

#include "renderdoc_app.h"

DEFINE_LOG_CATEGORY_STATIC(WVRRenderDoc, Display, All);

namespace Wave {
namespace RenderDoc {

// If you have a rooted device, you can modify this line in the libUE4.so of your APK to force trigger some captures since the app started.  You don't have to rebuild your APK, and just push a modifed libUE4.so and libVkLayer_GLES_RenderDoc.so into the <yourPackage>/lib/.
static const char * sCommandLine = "RenderDocCommandLine Capture 00 00000 AutoCapture 00 00000 000000 ,,";

FRenderDoc::FRenderDoc()
	: libraryHandle(nullptr)
	, apiPtr(nullptr)
	, IsCapturing(false)
	, NeedCapture(false)
	, CaptureFrameCount(0)
	, NeedAutoCapture(false)
	, AutoCaptureFrameCount(0)
	, AutoCapturePeriod(0)
{
	LOGD(WVRRenderDoc, "FRenderDoc()");
	LoadLibrary();
	if (apiPtr == nullptr)
		return;

	RENDERDOC_API_1_4_1 * rdoc_api = (RENDERDOC_API_1_4_1 *)apiPtr;
	rdoc_api->UnloadCrashHandler();

	char cmdlineStr[30] = {0};
	char captureStr[30] = {0};
	char autoCaptureStr[30] = {0};
	int32_t cfc = 0, delay = 0, acfc = 0, delayAuto = 0, acp = 0;
	int ret = 0;

#if PLATFORM_ANDROID
	// Android didn't have sscanf_s.  sscanf %d is 32bits.
	ret = sscanf(sCommandLine, "%20s %7s %2d %5d %11s %2d %5d %6d ,,",
		cmdlineStr, /*(unsigned)sizeof(cmdlineStr),*/
		captureStr, /*(unsigned)sizeof(captureStr),*/
		&cfc, &delay,
		autoCaptureStr, /*(unsigned)sizeof(autoCaptureStr),*/
		&acfc, &delayAuto, &acp);
#endif
	cmdlineStr[20] = 0;
	captureStr[7] = 0;
	autoCaptureStr[11] = 0;
	CaptureFrameCount = cfc;
	AutoCaptureFrameCount = acfc;
	AutoCapturePeriod = acp;

	LOGD(WVRRenderDoc, "%s, %s, %d, %d, %s, %d, %d, %d, ret=%d", cmdlineStr, captureStr, CaptureFrameCount, delay, autoCaptureStr, AutoCaptureFrameCount, delayAuto, AutoCapturePeriod, ret);
	if (ret != 8)
		return;
	CaptureFrame(CaptureFrameCount, delay);
	SetAutoCapture(AutoCaptureFrameCount, delayAuto, AutoCapturePeriod);
}

FRenderDoc::~FRenderDoc() {
	UnloadLibrary();
}

// Yes, launch RenderDoc in static.
FRenderDoc * FRenderDoc::instance = new FRenderDoc();

void FRenderDoc::StartFrameCapture() {
	LOGD(WVRRenderDoc, "StartFrameCapture()");
	RENDERDOC_API_1_4_1 * rdoc_api = (RENDERDOC_API_1_4_1 *)apiPtr;
	rdoc_api->StartFrameCapture(nullptr, nullptr);
}

void FRenderDoc::EndFrameCapture() {
	RENDERDOC_API_1_4_1 * rdoc_api = (RENDERDOC_API_1_4_1 *)apiPtr;
	rdoc_api->EndFrameCapture(nullptr, nullptr);
	LOGD(WVRRenderDoc, "EndFrameCapture()");
}

bool FRenderDoc::IsLibraryLoaded() const {
	return apiPtr != nullptr;
}

void FRenderDoc::UnloadLibrary() {
	LOGD(WVRRenderDoc, "UnloadLibrary");
	apiPtr = nullptr;
#if PLATFORM_ANDROID
	if (libraryHandle)
		dlclose(libraryHandle);
#endif
	libraryHandle = nullptr;
	// log unloaded.
}

void FRenderDoc::LoadLibrary() {
	LOGD(WVRRenderDoc, "LoadLibrary");
	if (libraryHandle != nullptr)
		return;
#if PLATFORM_ANDROID
	libraryHandle = dlopen("libVkLayer_GLES_RenderDoc.so", RTLD_NOW | RTLD_NOLOAD);
#endif
	if (libraryHandle == nullptr) {
#if PLATFORM_ANDROID
		LOGW(WVRRenderDoc, "RenderDoc's library is not found.");
#endif
		return;
	}

	pRENDERDOC_GetAPI RENDERDOC_GetAPI = nullptr;
#if PLATFORM_ANDROID
	RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(libraryHandle, "RENDERDOC_GetAPI");
#endif

	if (RENDERDOC_GetAPI == nullptr) {
		LOGE(WVRRenderDoc, "RenderDoc's library is not validate.");
		UnloadLibrary();
		return;
	}

	int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, &apiPtr);
	if (ret != 1) {
		LOGE(WVRRenderDoc, "RenderDoc's API version is not validate.");
		UnloadLibrary();
		return;
	}
	LOGD(WVRRenderDoc, "Loaded");
}

void FRenderDoc::CaptureFrame(int frameCount, int delayInMs) {
	LOGD(WVRRenderDoc, "CaptureFrame(fc=%d, d=%d)", frameCount, delayInMs);

	if (frameCount <= 0) {
		NeedCapture = false;
		return;
	}
	if (frameCount > 10)
		frameCount = 10;

	if (delayInMs < 0)
		delayInMs = 0;
	// Wait to long
	if (delayInMs > 60000) // 1 min
		return;

	NextTime = FDateTime::Now() + FTimespan::FromMilliseconds(delayInMs);
	NeedCapture = true;
	CaptureFrameCount = frameCount;
}

void FRenderDoc::SetAutoCapture(int frameCount, int periodInMs, int delayInMs) {
	LOGD(WVRRenderDoc, "SetAutoCapture(fc=%d, p=%d, d=%d)", frameCount, periodInMs, delayInMs);

	if (frameCount <= 0) {
		NeedAutoCapture = false;
		return;
	}
	if (frameCount > 10)
		frameCount = 10;

	if (delayInMs < 0)
		delayInMs = 0;
	// Wait to long
	if (delayInMs > 60000) // 1 min
		return;

	if (periodInMs < 3000) // 3 second
		periodInMs = 3000;
	// Wait to long
	if (periodInMs > 600000) // 10 min
		return;

	NextTimeAuto = FDateTime::Now() + FTimespan::FromMilliseconds(delayInMs);
	NeedAutoCapture = true;
	AutoCaptureFrameCount = frameCount;
	AutoCapturePeriod = periodInMs;
}

// Called by HMD
void FRenderDoc::OnFrameBegin() {
	if (apiPtr == nullptr)
		return;

	bool captureTimeUp = false;
	bool autoCaptureTimeUp = false;

	if (NeedCapture || NeedAutoCapture) {
		auto now = FDateTime::Now();
		if (NeedCapture && now >= NextTime) {
			captureTimeUp = true;
			NeedCapture = false;
		}

		if (NeedAutoCapture && now >= NextTimeAuto) {
			autoCaptureTimeUp = true;
			NextTimeAuto = now + FTimespan::FromMilliseconds(AutoCapturePeriod);
		}
	}

	if (!captureTimeUp && !autoCaptureTimeUp && !IsCapturing)
		return;

	LOGD(WVRRenderDoc, "captureTimeUp=%d, autoCaptureTimeUp=%d, IsCapturing=%d", captureTimeUp, autoCaptureTimeUp, IsCapturing);

	if (!IsCapturing) {
		if (captureTimeUp && autoCaptureTimeUp) {
			TargetFrameCount = CaptureFrameCount > AutoCaptureFrameCount ? CaptureFrameCount : AutoCaptureFrameCount;
			IsCapturing = true;
			StartFrameCapture();
		} else if (captureTimeUp) {
			TargetFrameCount = CaptureFrameCount;
			IsCapturing = true;
			StartFrameCapture();
		} else if (autoCaptureTimeUp) {
			TargetFrameCount = AutoCaptureFrameCount;
			IsCapturing = true;
			StartFrameCapture();
		}
		return;
	}

	if (--TargetFrameCount <= 0) {
		IsCapturing = false;
		EndFrameCapture();
	}
}

}  // namespace RenderDoc
}  // namespace Wave


UWaveVRRenderDoc::UWaveVRRenderDoc(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWaveVRRenderDoc::CaptureFrame(int frameCount, int delayInMs) {
	Wave::RenderDoc::FRenderDoc& rd = Wave::RenderDoc::FRenderDoc::GetInstance();
	if (!rd.IsLibraryLoaded())
		return;

	rd.CaptureFrame(frameCount, delayInMs);
}

void UWaveVRRenderDoc::SetAutoCapture(int frameCount, int periodInMs, int delayInMs) {
	Wave::RenderDoc::FRenderDoc& rd = Wave::RenderDoc::FRenderDoc::GetInstance();
	if (!rd.IsLibraryLoaded())
		return;
	rd.SetAutoCapture(frameCount, periodInMs, delayInMs);
}

bool UWaveVRRenderDoc::Available() {
	Wave::RenderDoc::FRenderDoc& rd = Wave::RenderDoc::FRenderDoc::GetInstance();
	return rd.IsLibraryLoaded();
}
