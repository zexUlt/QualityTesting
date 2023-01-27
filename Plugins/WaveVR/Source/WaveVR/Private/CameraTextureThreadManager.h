// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include "WaveVRCameraTexture.h"

class CameraTextureThreadManager
{
public:
	CameraTextureThreadManager(bool inIsSyncPose, UStaticMeshComponent* inStaticMeshComponent, UWaveVRCameraTexture* inCameraTextureInstance);
	~CameraTextureThreadManager();

	void startThread();
	void stopThread();

private:

	UWaveVRCameraTexture* cameraTextureInstance;
	static CameraTextureThreadManager* currentInstance;

	uint32_t predictInMs = 0;

	//Camera Texture Thread
	std::thread mCameraTextureThread;
	std::mutex mMutex;
	std::atomic<bool> mIsThreadRunning;
	std::atomic<bool> frameBufferUpdated;

	void threadInit();
	void threadTerminate();
	void threadCycle();

public:

	static inline CameraTextureThreadManager* CreateCameraTextureThreadManager(bool inIsSyncPose, UStaticMeshComponent* inStaticMeshComponent, UWaveVRCameraTexture* inCameraTextureInstance)
	{
		currentInstance = new CameraTextureThreadManager(inIsSyncPose, inStaticMeshComponent, inCameraTextureInstance);
		return currentInstance;
	}

	static inline CameraTextureThreadManager* GetInstance()
	{
		if (currentInstance != nullptr)
			return currentInstance;
		else
			return nullptr;
	}

	static inline void StopInstance()
	{
		if (currentInstance != nullptr)
		{
			delete currentInstance;
			currentInstance = nullptr;
		}
	}

	inline bool isFrameBufferUpdated()
	{
		return frameBufferUpdated.load(std::memory_order_acquire);
	}

	inline void setFrameBufferUpdated(bool status)
	{
		frameBufferUpdated.store(status, std::memory_order_release);
	}
};
