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

//#include "UnrealClient.h"
#include "CoreMinimal.h"
#include "XRRenderTargetManager.h"

#include "XRRenderBridge.h"

#include "wvr_render.h"

#include "WaveVRTextureManager.h"

class FWaveVRHMD;
class FWaveVRRender;
class FWaveVRTexturePool;

class FWaveVRFXRRenderBridge : public FXRRenderBridge
{
public:
	FWaveVRFXRRenderBridge(FWaveVRRender * render);
	virtual ~FWaveVRFXRRenderBridge();

	// FRHICustomPresent
	virtual bool NeedsNativePresent() override; //D3D12 only now, not mobile.
	virtual bool Present(int& SyncInterval) override;
	virtual void OnBackBufferResize() override;
	virtual bool NeedsAdvanceBackbuffer() override { return false; }; //D3D12 only now, not mobile.

#if 0
	// Called from RHI thread when the engine begins drawing to the viewport.
	virtual void BeginDrawing() override {};
	// Called from RHI thread after native Present has been called
	virtual void PostPresent() override {};
	// Called when rendering thread is acquired
	virtual void OnAcquireThreadOwnership() override {}
	// Called when rendering thread is released
	virtual void OnReleaseThreadOwnership() override {}
#endif

private:
	FWaveVRFXRRenderBridge(const FWaveVRFXRRenderBridge &) = delete;
	FWaveVRFXRRenderBridge(FWaveVRFXRRenderBridge &&) = delete;
	FWaveVRFXRRenderBridge &operator=(const FWaveVRFXRRenderBridge &) = delete;

private:
	FWaveVRRender * mRender;
};

// Manager queue and submit
class FWaveVRRender : public FXRRenderTargetManager
{
public:
	FWaveVRRender(FWaveVRHMD * hmd);
	virtual ~FWaveVRRender();

private:
	FWaveVRRender(const FWaveVRRender &) = delete;
	FWaveVRRender(FWaveVRRender &&) = delete;
	FWaveVRRender &operator=(const FWaveVRRender &) = delete;

public:
	inline Wave::Render::FWaveVRTextureManager * GetTextureManager() { return &mTextureManager; }
	inline bool IsInitialized() const { return bInitialized; }
	inline bool IsCustomPresentSet() const { return bAlreadySetCustomPresent; }

public:
	// Set
	void SetMultiSampleLevel(int msaaLevel);
	void SetMultiView(bool enable);
	void SetFoveationMode(WVR_FoveationMode Mode);
	void SetSingleEyePixelSize(uint32 w, uint32 h);
	void SetTextureFormat(EPixelFormat format);
	void SetFoveationParams(EStereoscopicPass Eye, const WVR_RenderFoveationParams_t& FoveatParams);
	void SetFrameSharpnessEnhancementLevel(float level);
	// If pose is nullptr, use internal pose.  Only benifted when late update is enabled.
	void SetSubmitWithPose(bool enable, const WVR_PoseState_t * pose = nullptr);
	void SetPixelDensity(float PixelDensity, bool forceUpdate = false);

	// Get
	int GetMultiSampleLevel() const;
	bool GetMultiView() const;
	bool IsRenderFoveationSupport() const;
	bool IsRenderFoveationEnabled() const;
	void GetFoveationParams(EStereoscopicPass Eye, WVR_RenderFoveationParams_t& FoveatParams) const;
	void GetSingleEyePixelSize(uint32 &w, uint32 &h) const;
	uint32 GetSingleEyePixelWidth() const;
	uint32 GetSingleEyePixelHeight() const;
	void GetSingleEyeScaledPixelSize(uint32 &w, uint32 &h) const;
	uint32 GetSingleEyeScaledPixelWidth() const;
	uint32 GetSingleEyeScaledPixelHeight() const;
	EPixelFormat GetTextureFormat() const;
	float GetPixelDensity() const;

	void Apply();

private:
	float CalculatePixelDensityAndSize(float newPixelDensity, int recursiveCount = 0);
	static bool IsMatchPixelDensityScaledSize(uint32_t width, uint32_t height, float pd, uint32_t scaledW, uint32_t scaledH);

public:
	void RenderInit();
	void RenderInit_RenderThread();
	void UpdateConsoleVariable();

	// Called by HMD
	void OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily);
	// Called by custom present
	void OnFinishRendering_RenderThread();

	void SubmitFrame_RenderThread();

	// Called by HMD
	void OnResume();
	void OnPause();
	void Shutdown();


/** IStereoRenderTargetManager interface **/
public:
	/**
	 * Whether a separate render target should be used or not.
	 * In case the stereo rendering implementation does not require special handling of separate render targets
	 * at all, it can leave out implementing this interface completely and simply let the default implementation
	 * of IStereoRendering::GetRenderTargetManager() return nullptr.
	 */
	virtual bool ShouldUseSeparateRenderTarget() const override;

	/**
	* Returns true, if render target texture must be re-calculated.
	*/
	virtual bool NeedReAllocateDepthTexture(const TRefCountPtr<struct IPooledRenderTarget>& DepthTarget) override;

	/**
		* Returns number of required buffered frames.
		*/
	virtual uint32 GetNumberOfBufferedFrames() const override;

	/**
	 * Allocates a render target texture.
	 * The default implementation always return false to indicate that the default texture allocation should be used instead.
	 *
	 * @param Index			(in) index of the buffer, changing from 0 to GetNumberOfBufferedFrames()
	 * @return				true, if texture was allocated; false, if the default texture allocation should be used.
	 */
	virtual bool AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples = 1) override;

	/**
	 * Allocates a depth texture.
	 *
	 * @param Index			(in) index of the buffer, changing from 0 to GetNumberOfBufferedFrames()
	 * @return				true, if texture was allocated; false, if the default texture allocation should be used.
	 */
	virtual bool AllocateDepthTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples = 1) override;

/** XRRenderTargetManager interface **/
public:
	/**
	 * Updates viewport for direct rendering of distortion. Should be called on a game thread.
	 *
	 * @param bUseSeparateRenderTarget	Set to true if a separate render target will be used. Can potentiallt be true even if ShouldUseSeparateRenderTarget() returned false earlier.
	 * @param Viewport					The Viewport instance calling this method.
	 * @param ViewportWidget			(optional) The Viewport widget containing the view. Can be used to access SWindow object.
	 */
	virtual void UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, SViewport* ViewportWidget = nullptr) override;

	/**
	 * Calculates dimensions of the render target texture for direct rendering of distortion.
	 * This implementation calculates the size based on the current value of vr.pixeldensity.
	 */
	virtual void CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) override;

	/**
	* Returns true, if render target texture must be re-calculated.
	*/
	virtual bool NeedReAllocateViewportRenderTarget(const class FViewport& Viewport) override;

protected:
	/**
	 * Return the current render bridge for use by the current viewport. Override this method if your device implements a custom FXRRenderBridge.
	 * Should only return non-null if the render bridge is initialized and should be used for the current frame
	 */
	virtual class FXRRenderBridge* GetActiveRenderBridge_GameThread(bool bUseSeparateRenderTarget) override;

private:
	TRefCountPtr<FWaveVRFXRRenderBridge> mCustomPresent;
	Wave::Render::FWaveVRTextureManager mTextureManager;
	FWaveVRHMD * mHMD;

private:
	// WaveVR state
	bool bInitialized;
	bool bAlreadySetCustomPresent;

private:
	// WaveVR settings
	int msaaLevel;
	uint32 width, height, scaledWidth, scaledHeight;
	float pixelDensity;
	float pixelDensityChanged;
	EPixelFormat pixelFormat;
	bool isFoveatedRenderingSupported;
	bool isFoveatedRenderingEnabled;
	WVR_FoveationMode mCurrentFoveationMode;
	bool isRenderMaskEnabled;
	bool isEyeTrackingEnabled;
	bool isMultiViewEnabled;
	bool isMultiViewDirectEnabled;
	bool isDirty;
	uint32 defaultQueueSize;
	bool isSRGB;
	WVR_AMCMode mAMCMode;
	bool isTripleDepthBufferEnabled;
	int colorGamutPreferences[4];
	WVR_TimeWarpStabilizedMode mTimeWarpStabilizedMode;
	bool isFadeOutEnabled;
	bool isFrameSharpnessEnhancementEnabled;
	float mFrameSharpnessEnhancementLevel;

private:
	// Pose
	WVR_PoseState_t mPoseUsedOnSubmit;

private:
	bool bSubmitWithPose;
	bool bSubmitWithInteralPose;

private:
	WVR_RenderFoveationParams_t EnableModeFoveationParams[2];
	WVR_RenderFoveationParams_t DefaultModeFoveationParams[2];

private:
	bool needReAllocateRenderTargetTexture;
	bool needReAllocateDepthTexture;
	bool useUnrealTextureQueue;
	bool createFromResource;

private:
	WVR_Matrix4f_t wvrProjections[2];
};
