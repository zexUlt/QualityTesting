#pragma once

/****************************************************
 *
 * FWaveVRTexturePool should only be included in CPP
 *
 ****************************************************/

#include "CoreMinimal.h"
#include "RHI.h"
#include "RHIUtilities.h"
#include "OpenGLDrv.h"
#include "wvr_render.h"

namespace Wave {
namespace Render {

struct FWaveVRRenderTextureInfo
{
	// Single eye size
	uint32 width;
	uint32 height;
	uint32 arraySize;  // Multiview layer
	ETextureCreateFlags flags;
	ETextureCreateFlags extraFlags;
	uint8 mips;
	uint8 format;
	uint8 numSamples;
	uint8 capacity;
	GLenum target;
	float pixelDensity;
	// Single eye scaled size
	uint32 scaledWidth;
	uint32 scaledHeight;
	// Real texture size
	uint32 renderWidth;
	uint32 renderHeight;

	bool createFromResource;  // If true, use the texture in wvrTextureQueue
	bool useUnrealTextureQueue;  // If true, we don't need create a base texture
	bool useWVRTextureQueue;  // Decided by TextureManager
	WVR_TextureQueueHandle_t wvrTextureQueue;  // Created by TextureManager
};

class FTextureVariation {
private:
	bool isSet;

public:
	FTextureRHIRef textureRHI;
	FTexture2DRHIRef texture2DRHI;
	TRefCountPtr<FOpenGLTexture2D> glTexture2D;
	uint32_t glTextureId;

public:
	FTextureVariation(FOpenGLTexture2D * in);
	FTextureVariation(const FTextureVariation& copy);
	FTextureVariation();
	~FTextureVariation();

public:
	inline FOpenGLTexture2D *operator=(FOpenGLTexture2D * in) {
		textureRHI = in;
		texture2DRHI = in;
		glTexture2D = in;
		if (in != nullptr)
			glTextureId = in->GetResource();
		isSet = in != nullptr;
		return in;
	}

public:
	inline operator FTextureRHIRef() const {
		return textureRHI;
	}

	inline operator FTexture2DRHIRef() const {
		return texture2DRHI;
	}

	inline operator TRefCountPtr<FOpenGLTexture2D>() const {
		return glTexture2D;
	}

	inline operator FOpenGLTexture2D*() const {
		return glTexture2D.GetReference();
	}

	inline operator FOpenGLTextureBase*() const {
		return glTexture2D.GetReference();
	}

	inline operator uint32_t() const {
		return glTextureId;
	}

public:
	inline operator bool() const {
		return isSet;
	}

public:
	inline FTextureRHIRef TRHI() const {
		return textureRHI;
	}

	inline FTexture2DRHIRef T2DRHI() const {
		return texture2DRHI;
	}

	inline TRefCountPtr<FOpenGLTexture2D> GLT() const {
		return glTexture2D;
	}

	inline uint32_t GLR() const {
		return glTextureId;
	}

public:
	void SafeRelease();
};


class FWaveVRTexturePool
{
public:
	// Create base texture
	FWaveVRTexturePool(const FWaveVRRenderTextureInfo& nativeInfo);

	virtual ~FWaveVRTexturePool();
	void ReleaseTextures();

public:
	TRefCountPtr<FOpenGLTexture2D> CreateBaseColor();
	FOpenGLTexture2D * CreateBaseDepth(const FWaveVRRenderTextureInfo& engineInfo);
	void CreateColorInPool(uint32 index);
	void CreateDepthInPool(const FWaveVRRenderTextureInfo& engineInfo, uint32 index);

public:
	void MakeAlias(int index, GLuint resource = 0);
	uint32 MakeDepthAlias(int index, GLuint resource = 0);
	uint32 FindIndexByGLId(GLuint id) const;
	uint32 FindIndexByResource(void * resource) const;
	uint32 FindIndexByTexture(void * textureBaseRHI) const;
	void UpdatePixelDensityInPool(const FWaveVRRenderTextureInfo& engineInfo);

public:
	// Get
	inline const FWaveVRRenderTextureInfo GetInfo() const { return textureInfo; }
	const TRefCountPtr<FOpenGLTexture2D> GetGLTexture(uint32 index) const;
	FTexture2DRHIRef GetRHIColor(uint32 index) const;
	FTexture2DRHIRef GetRHIDepth(uint32 index) const;

private:
	FWaveVRTexturePool(const FWaveVRTexturePool &) = delete;
	FWaveVRTexturePool(FWaveVRTexturePool &&) = delete;
	FWaveVRTexturePool &operator=(const FWaveVRTexturePool &) = delete;

private:
	TRefCountPtr<FOpenGLTexture2D> mBaseColor;
	FTextureVariation mBaseDepth;

	FTextureRHIRef * mColorPool;
	FTextureVariation * mDepthPool;

	FWaveVRRenderTextureInfo textureInfo;
};


}  // namespace Render
}  // namespace Wave
