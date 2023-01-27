
#include "WaveVRTexturePool.h"
#include "WaveVRRender.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLOGWrapper.h"

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

//#if PLATFORM_ANDROID
//#define WAVEVR_LOG_SHOW_ALL_ENTRY 1
//#endif

#if defined(WAVEVR_LOG_SHOW_ALL_ENTRY) && (PLATFORM_ANDROID || PLATFORM_WINDOWS)
#if WAVEVR_LOG_SHOW_ALL_ENTRY && !WITH_EDITOR
#ifdef LOG_FUNC
#undef LOG_FUNC
#endif
#define LOG_FUNC() LOGD(WVRRenderTP, "%s", WVR_FUNCTION_STRING) // ;FDebug::DumpStackTraceToLog();
#endif
#define LOG_STACKTRACE() FDebug::DumpStackTraceToLog()
#endif //PLATFORM_ANDROID


#ifdef WVR_SCOPED_NAMED_EVENT
#undef WVR_SCOPED_NAMED_EVENT
#endif
#if (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG) && !WITH_EDITOR
#define WVR_SCOPED_NAMED_EVENT(name, color) SCOPED_NAMED_EVENT(name, color)
#else
#define WVR_SCOPED_NAMED_EVENT(name, color)
#endif

namespace Wave {
namespace Render {

DEFINE_LOG_CATEGORY_STATIC(WVRRenderTP, Display, All);

#ifdef LogTextureInfo
#undef LogTextureInfo
#endif
#define LogTextureInfo(title, info) \
	LOGI(WVRRenderTP, title " TextureInfo width=%u height=%u PD=%f sWidth=%u sHeight=%u array=%u " \
		"rWidth=%u rHeight=%u samples=%u flags=%u extFlags=%u format=%u mips=%d " \
		"target=%d capacity=%u useUnrealTextureQueue=%d createFromResource=%d queue=%p", \
		info.width, info.height, info.pixelDensity, info.scaledWidth, info.scaledHeight,info.arraySize, \
		info.renderWidth, info.renderHeight, info.numSamples, info.flags, info.extraFlags, info.format, info.mips, \
		info.target, info.capacity, info.useUnrealTextureQueue, info.createFromResource, info.wvrTextureQueue);


FTextureVariation::FTextureVariation(FOpenGLTexture2D * in)
	: isSet(false)
	, textureRHI()
	, texture2DRHI()
	, glTexture2D()
	, glTextureId(0)
{
	if (in == nullptr)
		return;
	textureRHI = in;
	texture2DRHI = in;
	glTexture2D = in;
	glTextureId = in->GetResource();
	isSet = true;
}

FTextureVariation::FTextureVariation(const FTextureVariation& copy)
	: isSet(copy.isSet)
	, textureRHI(copy.glTexture2D)
	, texture2DRHI(copy.glTexture2D)
	, glTexture2D(copy.glTexture2D)
	, glTextureId(copy.glTextureId)
{
}

FTextureVariation::FTextureVariation()
	: isSet(false)
	, textureRHI()
	, texture2DRHI()
	, glTexture2D()
	, glTextureId(0)
{
}

FTextureVariation::~FTextureVariation()
{
	glTextureId = 0;
	textureRHI.SafeRelease();
	texture2DRHI.SafeRelease();
	glTexture2D.SafeRelease();
	isSet = false;
}

void FTextureVariation::SafeRelease() {
	glTextureId = 0;
	textureRHI.SafeRelease();
	texture2DRHI.SafeRelease();
	glTexture2D.SafeRelease();
	isSet = false;
}

void FWaveVRTexturePool::UpdatePixelDensityInPool(const FWaveVRRenderTextureInfo& engineInfo)
{
	WVR_SCOPED_NAMED_EVENT(UpdatePixelDensityInPool, FColor::Orange);
	LOG_FUNC();
	LogTextureInfo("UpdatePixelDensityInPool", engineInfo);

	textureInfo.pixelDensity = engineInfo.pixelDensity;
	textureInfo.scaledWidth = engineInfo.scaledWidth;
	textureInfo.scaledHeight = engineInfo.scaledHeight;
	textureInfo.renderWidth = engineInfo.renderWidth;
	textureInfo.renderHeight = engineInfo.renderHeight;

	if (textureInfo.useUnrealTextureQueue && textureInfo.createFromResource && textureInfo.wvrTextureQueue) {
		// No base color texture, use native created resource from texture queue.
		if (mColorPool != nullptr) {
			for (int i = 0; i < textureInfo.capacity; i++) {
				// When Refs count to zero, the texture's destructor will call Release() itself.  We don't have to call Release().
				//mColorPool[i]->Release();
				LOGD(WVRRenderTP, "SafeRelease mColorPool[%d].GetRefCount()=%d", i, mColorPool[i].GetRefCount());
				mColorPool[i].SafeRelease();
			}
			//LOGD(WVRRenderTP, "delete[] mColorPool");
			delete[] mColorPool;
			mColorPool = nullptr;
		}

		if (textureInfo.wvrTextureQueue != nullptr)
		{
			//LOGD(WVRRenderTP, "WVR_ReleaseTextureQueue()");
			WVR()->ReleaseTextureQueue(textureInfo.wvrTextureQueue);
			textureInfo.wvrTextureQueue = nullptr;
		}

		// Create new queue
		mColorPool = new FTextureRHIRef[textureInfo.capacity];

		WVR_TextureTarget wvrTextureTarget;
		if (textureInfo.arraySize > 1)
			wvrTextureTarget = WVR_TextureTarget_2D_ARRAY;
		else
			wvrTextureTarget = WVR_TextureTarget_2D;
		textureInfo.wvrTextureQueue = WVR()->ObtainTextureQueue(wvrTextureTarget, WVR_TextureFormat_RGBA, WVR_TextureType_UnsignedByte, textureInfo.renderWidth, textureInfo.renderHeight, 0);
		//GEngine->ForceGarbageCollection();
	}

	if (!textureInfo.useUnrealTextureQueue) {
		LOGD(WVRRenderTP, "update the base texture");
		CreateBaseColor();
	} else {
		// If useUnrealTextureQueue, no need to create a base texture, and each texture will be allocated during AllocateColorTexture.
	}
}

FWaveVRTexturePool::FWaveVRTexturePool(const FWaveVRRenderTextureInfo& info)
	: mBaseColor(nullptr)
	, mBaseDepth()
	, mColorPool(nullptr)
	, mDepthPool(nullptr)
	, textureInfo(info)
{
	LOG_FUNC();
	LogTextureInfo("FWaveVRTexturePool", info);

	// This is a pool.
	check(textureInfo.capacity);

	mColorPool = new FTextureRHIRef[textureInfo.capacity];
	for (int i = 0; i < textureInfo.capacity; i++)
		mColorPool[i] = nullptr;

	mDepthPool = new FTextureVariation[textureInfo.capacity];
	for (int i = 0; i < textureInfo.capacity; i++)
		mDepthPool[i] = nullptr;
}

FWaveVRTexturePool::~FWaveVRTexturePool()
{
	LOG_FUNC();
	ReleaseTextures();
}

// Used with old unreal WVR texture queue.  Not follow WVR's texture queue
TRefCountPtr<FOpenGLTexture2D> FWaveVRTexturePool::CreateBaseColor()
{
	LOG_FUNC();

	check(!textureInfo.useUnrealTextureQueue);
	auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
	check(GLRHI);
	const auto& info = textureInfo;

	// Create base texture which will give to SceneViewport of UnrealEngine.
	//if (!info.useUnrealTextureQueue)
	mBaseColor = new FOpenGLTexture2D(GLRHI, 0, info.target, GL_NONE/*GL_COLOR_ATTACHMENT0*/, info.renderWidth, info.renderHeight, 0, 1, 1, info.numSamples, info.arraySize, EPixelFormat(info.format), false, false, info.flags, FClearValueBinding::Black);
	mBaseColor->SetAliased(true);
	OpenGLTextureAllocated(mBaseColor, info.flags);  // Do what is done in InitializeGLTexture

	LOGD(WVRRenderTP, "mBaseColor tid=%u", mBaseColor->GetResource());

	return mBaseColor;
}

FOpenGLTexture2D * FWaveVRTexturePool::CreateBaseDepth(const FWaveVRRenderTextureInfo& engineInfo)
{
	LOG_FUNC();

	auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
	check(GLRHI);
#if 0
	// XXX Assert: Ensure condition failed : (DepthStencilRenderTarget.DepthStencilTarget->GetFlags() & TexCreate_InputAttachmentRead) != 0
	uint32_t flags = engineInfo.flags | engineInfo.extraFlags;
	// Not support arraySize
	return GLRHI->CreateOpenGLRHITextureOnly(engineInfo.renderWidth, engineInfo.renderHeight, false, engineInfo.arraySize > 1, false, engineInfo.format, 1, 1, engineInfo.arraySize, flags, FClearValueBinding::DepthFar);
#else
	int arraySize = engineInfo.arraySize;
	bool isArray = arraySize > 1;

	// target is not GL_TEXTURE_2D_MULTISAMPLE.  Because NumSamplesTileMem is not 1.
	GLenum target = isArray ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY;
	ETextureCreateFlags flags = ETextureCreateFlags(engineInfo.flags | engineInfo.extraFlags);
	FOpenGLTexture2D * depth = new FOpenGLTexture2D(GLRHI, 0, target, GL_DEPTH_ATTACHMENT, engineInfo.renderWidth, engineInfo.renderHeight, 0, 1, 1, engineInfo.numSamples/*NumSampleTileMem*/, arraySize, PF_X24_G8/*used for alias*/, false, false, flags, FClearValueBinding::DepthFar);
	depth->SetAliased(true);
	OpenGLTextureAllocated(depth, flags);
#endif

	LOGD(WVRRenderTP, "mBaseDepth tid=%u", depth->GetResource());  // Must be 0

	return depth;
}

static FTexture2DRHIRef CreateTexture2D(const FWaveVRRenderTextureInfo& info, GLuint resource = 0)
{
	LOG_FUNC();
	if (PLATFORM_WINDOWS) {
		FRHIResourceCreateInfo createInfo;
		return GDynamicRHI->RHICreateTexture2D(
			info.renderWidth, info.renderHeight, EPixelFormat(info.format),
			1/*Mips*/, info.numSamples, info.flags, ERHIAccess::CopySrc,createInfo);
	} else if (PLATFORM_ANDROID) {
		auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
		check(GLRHI);

		if (info.arraySize > 1) {
			if (resource == 0) {
				// Not Work ?  texture release it self?  modify the texture as FTexture2DRHIRef.  Need verify.
				FTexture2DRHIRef texture = new FOpenGLTexture2D(
					GLRHI, 0, info.target, GL_NONE, info.renderWidth, info.renderHeight, 0,
					info.mips, 1, info.numSamples/*SamplesTileMem*/,
					info.arraySize, EPixelFormat(info.format), false, false, info.flags, FClearValueBinding::Black);
				GLRHI->InitializeGLTexture(
					texture, info.renderWidth, info.renderHeight, false, true/*Texture3D*/, false,
					EPixelFormat(info.format), info.mips, 1/*numSamples*/,
					info.arraySize, info.flags, FClearValueBinding::Black, nullptr);
				return texture;
			} else {
				// Work
				return GLRHI->RHICreateTexture2DArrayFromResource(
					EPixelFormat(info.format), info.renderWidth, info.renderHeight,
					info.arraySize,
					1/*Mips*/, 1, info.numSamples/*SamplesTileMem*/,
					FClearValueBinding::Black, resource, info.flags);
			}
		} else {
			if (resource == 0) {
				FRHIResourceCreateInfo createInfo;
				return GDynamicRHI->RHICreateTexture2D(
					info.renderWidth, info.renderHeight, EPixelFormat(info.format),
					1/*Mips*/, info.numSamples, info.flags, ERHIAccess::CopySrc,createInfo);
			} else {
				// TODO not Work
				return GLRHI->RHICreateTexture2DFromResource(
					EPixelFormat(info.format), info.renderWidth, info.renderHeight,
					1/*Mips*/, info.numSamples, info.numSamples/*SamplesTileMem*/,
					FClearValueBinding::Black, resource, info.flags);
			}
		}
	}
}

// Only used to create the textures for WVR submit.  Textures are following the WVR settings.
void FWaveVRTexturePool::CreateColorInPool(uint32 index)
{
	LOG_FUNC();

	check(mColorPool);
	check(index <= textureInfo.capacity);

	const auto info = textureInfo;

	// 4 cases
	if (textureInfo.useUnrealTextureQueue && textureInfo.createFromResource) {
		// Use textures in wvrTextureQueue and not use a base texture to alias.
		WVR_TextureParams_t params;
		check(textureInfo.wvrTextureQueue);
		params = WVR()->GetTexture(info.wvrTextureQueue, index); //get texture id, target, layout
		mColorPool[index] = CreateTexture2D(info, (GLuint)PTR_TO_INT(params.id));
	} else if (!textureInfo.useUnrealTextureQueue && textureInfo.createFromResource) {
		// Use textures in wvrTextureQueue and use a base texture to alias.
		check(textureInfo.wvrTextureQueue);
		check(mBaseColor);
		check(index == 0);

		for (int i = 0; i < info.capacity; i++) {
			WVR_TextureParams_t params = WVR()->GetTexture(info.wvrTextureQueue, info.capacity - 1 - i /* damn it. wvr is inversed */);

			mColorPool[i] = CreateTexture2D(info, (GLuint)PTR_TO_INT(params.id));
		}
	} else if (!textureInfo.useUnrealTextureQueue && !textureInfo.createFromResource) {
		// As the old way
		// Create texture in unreal and use a base texture to alias.
		check(mBaseColor);
		check(index == 0);

		for (int i = 0; i < info.capacity; i++) {
			mColorPool[i] = CreateTexture2D(info);
			MakeAlias(i);
		}
		//void ** textures = new void *[info.capacity];
		//for (int i = 0; i < info.capacity; i++) {
		//	textures[i] = (void *)PTR_TO_INT(*(GLuint *)(mColorPool[i]->GetNativeResource()));
		//	LOGI(WVRRenderTP, "WVR_TextureParams_t[%d]=%p", i, textures[i]);
		//}
		//textureInfo.wvrTextureQueue = WVR()->StoreRenderTextures(textures, info.capacity, WVR_Eye_Left);

	} else if (textureInfo.useUnrealTextureQueue && !textureInfo.createFromResource) {
		// Create texture in unreal and not use a base texture to alias.
		mColorPool[index] = CreateTexture2D(info);
	} else {
		// Impossible
		check(false);
	}
}

static FOpenGLTexture2D * CreateDepthTexture2D(const FWaveVRRenderTextureInfo& info)
{
	LogTextureInfo("CreateDepthTexture2D", info);

	int arraySize = info.arraySize;
	bool isArray = arraySize > 1;

	auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
	check(GLRHI);

	ETextureCreateFlags flags = ETextureCreateFlags(info.flags | info.extraFlags);  // extraFlags 524 = TexCreate_DepthStencilTargetable 4 | TexCreate_ShaderResource 8 | TexCreate_InputAttachmentRead 512
#define USE_RHI 0
#if USE_RHI
	// TODO return variable is not compatible.
	FOpenGLTexture2D * depthPtr = GLRHI->RHICreateTexture2D()
#else
	GLenum target = isArray ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY;
	FOpenGLTexture2D * depthPtr = new FOpenGLTexture2D(
		GLRHI, 0, target, GL_DEPTH_ATTACHMENT, info.renderWidth, info.renderHeight, 0, 1, 1, info.numSamples/*NumSampleTileMem*/, arraySize, (EPixelFormat)info.format/* PF_DepthStencil*/,false, false, flags, FClearValueBinding::DepthFar);

	if (info.numSamples > 1) {
		// Copy necessary parts of InitializeGLTexture here.
		GLuint TextureID = 0;
		glGenTextures(1, &TextureID);
		glBindRenderbuffer(GL_TEXTURE_2D_ARRAY, TextureID);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, info.numSamples, GL_DEPTH_COMPONENT24, info.renderWidth, info.renderHeight, 2);
		glBindRenderbuffer(GL_TEXTURE_2D_ARRAY, 0);

		GLenum Attachment = GL_NONE;
		Attachment = GL_DEPTH_ATTACHMENT;
		//check(GMaxOpenGLDepthSamples >= (GLint)NumSamples);  // can't find GMaxOpenGLDepthSamples
		depthPtr->SetResource(TextureID);
		depthPtr->Target = GL_TEXTURE_2D_ARRAY;
		depthPtr->Attachment = Attachment;
		depthPtr->SetAllocatedStorage(false);
		OpenGLTextureAllocated(depthPtr, flags);
	} else {
		// This API will not create multisample correctly. Only used for sample=1.
	GLRHI->InitializeGLTexture(
		depthPtr, info.renderWidth, info.renderHeight, false, isArray, false,
			info.format, 1, 1/*numSamples*/,
		arraySize, flags, FClearValueBinding::DepthFar, nullptr);
	}
#endif
	LOGD(WVRRenderTP, "CreateDepthTexture2D() tid=%u", depthPtr->GetResource());
	return depthPtr;
}

void FWaveVRTexturePool::CreateDepthInPool(const FWaveVRRenderTextureInfo& engineInfo, uint32 index)
{
	LOG_FUNC();
	check(index == 0);

	const auto& info = textureInfo;

	if (!mBaseDepth) {
		mBaseDepth = CreateBaseDepth(engineInfo);
	}

	// Depth
	if (mDepthPool != nullptr) {
		for (int i = 0; i < info.capacity; i++) {
			// When Refs count to zero, the texture's destructor will call Release() itself.  We don't have to call Release().
			//mDepthPool[i]->Release();
			LOGD(WVRRenderTP, "SafeRelease mDepthPool[%d].GetRefCount()=%d", i, mDepthPool[i].TRHI().GetRefCount());
			mDepthPool[i].SafeRelease();
		}
		//LOGD(WVRRenderTP, "delete[] mDepthPool");
		delete[] mDepthPool;
		mDepthPool = nullptr;
	}

	// Create new queue
	mDepthPool = new FTextureVariation[info.capacity];

	for (int i = 0; i < info.capacity; i++) {
		mDepthPool[i] = CreateDepthTexture2D(engineInfo);
	}

	MakeDepthAlias(2);
}

// Very confusable here.
// Idx could be used by the target or the base.  It's depended on resource.
void FWaveVRTexturePool::MakeAlias(int index, GLuint resource)
{
	LOG_FUNC();
	check(mColorPool);
	check(mColorPool[index]);

	auto target = GetOpenGLTextureFromRHITexture(mColorPool[index]);  // static_cast<FOpenGLTextureBase*>(mColorPool[index]->GetTextureBaseRHI());
	auto base = mBaseColor ? mBaseColor : target;
	// mBaseColor of course is aliased.  target could be aliased or not.
	if (base->IsAliased()) {
		if (resource == 0) {
			// Has mBase and use idx target.
			// Alias the the target resource from texture pool
			base->AliasResources(target);
			//LOGV(WVRRenderTP, "MakeAlias(%d, 0) -> target %u", idx, target->Resource);
		} else {
			// For using unreal texture queue, the resource not match the wvr queue's idx.
			if (base->GetResource() != resource) {
				//LOGV(WVRRenderTP, "MakeAlias(%d, %u) -> originalResource %u -> %u", idx, resource, base->Resource, resource);
				base->SetResource(resource);
			} else {
				//LOGV(WVRRenderTP, "MakeAlias(%d, %u) noop", idx, resource);
			}
		}
	} else {
		// No mBase and target is not aliased
		// Do nothing
		//LOGV(WVRRenderTP, "MakeAlias(%d, %u) noop", idx, resource);
	}
}

// Idx could be used by the target or the base.  It's depended on resource.
uint32_t FWaveVRTexturePool::MakeDepthAlias(int index, GLuint resource)
{
	LOG_FUNC();
	// We don't manage depth texture.
	if (!mBaseDepth || !mDepthPool[index])
		return 0;

	check(mBaseDepth);
	check(mDepthPool);
	check(mDepthPool[index]);

	FOpenGLTextureBase* target = mDepthPool[index]; // static_cast<FOpenGLTextureBase*>([index]->GetTextureBaseRHI());
	auto base = mBaseDepth ? mBaseDepth : target;  // this will always use mBaseDepth
	// mBaseColor of course is aliased.  target could be aliased or not.
	if (base->IsAliased()) {
		if (resource == 0) {
			// Has mBase and use idx target.
			// Alias the the target resource from texture pool
			//GLRHI->RHIAliasTextureResources
			base->AliasResources(target);
			//LOGV(WVRRenderTP, "MakeDepthAlias(%d, 0) -> target %u", index, target->Resource);
			return target->GetResource();
		} else {
			// For using unreal texture queue, the resource not match the wvr queue's idx.
			if (base->GetResource() != resource) {
				//LOGV(WVRRenderTP, "MakeDepthAlias(%d, %u) -> originalResource %u -> %u", index, resource, base->Resource, resource);
				base->SetResource(resource);
				return resource;
			} else {
				//LOGV(WVRRenderTP, "MakeDepthAlias(%d, %u) noop", index, resource);
				return resource;
			}
		}
	} else {
		// No mBase and target is not aliased
		// Do nothing
		//LOGV(WVRRenderTP, "MakeDepthAlias(%d, %u) noop", index, resource);
		return resource;
	}
}

uint32 FWaveVRTexturePool::FindIndexByGLId(GLuint id) const
{
	check(id);

	// Find the index
	for (int i = 0; i < textureInfo.capacity; i++) {
		auto base = (FOpenGLTextureBase*)mColorPool[i]->GetTextureBaseRHI();
		if (base->GetResource() == id) {
			return i;
		}
	}

	// Imposible
	LOGD(WVRRenderTP, "FindIndexByGLId(id=%u) texture was not found in pool", id);
	for (int i = 0; i < textureInfo.capacity; i++) {
		auto base = (FOpenGLTextureBase*)mColorPool[i]->GetTextureBaseRHI();
		LOGD(WVRRenderTP, "  mColorPool[%d]=%u", i, base->GetResource());
	}
	check(false);
	return 0;
}

uint32 FWaveVRTexturePool::FindIndexByResource(void * nativeResource) const
{
	check(nativeResource);

	// Find the index
	for (int i = 0; i < textureInfo.capacity; i++) {
		check(mColorPool[i]);
		if (mColorPool[i]->GetNativeResource() == nativeResource) {
			return i;
		}
	}
	// Imposible
	check(false);
	return 0;
}

uint32 FWaveVRTexturePool::FindIndexByTexture(void * textureBaseRHI) const
{
	check(textureBaseRHI);

	// Find the index
	for (int i = 0; i < textureInfo.capacity; i++) {
		check(mColorPool[i]);
		if (mColorPool[i]->GetTextureBaseRHI() == textureBaseRHI) {
			return i;
		}
	}
	// Imposible
	check(false);
	return 0;
}

const TRefCountPtr<FOpenGLTexture2D> FWaveVRTexturePool::GetGLTexture(uint32 index) const
{
	if (mBaseColor) {
		return mBaseColor;
	} else {
		check(index < textureInfo.capacity);
		check(mColorPool[index]);
		return static_cast<FOpenGLTexture2D*>(mColorPool[index]->GetTextureBaseRHI());
	}
}

FTexture2DRHIRef FWaveVRTexturePool::GetRHIColor(uint32 index) const
{
	LOG_FUNC();
	if (mBaseColor) {
		return mBaseColor->GetTexture2D();
	} else {
		check(index < textureInfo.capacity);
		check(mColorPool[index]);
		return mColorPool[index]->GetTexture2D();
	}
}

// Because the Engine will always get depth index 0.  The real index is maintained by color side.
FTexture2DRHIRef FWaveVRTexturePool::GetRHIDepth(uint32 index) const
{
	LOG_FUNC();
	return mBaseDepth;
}

void FWaveVRTexturePool::ReleaseTextures()
{
	bool hasBaseTexture = !!mBaseColor;
	LOG_FUNC();
	if (mColorPool) {
		for (int i = 0; i < textureInfo.capacity; i++) {
			LOGD(WVRRenderTP, "TexturePool: Release color texture %d", i);
			// When Refs count to zero, the texture's destructor will call Release() itself.  We don't have to call Release().
			//if (hasBaseTexture && mColorPool[i])
			//	mColorPool[i]->Release();
			mColorPool[i].SafeRelease();
		}
		delete[] mColorPool;
		mColorPool = nullptr;
	}

	if (mDepthPool) {
		for (int i = 0; i < textureInfo.capacity; i++) {
			LOGD(WVRRenderTP, "TexturePool: Release depth texture %d", i);
			// When Refs count to zero, the texture's destructor will call Release() itself.  We don't have to call Release().
			//if (hasBaseTexture && mColorPool[i])
			//	mColorPool[i]->Release();
			mDepthPool[i].SafeRelease();
		}
		delete[] mDepthPool;
		mDepthPool = nullptr;
	}

	if (mBaseColor) {
		LOGD(WVRRenderTP, "TexturePool: Release Base color");
		mBaseColor.SafeRelease();
	}

	if (mBaseDepth) {
		LOGD(WVRRenderTP, "TexturePool: Release Base depth");
		mBaseDepth.SafeRelease();
	}

	if (textureInfo.wvrTextureQueue) {
		LOGD(WVRRenderTP, "TexturePool: Release wvrTextureQueue");
		WVR()->ReleaseTextureQueue(textureInfo.wvrTextureQueue);
		textureInfo.wvrTextureQueue = nullptr;
	}

	LOGD(WVRRenderTP, "TexturePool: Released");
}

}  // namespace Render
}  // namespace Wave
