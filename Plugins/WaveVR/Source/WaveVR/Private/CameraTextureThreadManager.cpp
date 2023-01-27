// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraTextureThreadManager.h"

#include "WaveVRHMD.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRPermissionManager.h"

#if PLATFORM_ANDROID
#include <pthread.h>
#endif

DEFINE_LOG_CATEGORY_STATIC(WVR_CameraThread, Display, All);

CameraTextureThreadManager* CameraTextureThreadManager::currentInstance;

CameraTextureThreadManager::CameraTextureThreadManager(bool inIsSyncPose, UStaticMeshComponent* inStaticMeshComponent, UWaveVRCameraTexture* inCameraTextureInstance)
	: cameraTextureInstance(inCameraTextureInstance)
{
	mIsThreadRunning.store(false, std::memory_order_release);
	frameBufferUpdated.store(false, std::memory_order_release);

	//Start Camera
	if (cameraTextureInstance->StartCamera())
	{
		if (cameraTextureInstance->CreateCameraTexture())
		{
			if (cameraTextureInstance->CreateCameraTextureMID())
			{
				cameraTextureInstance->StartCameraCompletedDelegate.Broadcast(true);
			}
		}
	}
}

CameraTextureThreadManager::~CameraTextureThreadManager()
{
	LOGD(WVR_CameraThread, "~CameraTextureThreadManager()");
	stopThread();
	LOGD(WVR_CameraThread, "~CameraTextureThreadManager() END");
}

//#if PLATFORM_ANDROID
#pragma region Camera Texture Thread Management

void CameraTextureThreadManager::threadInit() //exec in thread
{
	LOGI(WVR_CameraThread, "%s Begin", __func__);
	mIsThreadRunning.store(true, std::memory_order_release);

	LOGI(WVR_CameraThread, "%s End", __func__);
}

void CameraTextureThreadManager::threadTerminate() //exec in thread
{
	LOGI(WVR_CameraThread, "%s Begin", __func__);

	frameBufferUpdated.store(false, std::memory_order_release);

	LOGI(WVR_CameraThread, "%s End", __func__);
}

void CameraTextureThreadManager::threadCycle() //exec in thread
{
	LOGI(WVR_CameraThread, "%s Begin", __func__);
	threadInit();

#if PLATFORM_ANDROID
	int resultCode = pthread_setname_np(mCameraTextureThread.native_handle(), "CamTexThread");
	LOGI(WVR_CameraThread, "Set thread name as CameraTextureRender with result: %d", resultCode);
#endif

	int frameBufferTimeoutCounter = 0;

	while (mIsThreadRunning.load(std::memory_order_acquire)) {

		uint8_t* frameBuffer = cameraTextureInstance->getFrameBuffer();
		uint32_t frameBufferSize = cameraTextureInstance->getFrameBufferSize();

		if (frameBuffer)
		{
#pragma region Update Frame Buffer
			bool ret = false;
			ret = FWaveVRAPIWrapper::GetInstance()->GetCameraFrameBuffer(frameBuffer, frameBufferSize);
			frameBufferUpdated.store(ret, std::memory_order_release);
			if (!frameBufferUpdated.load(std::memory_order_acquire))
			{
				frameBufferTimeoutCounter++;

				if (frameBufferTimeoutCounter > 100)
				{
					LOGD(WVR_CameraThread, "camerathreadcycle: GetCameraFrameBuffer failed");
					break;
				}
			}
			else
			{
				frameBufferTimeoutCounter = 0;
			}
#pragma endregion
		}
		else
		{
			LOGD(WVR_CameraThread, "camerathreadcycle: frame buffer is null");
		}
	}

	threadTerminate();
	LOGI(WVR_CameraThread, "%s End", __func__);
}

void CameraTextureThreadManager::startThread()
{
	LOGI(WVR_CameraThread, "%s Begin", __func__);
	std::lock_guard<std::mutex> guard(mMutex);
	if (!mIsThreadRunning.load(std::memory_order_acquire)) {
		LOGD(WVR_CameraThread, "Starting new thread");
		mCameraTextureThread = std::thread(&CameraTextureThreadManager::threadCycle, this);
	}
	LOGI(WVR_CameraThread, "%s End", __func__);
}

void CameraTextureThreadManager::stopThread()
{
	LOGI(WVR_CameraThread, "%s Begin", __func__);
	std::lock_guard<std::mutex> guard(mMutex);

	if (mIsThreadRunning.load(std::memory_order_acquire)) {
		mIsThreadRunning.store(false, std::memory_order_release);
		mCameraTextureThread.join();
	}

	LOGI(WVR_CameraThread, "%s End", __func__);
}
