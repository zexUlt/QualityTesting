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

#include "CoreMinimal.h"
#include "PixelFormat.h"
#include "TextureResource.h"
#include "Components/ActorComponent.h"
#include "Components/ApplicationLifecycleComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WaveVRCameraTexture.generated.h"

UENUM(BlueprintType)
enum class EWVR_CameraImageType : uint8
{
	WVR_CameraImageType_Invalid = 0,
	WVR_CameraImageType_SingleEye = 1,
	WVR_CameraImageType_DualEye = 2,
};

UENUM(BlueprintType)
enum class EWVR_CameraImageFormat : uint8
{
	WVR_CameraImageFormat_Invalid = 0,
	WVR_CameraImageFormat_YUV_420 = 1,
	WVR_CameraImageFormat_Grayscale = 2,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartCameraCompleted, bool, result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCameraTextureUpdateCompleted, bool, result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStopCameraCompleted);

UCLASS(ClassGroup = (WaveVR), meta = (BlueprintSpawnableComponent))
class WAVEVR_API UWaveVRCameraTexture : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWaveVRCameraTexture();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Start WaveVRCamera Texture Component."))
		void StartCameraTexture(bool enableSyncPose, UStaticMeshComponent* staticMeshComponent);

	UFUNCTION(Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Start native camera component."))
		bool StartCamera();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Return UTextureRenderTarget2D pointer for displaying camera texture."))
		UTextureRenderTarget2D* GetCameraTexture();

	UFUNCTION(Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Create UTextureRenderTarget2D pointer for displaying camera texture."))
		UTextureRenderTarget2D* CreateCameraTexture();

	UFUNCTION(Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Create and return MID camera texture."))
		UMaterialInstanceDynamic* CreateCameraTextureMID();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "To update camera texture."))
		void UpdateCameraTexture();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Stop WaveVRCamera Texture Component."))
		void StopCameraTexture();

#pragma region Get Set Functions

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Return image format from native camera component."))
		EWVR_CameraImageFormat getCameraImageFormat();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Return image type from native camera component."))
		EWVR_CameraImageType getCameraImageType();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Return image width from native camera component."))
		int getCameraImageWidth();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Return image height from native camera component."))
		int getCameraImageHeight();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Return true if native camera component is started."))
		bool isCameraStarted();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Get camera texture sync pose mode."))
		bool isSyncPose();

	uint8_t* getFrameBuffer();
	int getFrameBufferSize();

#pragma endregion

#pragma region  Delegates

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Delegate that will be called when camera is started."))
		FStartCameraCompleted StartCameraCompletedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Delegate that will be called when camera texture is updated."))
		FCameraTextureUpdateCompleted CameraTextureUpdateCompletedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|CameraTexture", meta = (
		ToolTip = "Delegate that will be called when camera is stopped."))
		FStopCameraCompleted StopCameraCompletedDelegate;

#pragma endregion

private:
	bool bCameraActive = false;
	bool bSyncPose = false;
	bool bFrameBufferUpdated = false;
	bool bNativeResourceReleased = true;
	bool bEnableCropping = true;
	bool bClearClampingRegion = true;

	FDelegateHandle deactivateDelegate, enterBackgroundDelegate, terminateDelegate;

	FTextureRenderTargetResource* cameraTextureRenderTargetResource = nullptr;
	FTexture2DRHIRef cameraTextureWrapperRHIRef;
	UTextureRenderTarget2D* cameraTexture = nullptr;
	UMaterialInstanceDynamic* cameraTextureMID = nullptr;
	UStaticMeshComponent* targetMesh = nullptr;

	void getCameraTextureId_RenderThread();
	bool drawTextureWithBuffer_RenderThread(uint32_t InTextureID, bool enableCropping, bool clearClampingRegion, bool noErrorContext);
	void releaseNativeResources_RenderThread();

	uint32_t mWidth = 0;
	uint32_t mHeight = 0;
	uint32_t mSize = 0;
	uint32_t textureid = 0;
	uint32_t predictInMs = 0;
	EWVR_CameraImageType mImgType;
	EWVR_CameraImageFormat mImgFormat;

	uint8_t* frameBuffer = nullptr;

	void reset();
};
