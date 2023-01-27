// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRCameraTexture.h"
#include "Engine/Engine.h"

#include "Components/StaticMeshComponent.h"

#include "RHI.h"
#include "RHIUtilities.h"
#include "OpenGLDrv.h"

#include "WaveVRHMD.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRPermissionManager.h"

#include "CameraTextureThreadManager.h"

DEFINE_LOG_CATEGORY_STATIC(WVR_Camera, Display, All);

#pragma region UE Lifecycle

// Sets default values for this component's properties
UWaveVRCameraTexture::UWaveVRCameraTexture()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	reset();

	if (frameBuffer != nullptr) {
		delete frameBuffer;
		frameBuffer = nullptr;
	}

	// ...
}

// Called when the game starts
void UWaveVRCameraTexture::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UWaveVRCameraTexture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWaveVRCameraTexture::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	LOGD(WVR_Camera, "CameraTexture: EndPlay, Reason: %d", EndPlayReason);

	Super::EndPlay(EndPlayReason);
}

#pragma endregion

#pragma region Get Set Functions

bool UWaveVRCameraTexture::isCameraStarted()
{
	return bCameraActive;
}

bool UWaveVRCameraTexture::isSyncPose()
{
	return bSyncPose;
}

EWVR_CameraImageFormat UWaveVRCameraTexture::getCameraImageFormat() {
	return (bCameraActive ? mImgFormat : EWVR_CameraImageFormat::WVR_CameraImageFormat_Invalid);
}

EWVR_CameraImageType UWaveVRCameraTexture::getCameraImageType() {
	return (bCameraActive ? mImgType : EWVR_CameraImageType::WVR_CameraImageType_Invalid);
}

int UWaveVRCameraTexture::getCameraImageWidth() {
	return (bCameraActive ? mWidth : 0);
}

int UWaveVRCameraTexture::getCameraImageHeight() {
	return (bCameraActive ? mHeight : 0);
}

uint8_t* UWaveVRCameraTexture::getFrameBuffer() {
	return (frameBuffer ? frameBuffer : nullptr);
}

int UWaveVRCameraTexture::getFrameBufferSize() {
	return (bCameraActive ? mSize : 0);
}

#pragma endregion

void UWaveVRCameraTexture::StartCameraTexture(bool enableSyncPose, UStaticMeshComponent* staticMeshComponent)
{
	LOGD(WVR_Camera, "Start Camera Texture");

	if (!bCameraActive)
	{
		bSyncPose = enableSyncPose;
		targetMesh = staticMeshComponent;
		LOGD(WVR_Camera, "Sync Pose: %s", bSyncPose ? "True" : "False");

		if (!bSyncPose) //Start new thread if non sync pose
		{
			CameraTextureThreadManager* cameraTextureThreadInstance = CameraTextureThreadManager::CreateCameraTextureThreadManager(enableSyncPose, staticMeshComponent, this);
			if (!cameraTextureThreadInstance)
			{
				LOGD(WVR_Camera, "CreateCameraTextureThreadManager failed");
				return;
			}
			cameraTextureThreadInstance->startThread();
		}
		else
		{
			//Start Camera
			if (StartCamera())
			{
				if (CreateCameraTexture())
				{
					if (CreateCameraTextureMID())
					{
						StartCameraCompletedDelegate.Broadcast(true);
					}
				}
			}
		}

		PrimaryComponentTick.bCanEverTick = true;
		PrimaryComponentTick.TickGroup = TG_PostPhysics;
		bCameraActive = true;

		deactivateDelegate = FCoreDelegates::ApplicationWillDeactivateDelegate.AddUFunction(this, FName("StopCameraTexture"));
		enterBackgroundDelegate = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUFunction(this, FName("StopCameraTexture"));
		terminateDelegate = FCoreDelegates::ApplicationWillTerminateDelegate.AddUFunction(this, FName("StopCameraTexture"));
	}
}

bool UWaveVRCameraTexture::StartCamera()
{
	LOGD(WVR_Camera, "Start Camera");
	if (!bCameraActive)
	{
		LOGD(WVR_Camera, "Checking Camera Permission");
		bool bCameraPermissionGranted = UWaveVRPermissionManager::isPermissionGranted(FString("android.permission.CAMERA"));
		LOGD(WVR_Camera, "Camera Permission status: %s", bCameraPermissionGranted ? "True" : "False");

		if (!bCameraPermissionGranted)
		{
			LOGD(WVR_Camera, "Camera Permission not granted, camera will not be started.");
			bCameraActive = false;
			return bCameraActive;
		}

		WVR_CameraInfo cameraInfo;

		bCameraActive = FWaveVRAPIWrapper::GetInstance()->StartCamera(&cameraInfo);
		if (bCameraActive)
		{
			bNativeResourceReleased = false;
			if (cameraInfo.imgType == WVR_CameraImageType_SingleEye)
			{
				mSize = cameraInfo.size;
				mWidth = cameraInfo.width;
				mHeight = cameraInfo.height;
				frameBuffer = (uint8_t *)malloc(mSize);
				mImgType = (EWVR_CameraImageType)WVR_CameraImageType_SingleEye;
				mImgFormat = (EWVR_CameraImageFormat)cameraInfo.imgFormat;
			}
			else if (cameraInfo.imgType == WVR_CameraImageType_DualEye)
			{
				mSize = cameraInfo.size;
				mWidth = cameraInfo.width;
				mHeight = cameraInfo.height;
				frameBuffer = (uint8_t *)malloc(mSize);
				mImgType = (EWVR_CameraImageType)WVR_CameraImageType_DualEye;
				mImgFormat = (EWVR_CameraImageFormat)cameraInfo.imgFormat;
			}
			else
			{
				LOGE(WVR_Camera, "Image Type not supported.");
				mWidth = 640;
				mHeight = 480;
			}
			LOGD(WVR_Camera, "Camera Started successfully, width = %d, height = %d, size = %d", mWidth, mHeight, mSize);
		}
		else
		{
			LOGE(WVR_Camera, "Failed to start camera");
			mWidth = 640;
			mHeight = 480;
		}

	}

	LOGD(WVR_Camera, "Start camera result: %s", bCameraActive ? "True" : "False");
	return bCameraActive;
}

UTextureRenderTarget2D* UWaveVRCameraTexture::GetCameraTexture()
{
	LOGD(WVR_Camera, "Get camera texture");
	if (!cameraTexture)
	{
		return nullptr;
	}
	LOGD(WVR_Camera, "GetCameraTexture: CameraTexture ptr=%p", cameraTexture);

	return cameraTexture;
}

UTextureRenderTarget2D* UWaveVRCameraTexture::CreateCameraTexture()
{
	LOGD(WVR_Camera, "Create new camera texture");
	cameraTexture = NewObject<UTextureRenderTarget2D>();
	if (!cameraTexture)
	{
		return nullptr;
	}
	textureid = 0;
	cameraTexture->ClearColor = FLinearColor::White;
	cameraTexture->TargetGamma = 1;
	cameraTexture->InitAutoFormat(mWidth, mHeight);

	LOGD(WVR_Camera, "GetCameraTexture: CameraTexture ptr=%p", cameraTexture);

	return cameraTexture;
}

UMaterialInstanceDynamic* UWaveVRCameraTexture::CreateCameraTextureMID()
{
	if (targetMesh)
	{
		cameraTextureMID = targetMesh->CreateDynamicMaterialInstance(0, targetMesh->GetMaterial(0), "CameraTextureMID");

		TArray<FMaterialParameterInfo> matParams;
		TArray<FGuid> guids;

		targetMesh->GetMaterial(0)->GetAllTextureParameterInfo(matParams, guids);
		LOGD(WVR_Camera, "targetMesh Material: %s, Texture Name: %s", TCHAR_TO_ANSI(*(targetMesh->GetMaterial(0)->GetFName().ToString())), TCHAR_TO_ANSI(*(matParams[0].Name.ToString())));

		cameraTextureMID->SetTextureParameterValue(matParams[0].Name, cameraTexture);

		LOGD(WVR_Camera, "Camera Started and Camera Texture is Ready");
	}

	return cameraTextureMID;
}

void UWaveVRCameraTexture::StopCameraTexture()
{
	if (bCameraActive)
	{
		if (bSyncPose)
		{
			LOGD(WVR_Camera, "Reset Submit Pose");
			FWaveVRHMD::GetInstance()->RenderSetSubmitWithPose(false, nullptr);
		}

		CameraTextureThreadManager* cameraTextureThreadInstance = CameraTextureThreadManager::GetInstance();
		if (cameraTextureThreadInstance)
		{
			LOGD(WVR_Camera, "Stop thread instance");
			cameraTextureThreadInstance->StopInstance();

			UWaveVRCameraTexture* cameraTextureInstance = this;
			ENQUEUE_RENDER_COMMAND(CameraTextureReleaseNativeReasources) (
				[cameraTextureInstance](FRHICommandListImmediate& RHICmdList)
			{
				cameraTextureInstance->releaseNativeResources_RenderThread();
			});
		}

		if (frameBuffer)
		{
			free(frameBuffer);
			frameBuffer = nullptr;
		}

		FWaveVRAPIWrapper::GetInstance()->StopCamera();
		StopCameraCompletedDelegate.Broadcast();
		bCameraActive = false;
		reset();

		FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(deactivateDelegate);
		FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(enterBackgroundDelegate);
		FCoreDelegates::ApplicationWillTerminateDelegate.Remove(terminateDelegate);
	}
}

void UWaveVRCameraTexture::UpdateCameraTexture()
{
	if (bCameraActive)
	{
		if (bSyncPose)
		{
			//Get origin model
			WVR_PoseOriginModel originModel = WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnGround;
			IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
			EHMDTrackingOrigin::Type hmdOrigin = XRSystem->GetTrackingOrigin();
			switch (hmdOrigin)
			{
			case EHMDTrackingOrigin::Eye:
				originModel = WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnHead;
				break;
			case EHMDTrackingOrigin::Floor:
				originModel = WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnGround;
				break;
			}

			WVR_PoseState_t cameraPoseState;
			bool ret = FWaveVRAPIWrapper::GetInstance()->CamUtil_GetFrameBufferWithPoseState(frameBuffer, mSize, originModel, predictInMs, &cameraPoseState);
			if (ret)
			{
				FWaveVRHMD::GetInstance()->RenderSetSubmitWithPose(true, &cameraPoseState);

				cameraTextureRenderTargetResource = cameraTexture->GameThread_GetRenderTargetResource();
				UWaveVRCameraTexture* cameraTextureInstance = this;
				ENQUEUE_RENDER_COMMAND(CameraTextureUpdateTextureReference) (
					[cameraTextureInstance](FRHICommandListImmediate& RHICmdList)
				{
					cameraTextureInstance->getCameraTextureId_RenderThread();
				});

				LOGD(WVR_Camera, "Enqueue draw texture into render queue");

				ENQUEUE_RENDER_COMMAND(CameraTextureDrawTexture) (
					[cameraTextureInstance](FRHICommandListImmediate& RHICmdList)
				{
					bool isNoErrorContext = false;
#if UE_BUILD_SHIPPING //Add GL_No_Error because shipping build change the elg config attribute since UE4.23.
					isNoErrorContext = true;
#endif
					cameraTextureInstance->drawTextureWithBuffer_RenderThread(cameraTextureInstance->textureid, cameraTextureInstance->bEnableCropping, cameraTextureInstance->bClearClampingRegion, isNoErrorContext);
				});
			}
		}
		else
		{
			CameraTextureThreadManager* cameraTextureThreadInstance = CameraTextureThreadManager::GetInstance();

			if (cameraTextureThreadInstance && cameraTextureThreadInstance->isFrameBufferUpdated())
			{
				cameraTextureRenderTargetResource = cameraTexture->GameThread_GetRenderTargetResource();
				UWaveVRCameraTexture* cameraTextureInstance = this;
				ENQUEUE_RENDER_COMMAND(CameraTextureUpdateTextureReference) (
					[cameraTextureInstance](FRHICommandListImmediate& RHICmdList)
				{
					cameraTextureInstance->getCameraTextureId_RenderThread();
				});

				LOGD(WVR_Camera, "Enqueue draw texture into render queue");

				ENQUEUE_RENDER_COMMAND(CameraTextureDrawTexture) (
					[cameraTextureInstance](FRHICommandListImmediate& RHICmdList)
				{
					bool isNoErrorContext = false;
#if UE_BUILD_SHIPPING //Add GL_No_Error because shipping build change the elg config attribute since UE4.23.
					isNoErrorContext = true;
#endif
					cameraTextureInstance->drawTextureWithBuffer_RenderThread(cameraTextureInstance->textureid, cameraTextureInstance->bEnableCropping, cameraTextureInstance->bClearClampingRegion, isNoErrorContext);
				});
			}
		}
	}
	else
	{
		if (!bNativeResourceReleased)
		{
			UWaveVRCameraTexture* cameraTextureInstance = this;
			ENQUEUE_RENDER_COMMAND(CameraTextureReleaseNativeReasources) (
				[cameraTextureInstance](FRHICommandListImmediate& RHICmdList)
			{
				cameraTextureInstance->releaseNativeResources_RenderThread();
			});

			bNativeResourceReleased = true;
		}
	}
}

void UWaveVRCameraTexture::getCameraTextureId_RenderThread()
{
	if (textureid == 0)
	{
		cameraTextureWrapperRHIRef = cameraTextureRenderTargetResource->GetRenderTargetTexture();//RHIRef
		if (cameraTextureWrapperRHIRef.IsValid()) {
			textureid = static_cast<uint32_t>(*(GLuint*)cameraTextureWrapperRHIRef->GetNativeResource());
		}
		else {
			LOGE(WVR_Camera, "cameraTextureWrapperRHIRef is not valid!");
		}
		LOGE(WVR_Camera, "textureid = %d", textureid);
	}
}

bool UWaveVRCameraTexture::drawTextureWithBuffer_RenderThread(uint32_t InTextureID, bool enableCropping, bool clearClampingRegion, bool noErrorContext) //Called in render thread
{
	bool ret = FWaveVRAPIWrapper::GetInstance()->CamUtil_DrawTextureWithBuffer(InTextureID, (WVR_CameraImageFormat)mImgFormat, frameBuffer, mSize, mWidth, mHeight, enableCropping, clearClampingRegion, noErrorContext);
	LOGD(WVR_Camera, "drawTextureWithBuffer_RenderThread: Textureid = %d, frameBuffer = %p, Size = %d, Width = %d, Height = %d", InTextureID, frameBuffer, mSize, mWidth, mHeight);
	CameraTextureUpdateCompletedDelegate.Broadcast(ret);
	return ret;
}

void UWaveVRCameraTexture::releaseNativeResources_RenderThread()
{
	FWaveVRAPIWrapper::GetInstance()->CamUtil_ReleaseCameraTexture();
}

void UWaveVRCameraTexture::reset() {
	bCameraActive = false;
	bFrameBufferUpdated = false;
	mSize = 0;
	mWidth = 0;
	mHeight = 0;
	textureid = 0;
	mImgFormat = (EWVR_CameraImageFormat)WVR_CameraImageFormat_Invalid;
	mImgType = (EWVR_CameraImageType)WVR_CameraImageType_Invalid;

	cameraTexture = nullptr;
	cameraTextureMID = nullptr;
}
