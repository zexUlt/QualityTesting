
#include "WaveVRTextureManager.h"
#include "WaveVRTexturePool.h"
#include "WaveVRRender.h"
#include "WaveVRHMD.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLOGWrapper.h"
#include "XRThreadUtils.h"
#include "UnrealClient.h"  // FViewport

namespace Wave {
namespace Render {

DEFINE_LOG_CATEGORY_STATIC(WVRRenderTM, Display, All);

#ifdef LogTextureInfo
#undef LogTextureInfo
#endif
#define LogTextureInfo(title, info) \
	LOGI(WVRRenderTM, title " TextureInfo width=%u height=%u PD=%f sWidth=%u sHeight=%u array=%u " \
		"rWidth=%u rHeight=%u samples=%u flags=%u extFlags=%u format=%u mips=%d " \
		"target=%d capacity=%u useUnrealTextureQueue=%d createFromResource=%d queue=%p", \
		info.width, info.height, info.pixelDensity, info.scaledWidth, info.scaledHeight,info.arraySize, \
		info.renderWidth, info.renderHeight, info.numSamples, info.flags, info.extraFlags, info.format, info.mips, \
		info.target, info.capacity, info.useUnrealTextureQueue, info.createFromResource, info.wvrTextureQueue);

#ifndef PTR_TO_INT
#if PLATFORM_ANDROID && PLATFORM_ANDROID_ARM //(32bit)
#define PTR_TO_INT(x) x
#define INT_TO_PTR(x) x
#else //PLATFORM_ANDROID_ARM64 || PLATFORM_WINDOWS || WITH_EDITOR  /*Other 64bit*/
#define PTR_TO_INT(x) \
  (int)(long long)(x)
#define INT_TO_PTR(x) \
  (long long)(x)
#endif
#endif

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
#define LOG_FUNC() LOGD(WVRRenderTM, "%s", WVR_FUNCTION_STRING) // ;FDebug::DumpStackTraceToLog();
#endif
#define LOG_STACKTRACE() FDebug::DumpStackTraceToLog()
#endif //PLATFORM_ANDROID


/****************************************************
 *
 * FWaveVRTextureManager
 *
 ****************************************************/


FWaveVRTextureManager::FWaveVRTextureManager(FWaveVRRender * render, FWaveVRHMD * hmd) :
	mTexturePool(nullptr),
	mCurrentResource(0),
	mCurrentResourceRT(0),
	mCurrentResourceDepth(0),
	mCurrentResourceDepthRT(0),
	mCurrentWVRTextureQueueIndex(0),
	mRender(render),
	mHMD(hmd)
{
	LOG_FUNC();
}

FWaveVRTextureManager::~FWaveVRTextureManager()
{
	LOG_FUNC();
	CleanTextures();
}

// Called by AllocateRenderTargetTexture
bool FWaveVRTextureManager::CreateTexturePool(const FWaveVRRenderTextureInfo& info)
{
	LOG_FUNC();
	LogTextureInfo("CreateColorTextures", info);

	check(!mTexturePool);

	// Init
	FWaveVRRenderTextureInfo infoCopy = info;

	if (info.arraySize > 1)
		wvrTextureTarget = WVR_TextureTarget_2D_ARRAY;  // Used when submit
	else
		wvrTextureTarget = WVR_TextureTarget_2D_DUAL;  // Used when submit

	if (info.useUnrealTextureQueue && info.createFromResource)
		infoCopy.useWVRTextureQueue = false;
	else if (!info.useUnrealTextureQueue && !info.createFromResource)
		infoCopy.useWVRTextureQueue = false;
	else
		infoCopy.useWVRTextureQueue = true;

	if (info.useUnrealTextureQueue && !info.createFromResource) {

	} else if (!info.useUnrealTextureQueue && info.createFromResource) {
		uint32 keyArray[3] = {0, 0, 0};
		LOGD(WVRRenderTM, "CreateTexturePool() case 2 textures:[%d] %p, [%d] %p, [%d] %p ", keyArray[0], &keyArray[0], keyArray[1], &keyArray[1], keyArray[2], &keyArray[2]);
		for (int i = 0; i < 3; i++) {
			glEnable(GL_TEXTURE_2D_ARRAY);
			glGenTextures(1, &keyArray[i]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, keyArray[i]);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, info.renderWidth, info.renderHeight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			//LOGD(WVRRenderTM, "CreateTexturePool() case 2 id:%d  ", keyArray[i]);
		}
		infoCopy.wvrTextureQueue = WVR()->StoreRenderTextures(keyArray, 3, /*WVR_Eye::WVR_Eye_Left*/false, wvrTextureTarget);
	} else {
		if (wvrTextureTarget == WVR_TextureTarget_2D_DUAL)
			infoCopy.wvrTextureQueue = WVR()->ObtainTextureQueue(WVR_TextureTarget_2D, WVR_TextureFormat_RGBA, WVR_TextureType_UnsignedByte, info.renderWidth, info.renderHeight, 0);
		else  // WVR_TextureTarget_2D_ARRAY
			infoCopy.wvrTextureQueue = WVR()->ObtainTextureQueue(WVR_TextureTarget_2D_ARRAY, WVR_TextureFormat_RGBA, WVR_TextureType_UnsignedByte, info.renderWidth, info.renderHeight, 0);
		LOGD(WVRRenderTM, "CreateTexturePool() case 1 ");
	}
	infoCopy.capacity = info.wvrTextureQueue ? WVR()->GetTextureQueueLength(info.wvrTextureQueue) : infoCopy.capacity;

	// Create pool
	mTexturePool = new FWaveVRTexturePool(infoCopy);

	if (!infoCopy.useUnrealTextureQueue) {
		if (infoCopy.createFromResource) {
			mTexturePool->CreateBaseColor();
		} else {
			mTexturePool->CreateBaseColor();
		}
	} else {
		// If useUnrealTextureQueue, no need to create a base texture, and each texture will be allocated during AllocateColorTexture.
	}

	// BaseDepth will be created when unreal invoke AllocateDepthTexture
	//mTexturePool->CreateBaseDepth();

	return true;
}

// Called by Render::AllocateRenderTargetTexture
FTexture2DRHIRef FWaveVRTextureManager::AllocateColorTexture(const FWaveVRRenderTextureInfo& engineInfo, uint32 index)
{
	LOG_FUNC();
	if (!mTexturePool) {
		LOGD(WVRRenderTM, "Without TexturePool");
		return nullptr;
	}

	auto textureInfo = mTexturePool->GetInfo();

	// check if we should create the texture for engine
	bool match =
		((textureInfo.width == engineInfo.renderWidth && textureInfo.height == engineInfo.renderHeight) ||
		(engineInfo.width == engineInfo.renderWidth && engineInfo.height == engineInfo.renderHeight)) &&
		textureInfo.mips == textureInfo.mips;

	if (!match) {
		LOGD(WVRRenderTM, "The engineInfo not match our requirement.");
		LogTextureInfo("AllocateColorTexture", engineInfo);
		return nullptr;
	}

	mTexturePool->CreateColorInPool(index);

	return mTexturePool->GetRHIColor(index);
}

// Called by Render::AllocateDepthTexture
FTexture2DRHIRef FWaveVRTextureManager::AllocateDepthTexture(const FWaveVRRenderTextureInfo& engineInfo, uint32 index)
{
	LOG_FUNC();

	if (mTexturePool == nullptr)
	{
		LOGW(WVRRenderTM, "Pool not initialized.");
		return nullptr;
	}

	auto textureInfo = mTexturePool->GetInfo();

	// check if we should create the texture for engine
	bool match = (textureInfo.width == engineInfo.renderWidth && textureInfo.height == engineInfo.renderHeight) ||
		(engineInfo.width == engineInfo.renderWidth && engineInfo.height == engineInfo.renderHeight);

	LogTextureInfo("AllocateDepthTexture", engineInfo);
	if (!match) {
		LogTextureInfo("DesiredTexture", textureInfo);
		LOGD(WVRRenderTM, "The engineInfo not match our requirement.");
		return nullptr;
	}

	// Unreal always invoke this for only one depth texture.  Use aliased texture.
	mTexturePool->CreateDepthInPool(engineInfo, index);
	return mTexturePool->GetRHIDepth(index);
}

// Can be invoked in both thread.
void FWaveVRTextureManager::Next(const FViewport& Viewport)
{
	WVR_SCOPED_NAMED_EVENT(TextMngrNext, FColor::Purple);

	LOG_FUNC();
	bool isInRenderingThread = IsInRenderingThread();

	if (!mTexturePool) {
		LOGD(WVRRenderTM, "mTexturePool is nullptr");
		return;
	}

	auto info = mTexturePool->GetInfo();
	auto renderTarget = Viewport.GetRenderTargetTexture();
	check(renderTarget);

	if (PLATFORM_WINDOWS) {
		//if (!info.wvrTextureQueue || !info.useWVRTextureQueue)  // Always true in windows?
		mCurrentResource = renderTarget->GetNativeResource();
	} else if (PLATFORM_ANDROID) {
		int idx = -1;
		if (!info.wvrTextureQueue || !info.useWVRTextureQueue) {
			// Without texture queue, find the queue our self.
			GLuint id = ((FOpenGLTextureBase*)renderTarget->GetTextureBaseRHI())->GetResource();
			mCurrentResource = (void *)INT_TO_PTR(id);
			idx = mTexturePool->FindIndexByGLId(id);
			idx = (++idx) % info.capacity;
			mTexturePool->MakeAlias(idx);

			mCurrentResourceDepth = (void *)INT_TO_PTR(mTexturePool->MakeDepthAlias(idx));
			//LOGV(WVRRenderTM, "thread(%d) wvrTextureQueue=%d index=%d, mCurrentResource=%u", isInRenderingThread ? 1 : 0, !!info.wvrTextureQueue, idx, id);
			//LOGV(WVRRenderTM, "next() 1 texture index = %d", idx);
		} else {
			idx = WVR()->GetAvailableTextureIndex(info.wvrTextureQueue);
			//LOGV(WVRRenderTM, "next() 2 texture index = %d", idx);

			// No available texture!
			if (idx < 0) {
				LOGE(WVRRenderTM, "No available texture %d.  Should fix it.", idx);
				mCurrentWVRTextureQueueIndex = ++mCurrentWVRTextureQueueIndex % info.capacity;
				idx = mCurrentWVRTextureQueueIndex;
			} else {
				mCurrentWVRTextureQueueIndex = idx;
			}
			checkf(idx >= 0 && idx < info.capacity, TEXT("RenderTexture=%d is out of range."), idx);

			WVR_TextureParams_t params = WVR()->GetTexture(info.wvrTextureQueue, idx);
			mCurrentResource = params.id;

			//TRefCountPtr<FOpenGLTexture2D> openglRenderTarget = static_cast<FOpenGLTexture2D*>();
			//check(openglRenderTarget);
			GLuint id = (GLuint)PTR_TO_INT(mCurrentResource);
			uint32 foundIdx = mTexturePool->FindIndexByGLId(id);
			if (foundIdx != idx) {
				mTexturePool->MakeAlias(foundIdx, id);
				mCurrentResourceDepth = (void *)INT_TO_PTR(mTexturePool->MakeDepthAlias(foundIdx));
			} else {
				mTexturePool->MakeAlias(idx);
				mCurrentResourceDepth = (void *)INT_TO_PTR(mTexturePool->MakeDepthAlias(idx));
			}

			//LOGV(WVRRenderTM, "thread(%d) index=%d, mCurrentResource=%u", isInRenderingThread ? 1 : 0, idx, mCurrentResource);
		}
	}

	if (isInRenderingThread) {
		mCurrentResourceRT = mCurrentResource;
		mCurrentResourceDepthRT = mCurrentResourceDepth;
	} else {
		void * ResourceToRT = mCurrentResource;
		void * ResourceDepthToRT = mCurrentResourceDepth;
		ExecuteOnRenderThread_DoNotWait([this, ResourceToRT, ResourceDepthToRT](FRHICommandListImmediate& RHICmdList) {
			mCurrentResourceRT = ResourceToRT;
			mCurrentResourceDepthRT = ResourceDepthToRT;
		});
	}
}

void* FWaveVRTextureManager::GetCurrentTextureResource()
{
	LOG_FUNC();
	return mCurrentResource;
}

/* unused */
WVR_Texture_t FWaveVRTextureManager::GetCurrentWVRTexture()
{
	LOG_FUNC();
	return (WVR_Texture_t)mCurrentResource;
}

/* unused */
WVR_TextureQueueHandle_t FWaveVRTextureManager::GetWVRTextureQueue() {
	LOG_FUNC();
	if (!mTexturePool) return nullptr;
	auto info = mTexturePool->GetInfo();
	return info.wvrTextureQueue;
}


WVR_TextureParams_t FWaveVRTextureManager::GetSubmitParams(WVR_Eye eye) {
	LOG_FUNC();
	WVR_TextureParams_t params = {0};
	if (!mTexturePool) return params;
	auto info = mTexturePool->GetInfo();

	// Submit
	params.id = mCurrentResourceRT;
	params.depth = mCurrentResourceDepthRT;

	if (wvrTextureTarget == WVR_TextureTarget_2D_DUAL)
	{
		params.target = wvrTextureTarget;
		float eyeOffset = eye == WVR_Eye_Left ? 0 : 0.5f;
		params.layout.leftLowUVs.v[0] = 0 + eyeOffset;
		params.layout.leftLowUVs.v[1] = 0;
		params.layout.rightUpUVs.v[0] = 0.5f + eyeOffset;
		params.layout.rightUpUVs.v[1] = 1;
	}
	else
	{
		params.target = wvrTextureTarget;
		params.layout.leftLowUVs.v[0] = 0;
		params.layout.leftLowUVs.v[1] = 0;
		params.layout.rightUpUVs.v[0] = 1;
		params.layout.rightUpUVs.v[1] = 1;
	}

	return params;
}

uint32 FWaveVRTextureManager::UpdatePixelDensity(const FWaveVRRenderTextureInfo& engineInfo) {
	LOG_FUNC();
	if (!mTexturePool) {
		LOGD(WVRRenderTM, "UpdatePixelDensity(engineInfo) mTexturePool is nullptr");
		LogTextureInfo("UpdatePixelDensityTM", engineInfo);
		return 0;
	}
	mTexturePool->UpdatePixelDensityInPool(engineInfo);

	//auto info = mTexturePool->GetInfo();
	//LogTextureInfo("UpdatePixelDensity(**check after update global)", info);
	//if (GSupportsMobileMultiView && info.useUnrealTextureQueue && info.createFromResource) {
	//	mRender->SubmitFrame_RenderThread();
	//}
	return 0;
}

void FWaveVRTextureManager::CleanTextures()
{
	LOG_FUNC();
	if (mTexturePool != nullptr) {
		mTexturePool->ReleaseTextures();
		delete mTexturePool;
	}
	mTexturePool = nullptr;
}

}  // namespace Render
}  // namespace Wave
