#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "wvr_render.h"
#include <list>

class FWaveVRFXRRenderBridge;
class FWaveVRRender;
class FWaveVRHMD;
class FViewport;

namespace Wave {
namespace Render {

class FWaveVRTexturePool;
struct FWaveVRRenderTextureInfo;

// Manager queue and submit
class FWaveVRTextureManager
{
private:
	friend class FWaveVRFXRRenderBridge;

public:
	FWaveVRTextureManager(FWaveVRRender * render, FWaveVRHMD * hmd);
	~FWaveVRTextureManager();

public:
	bool CreateTexturePool(const FWaveVRRenderTextureInfo& info);
	//FTexture2DRHIRef CreateDepthTexture(const FWaveVRRenderTextureInfo& info);

	// Get next texture from pool
	void Next(const FViewport& Viewport);

	void CleanTextures();

public:
	FTexture2DRHIRef AllocateColorTexture(const FWaveVRRenderTextureInfo& engineInfo, uint32 index);
	FTexture2DRHIRef AllocateDepthTexture(const FWaveVRRenderTextureInfo& engineInfo, uint32 index);

	WVR_Texture_t GetCurrentWVRTexture();
	void * GetCurrentTextureResource();
	WVR_TextureQueueHandle_t GetWVRTextureQueue();
	WVR_TextureParams_t GetSubmitParams(WVR_Eye eye);
	uint32 UpdatePixelDensity(const FWaveVRRenderTextureInfo& engineInfo);

	//private:
	//	inline FWaveVRTexturePool * GetColorTextureQueue() const { return mTexturePool; }
	//	inline FWaveVRTexturePool * GetDepthTextureQueue() const { return mDepthTextureQueue; }

private:
	FWaveVRTextureManager(const FWaveVRTextureManager &) = delete;
	FWaveVRTextureManager(FWaveVRTextureManager &&) = delete;
	FWaveVRTextureManager &operator=(const FWaveVRTextureManager &) = delete;

private:
	FWaveVRTexturePool * mTexturePool;
	std::list<FWaveVRTexturePool *> mTexturePools;
	std::list<FWaveVRTexturePool *> mTexturePoolRecycleBin;

	void * mCurrentResource;
	void * mCurrentResourceRT;
	void * mCurrentResourceDepth;
	void * mCurrentResourceDepthRT;
	uint32 mCurrentWVRTextureQueueIndex;  // Sometimes the wvr texture queue will return -1.  The useUnrealTextureQueue help handle it.

	WVR_TextureTarget wvrTextureTarget;

protected:
	FWaveVRRender * mRender;
	FWaveVRHMD * mHMD;
};

}  // namespace Render
}  // namespace Wave
