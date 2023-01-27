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

#include "IStereoLayers.h"
#include "RendererInterface.h"
#include "ProceduralMeshComponent.h"

#include "OpenGLDrv.h"

#include "WaveVRMultiLayerResource.h"
#include "wvr_render.h"
#include "Platforms/WaveVRAPIWrapper.h"

class FWaveVRMultiLayer;

class FWaveVRMultiLayerManager
{
private:
	static FWaveVRMultiLayerManager* managerInstance;
	int frameIndex = -1;
	uint32 NextLayerId;
	TMap<uint32, FWaveVRMultiLayer*> MultiLayerMap;
	float currentWorldToMeterScale = 100;

public:

	FWaveVRMultiLayerManager();
	~FWaveVRMultiLayerManager();

	FWaveVRMultiLayer* GetWaveVRMultiLayer(uint32 InLayerID);
	uint32_t GetCurrentLayerCount();
	uint32 CreateLayer(const IStereoLayers::FLayerDesc& InLayerDesc);
	void MarkLayerToDestroy(uint32 LayerId);
	void SetLayerDesc(uint32 LayerId, const IStereoLayers::FLayerDesc& InLayerDesc);
	bool GetLayerDesc(uint32 LayerId, IStereoLayers::FLayerDesc& OutLayerDesc);
	void UpdateTextures(FRHICommandListImmediate& RHICmdList);
	void SubmitLayers();

	static inline FWaveVRMultiLayerManager* GetInstance()
	{
		if (!FWaveVRMultiLayerManager::managerInstance)
		{
			FWaveVRMultiLayerManager::managerInstance = new FWaveVRMultiLayerManager();
		}
		return FWaveVRMultiLayerManager::managerInstance;
	}

	static WVR_Vector3f_t GetVector3(const FVector& ue_pos, float worldToMeterScale) {
		WVR_Vector3f_t vec;

		vec.v[0] = ue_pos.Y / worldToMeterScale;
		vec.v[1] = ue_pos.Z / worldToMeterScale;
		vec.v[2] = -ue_pos.X / worldToMeterScale;
		return vec;
	}

	static WVR_Quatf_t GetQuaternion(const FQuat& ue_ori)
	{
		WVR_Quatf_t qat;
		//FQuat ue_ori = ue_rot.Quaternion();
		qat.w = -ue_ori.W;
		qat.x = ue_ori.Y;
		qat.y = ue_ori.Z;
		qat.z = -ue_ori.X;
		return qat;
	}

	static WVR_Matrix4f_t GetMatrix44(const FVector& ue_pos, const FRotator& ue_rot, float worldToMeterScale)
	{
		WVR_Vector3f_t pos;
		pos.v[0] = ue_pos.Y / worldToMeterScale;
		pos.v[1] = ue_pos.Z / worldToMeterScale;
		pos.v[2] = -ue_pos.X / worldToMeterScale;

		FQuat ue_ori = ue_rot.Quaternion();
		WVR_Quatf_t rot;
		rot.w = -ue_ori.W;
		rot.x = ue_ori.Y;
		rot.y = ue_ori.Z;
		rot.z = -ue_ori.X;

		WVR_Matrix4f_t mat;

		// m[0, 0] = 1 - 2 * y^2 - 2 * z^2
		mat.m[0][0] = 1 - (2 * (rot.y * rot.y)) - (2 * (rot.z * rot.z));
		// m[0, 1] = 2xy - 2zw
		mat.m[0][1] = (2 * rot.x * rot.y) - (2 * rot.z * rot.w);
		// m[0, 2] = 2xz + 2yw
		mat.m[0][2] = (2 * rot.x * rot.z) + (2 * rot.y * rot.w);
		// m[0, 3] = X
		mat.m[0][3] = pos.v[0];
		// m[1, 0] = 2xy + 2zw
		mat.m[1][0] = (2 * rot.x * rot.y) + (2 * rot.z * rot.w);
		// m[1, 1] = 1 - 2 * x^2 - 2 * z^2
		mat.m[1][1] = 1 - (2 * (rot.x * rot.x)) - (2 * (rot.z * rot.z));
		// m[1, 2] = 2yz -2xw
		mat.m[1][2] = (2 * rot.y * rot.z) - (2 * rot.x * rot.w);
		// m[1, 3] = Y
		mat.m[1][3] = pos.v[1];
		// m[2, 0] = 2xz - 2yw
		mat.m[2][0] = (2 * rot.x * rot.z) - (2 * rot.y * rot.w);
		// m[2, 1] = 2yz + 2xw
		mat.m[2][1] = (2 * rot.y * rot.z) + (2 * rot.x * rot.w);
		// m[2, 2] = 1 - 2 * x^2 - 2 * y^2
		mat.m[2][2] = 1 - (2 * rot.x * rot.x) - (2 * rot.y * rot.y);
		// m[2, 3] = Z
		mat.m[2][3] = pos.v[2];
		// m[3, 0] = 0
		mat.m[3][0] = 0;
		// m[3, 1] = 0
		mat.m[3][1] = 0;
		// m[3, 2] = 0
		mat.m[3][2] = 0;
		// m[3, 3] = 1
		mat.m[3][3] = 1;

		return mat;
	}

	float GetCachedWorldToMeterScale()
	{
		return currentWorldToMeterScale;
	}
};

typedef enum {
	Left,
	Right,
} Eye;

class FWaveVRMultiLayer
{
private:
	FWaveVRMultiLayerManager* managerInstance = nullptr;
	IRendererModule* RendererModule;
public:
	bool isDebugCanvas = false;
	bool initializationCompleted = false;
	bool isReadyForSubmit = false;

public:

	FWaveVRMultiLayer(uint32 InLayerID, const IStereoLayers::FLayerDesc& InLayerDesc, FWaveVRMultiLayerManager* InManagerInstance);
	~FWaveVRMultiLayer();

	void SetupMultiLayer(const IStereoLayers::FLayerDesc& InLayerDesc);
	void DestroyMultiLayer();
	bool GetAvaliableLayerTexture();
	bool SetTextureContent(FRHICommandListImmediate& RHICmdList);
	void GetLayerPose(const IStereoLayers::FLayerDesc& InLayerDesc, WVR_Pose_t *currentLayerPose, WVR_PoseState_t *currentLayerPoseState);
	bool AssignLayerParamsFromLayerDesc(const IStereoLayers::FLayerDesc& InLayerDesc, WVR_Pose_t *currentLayerPose, WVR_PoseState_t *currentLayerPoseState, WVR_LayerSetParams_t **OutLayerParams);
	void SubmitLayer();

	bool ReinitCheck(const IStereoLayers::FLayerDesc& InLayerDesc);
	void UnderlayMeshActorProcessor(const IStereoLayers::FLayerDesc& InLayerDesc);

private:

	UWaveVRMultiLayerResource* multilayerResource = nullptr;
	UProceduralMeshComponent* underlayAlphaMeshPtr = nullptr, * underlayClearColorMeshPtr = nullptr;
	AActor* underlayMeshBaseActor = nullptr;

	bool GenerateUnderlayMesh(TArray<FVector>& verts, TArray<int32>& tris, TArray<FVector2D>& UV0);

public:

	IStereoLayers::FLayerDesc layerDesc;

	bool toBeDestroyed = false;
	bool dynamicLayer = false;
	bool** layerTextureBlitCompleted;
	uint32 layerID, currentLayerWidth, currentLayerHeight;
	uint32_t textureQueueLengths[2];
	uint32_t currentAvaliableTextureIndex[2];
	WVR_TextureQueueHandle_t layerTextureQueueHandles[2];
	GLuint** textureIDs;
	FTexture2DRHIRef** layerTextures;

	WVR_Pose_t* currentLayerPosePtr = nullptr;
	WVR_PoseState_t* currentLayerPoseStatePtr = nullptr;
	WVR_Vector3f* layerSizeParamPtr = nullptr;
	WVR_LayerSetParams_t currentLayerParams;
	WVR_LayerSetParams_t* currentLayerParamsPtr = nullptr;

	WVR_TextureShape textureShape = WVR_TextureShape_Quad;
	const WVR_TextureTarget textureTarget = WVR_TextureTarget_2D;
	const WVR_TextureFormat textureFormat = WVR_TextureFormat_RGBA;
	const WVR_TextureType textureType = WVR_TextureType_UnsignedByte;
	const WVR_TextureOptions textureOptions = WVR_TextureOption_None;
};

class FDebugCanvasHandler
{
public:
	static FDebugCanvasHandler* GetInstance();
	void PostRenderView_RenderThread(FSceneView& InView);
	void ResetDebugCanvasOverlay() { bNeedResetDebugCanvasOverlay = true; }
	bool NeedResetDebugCanvasOverlay() { return bNeedResetDebugCanvasOverlay; }
	FTexture2DRHIRef GetDebugCanvasFlipRHIRef() { return DebugCanvasFlipRHIRef; };
	bool ShouldShowDebugCanvas() { return bShouldShowDebugCanvas; }
	void Hide();
	void Show();

private:
	FDebugCanvasHandler();
	virtual ~FDebugCanvasHandler() {}
	bool AllocateDebugCanvasFlip_RenderThread(const FRHITexture2D* DebugCanvasTexture);
	void RenderDebugCanvasFlip_RenderThread(FRHITexture2D* DebugCanvasTexture);

private:
	int32_t DebugCanvasOverlayId;
	uint32_t DebugCanvasFlipTextureId;
	bool bNeedResetDebugCanvasOverlay;
	bool bShouldShowDebugCanvas;
	uint32 DebugCanvasLayerID;
	FTexture2DRHIRef DebugCanvasFlipRHIRef;
	FRHITexture2D* DebugCanvasTexturePtr;

	friend class FWaveVRMultiLayer;
	friend class FWaveVRMultiLayerManager;
};
