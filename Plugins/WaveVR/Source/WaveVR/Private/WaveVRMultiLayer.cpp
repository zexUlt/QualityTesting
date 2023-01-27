// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRMultiLayer.h"

#include "Modules/ModuleManager.h"
#include "RHI.h"
#include "RHIUtilities.h"
#include "OpenGLDrv.h"
#include "ScreenRendering.h"
#include "ClearQuad.h"
#include "Materials/Material.h"

#include "Platforms/WaveVRLogWrapper.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "WaveVRUtils.h"
#include "WaveVRHMD.h"

#include "wvr_render.h"

using namespace wvr::utils;

#if PLATFORM_ANDROID && PLATFORM_ANDROID_ARM //(32bit)
#define PTR_TO_INT(x) x
#define INT_TO_PTR(x) x
#else //PLATFORM_ANDROID_ARM64 || PLATFORM_WINDOWS || WITH_EDITOR  /*Other 64bit*/
#define PTR_TO_INT(x) \
  (int)(long long)(x)
#define INT_TO_PTR(x) \
  (long long)(x)
#endif

DEFINE_LOG_CATEGORY_STATIC(WVR_MultiLayer, Log, All);

#pragma region Layer Manager Implementation

FWaveVRMultiLayerManager* FWaveVRMultiLayerManager::managerInstance;

FWaveVRMultiLayerManager::FWaveVRMultiLayerManager()
	:frameIndex(-1),
	NextLayerId(1)
{
	LOGD(WVR_MultiLayer, "%s", __func__);
	currentWorldToMeterScale = FWaveVRHMD::GetInstance()->GetWorldToMetersScale();
}

FWaveVRMultiLayerManager::~FWaveVRMultiLayerManager()
{
	LOGD(WVR_MultiLayer, "%s", __func__);
	MultiLayerMap.Empty();
}

uint32_t FWaveVRMultiLayerManager::GetCurrentLayerCount()
{
	LOGD(WVR_MultiLayer, "%s", __func__);
	return MultiLayerMap.Num();
}

FWaveVRMultiLayer* FWaveVRMultiLayerManager::GetWaveVRMultiLayer(uint32 InLayerID)
{
	//LOGD(WVR_MultiLayer, "%s", __func__);
	FWaveVRMultiLayer* layerPtr = *MultiLayerMap.Find(InLayerID);

	if (layerPtr)
	{
		return layerPtr;
	}

	return nullptr;
}

uint32 FWaveVRMultiLayerManager::CreateLayer(const IStereoLayers::FLayerDesc& InLayerDesc)
{
	LOGD(WVR_MultiLayer, "%s", __func__);
	uint32 LayerId = NextLayerId++;
	check(LayerId > 0);
	LOGD(WVR_MultiLayer, "CreateLayer with ID: %d", LayerId);
	FWaveVRMultiLayer* NewLayer = new FWaveVRMultiLayer(LayerId, InLayerDesc, this);
	MultiLayerMap.Emplace(LayerId, NewLayer);
	return LayerId;
}

void FWaveVRMultiLayerManager::MarkLayerToDestroy(uint32 LayerId)
{
	LOGD(WVR_MultiLayer, "%s", __func__);
	FWaveVRMultiLayer* RemovedLayer;
	if (MultiLayerMap.RemoveAndCopyValue(LayerId, RemovedLayer))
	{
		RemovedLayer->toBeDestroyed = true;

		if (RemovedLayer != nullptr && RemovedLayer->isDebugCanvas)
		{
			FDebugCanvasHandler* DebugCanvasHandler = FDebugCanvasHandler::GetInstance();
			if (DebugCanvasHandler != nullptr) {
				DebugCanvasHandler->bShouldShowDebugCanvas = false;
				DebugCanvasHandler->DebugCanvasTexturePtr = nullptr;
				DebugCanvasHandler->Hide();
			}
		}
	}
}

void FWaveVRMultiLayerManager::SetLayerDesc(uint32 LayerId, const IStereoLayers::FLayerDesc& InLayerDesc)
{
	//LOGD(WVR_MultiLayer, "%s", __func__);
	if (InLayerDesc.Id == 1)
	{
		return;
	}
	FWaveVRMultiLayer* TargetLayer = GetWaveVRMultiLayer(LayerId);
	if (TargetLayer)
	{
		if (TargetLayer->toBeDestroyed)
		{
			delete TargetLayer;
			return;
		}

		//Reinit check
		if (TargetLayer->ReinitCheck(InLayerDesc)) //Need reinit
		{
			LOGD(WVR_MultiLayer, "Layer with ID: %d needs to be reinitialized", LayerId);
			MultiLayerMap.Remove(LayerId);
			delete TargetLayer;
			FWaveVRMultiLayer* ReinitLayer = new FWaveVRMultiLayer(LayerId, InLayerDesc, this);
			MultiLayerMap.Emplace(LayerId, ReinitLayer);

			return;
		}
		
		TargetLayer->layerDesc = InLayerDesc;
		if (InLayerDesc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH)
		{
			TargetLayer->UnderlayMeshActorProcessor(InLayerDesc);
		}
	}
}

bool FWaveVRMultiLayerManager::GetLayerDesc(uint32 LayerId, IStereoLayers::FLayerDesc& OutLayerDesc)
{
	LOGD(WVR_MultiLayer, "%s", __func__);
	FWaveVRMultiLayer* TargetLayer = GetWaveVRMultiLayer(LayerId);
	if (TargetLayer)
	{
		OutLayerDesc = TargetLayer->layerDesc;
		return true;
	}
	return false;
}

void FWaveVRMultiLayerManager::UpdateTextures(FRHICommandListImmediate& RHICmdList) //Call in Render Thread
{
	check(IsInRenderingThread());
	//LOGD(WVR_MultiLayer, "%s", __func__);

	for (auto& Elem : MultiLayerMap)
	{
		FWaveVRMultiLayer* currentLayer = Elem.Value;
		if (!currentLayer)
		{
			LOGD(WVR_MultiLayer, "%s : Layer object not found.", __func__);

		}
		else if (currentLayer->isDebugCanvas)
		{
			continue;
		}
		else if (!currentLayer->initializationCompleted)
		{
			LOGD(WVR_MultiLayer, "%s : Initialization is not completed yet.", __func__);

			currentLayer->SetupMultiLayer(currentLayer->layerDesc); //Try to setup layer again
		}
		else if (currentLayer->GetAvaliableLayerTexture())
		{
			if (currentLayer->dynamicLayer)
			{
				currentLayer->layerTextureBlitCompleted[Eye::Left][currentLayer->currentAvaliableTextureIndex[Eye::Left]] = false;
				currentLayer->layerTextureBlitCompleted[Eye::Right][currentLayer->currentAvaliableTextureIndex[Eye::Right]] = false;
				currentLayer->isReadyForSubmit = currentLayer->SetTextureContent(RHICmdList);

				if (currentLayer->isReadyForSubmit)
				{
					currentLayer->layerTextureBlitCompleted[Eye::Left][currentLayer->currentAvaliableTextureIndex[Eye::Left]] = true;
					currentLayer->layerTextureBlitCompleted[Eye::Right][currentLayer->currentAvaliableTextureIndex[Eye::Right]] = true;
				}
			}
			else
			{
				if (!currentLayer->layerTextureBlitCompleted[Eye::Left][currentLayer->currentAvaliableTextureIndex[Eye::Left]] && !currentLayer->layerTextureBlitCompleted[Eye::Right][currentLayer->currentAvaliableTextureIndex[Eye::Right]])
				{
					currentLayer->isReadyForSubmit = currentLayer->SetTextureContent(RHICmdList);
					if (currentLayer->isReadyForSubmit)
					{
						currentLayer->layerTextureBlitCompleted[Eye::Left][currentLayer->currentAvaliableTextureIndex[Eye::Left]] = true;
						currentLayer->layerTextureBlitCompleted[Eye::Right][currentLayer->currentAvaliableTextureIndex[Eye::Right]] = true;
					}
				}
				else if (currentLayer->layerTextureBlitCompleted[Eye::Left][currentLayer->currentAvaliableTextureIndex[Eye::Left]] && currentLayer->layerTextureBlitCompleted[Eye::Right][currentLayer->currentAvaliableTextureIndex[Eye::Right]])
				{
					currentLayer->isReadyForSubmit = true;
				}
			}
		}

	}
}

void FWaveVRMultiLayerManager::SubmitLayers()
{
	check(IsInRenderingThread());

	//LOGD(WVR_MultiLayer, "%s : Frame Index : %d", __func__, ++frameIndex);

	for (auto& Elem : MultiLayerMap)
	{
		FWaveVRMultiLayer* currentLayer = Elem.Value;
		if (currentLayer->isDebugCanvas)
		{
			continue;
		}
		currentLayer->SubmitLayer();
	}
}

#pragma endregion

#pragma region Layer Implementation

FWaveVRMultiLayer::FWaveVRMultiLayer(uint32 InLayerID, const IStereoLayers::FLayerDesc& InLayerDesc, FWaveVRMultiLayerManager* InManagerInstance)
	:managerInstance(InManagerInstance),
	layerDesc(InLayerDesc),
	toBeDestroyed(false),
	layerID(InLayerID)
{
	LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);

	static const FName RendererModuleName("Renderer");
	RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	check(RendererModule);

	currentLayerPosePtr = new WVR_Pose_t();
	currentLayerPoseStatePtr = new WVR_PoseState_t();
	layerSizeParamPtr = new WVR_Vector3f_t();

	WVR_TextureLayout_t layout;
	layout.leftLowUVs.v[0] = InLayerDesc.UVRect.Min.X;
	layout.leftLowUVs.v[1] = InLayerDesc.UVRect.Min.Y;
	layout.rightUpUVs.v[0] = InLayerDesc.UVRect.Max.X;
	layout.rightUpUVs.v[1] = InLayerDesc.UVRect.Max.Y;

	if (InLayerDesc.HasShape<FCylinderLayer>())
	{
		textureShape = WVR_TextureShape::WVR_TextureShape_Cylinder;
	}
	else
	{
		textureShape = WVR_TextureShape::WVR_TextureShape_Quad;
	}

	WVR_LayerParams_t left = {
		WVR_Eye::WVR_Eye_Left,
		nullptr,
		textureTarget,
		layout,
		WVR_TextureOptions::WVR_TextureOption_None,
		textureShape,
		WVR_TextureLayerType::WVR_TextureLayerType_Overlay,
		0,
		nullptr,
		nullptr,
		nullptr,
		1,
		1,
		0,
		nullptr,
		nullptr
	};

	WVR_LayerParams_t right = {
		WVR_Eye::WVR_Eye_Right,
		nullptr,
		textureTarget,
		layout,
		WVR_TextureOptions::WVR_TextureOption_None,
		textureShape,
		WVR_TextureLayerType::WVR_TextureLayerType_Overlay,
		0,
		nullptr,
		nullptr,
		nullptr,
		1,
		1,
		0,
		nullptr,
		nullptr
	};

	currentLayerParams = { {left, right} };
	currentLayerParamsPtr = &currentLayerParams;

	if (InLayerDesc.Id == 1)
	{
		isDebugCanvas = true;
		FDebugCanvasHandler* DebugCanvasHandler = FDebugCanvasHandler::GetInstance();
		if (DebugCanvasHandler != nullptr) {
			DebugCanvasHandler->bShouldShowDebugCanvas = true;
			DebugCanvasHandler->DebugCanvasTexturePtr = InLayerDesc.Texture->GetTexture2D();
		}
	}
	else
	{
		SetupMultiLayer(InLayerDesc);
	}
}

FWaveVRMultiLayer::~FWaveVRMultiLayer()
{
	LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);

	DestroyMultiLayer();
	delete currentLayerPosePtr;
	delete currentLayerPoseStatePtr;
	delete layerSizeParamPtr;

	if (multilayerResource)
	{
		multilayerResource->ConditionalBeginDestroy();
		multilayerResource = nullptr;
	}

	if (underlayAlphaMeshPtr)
	{
		underlayAlphaMeshPtr->DestroyComponent();
	}

	if (underlayClearColorMeshPtr)
	{
		underlayClearColorMeshPtr->DestroyComponent();
	}

	if (underlayMeshBaseActor)
	{
		underlayMeshBaseActor->Destroy();
	}
}

void FWaveVRMultiLayer::SetupMultiLayer(const IStereoLayers::FLayerDesc& InLayerDesc)
{
	LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);
	if (!layerDesc.Texture) return; //Stop setting up layer if texture is unavailable
	FWaveVRMultiLayer* WaveVRMultiLayerRef = this;
	ENQUEUE_RENDER_COMMAND(ObtainTextureQueue) (
		[WaveVRMultiLayerRef, InLayerDesc](FRHICommandListImmediate& RHICmdList)
	{
		FRHITexture2D* layerTexture2D = WaveVRMultiLayerRef->layerDesc.Texture->GetTexture2D();
		WaveVRMultiLayerRef->currentLayerWidth = layerTexture2D->GetSizeX();
		WaveVRMultiLayerRef->currentLayerHeight = layerTexture2D->GetSizeY();

		//1. Obtain Texture Queue
		WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Left] = WVR()->ObtainTextureQueue(WaveVRMultiLayerRef->textureTarget, WaveVRMultiLayerRef->textureFormat, WaveVRMultiLayerRef->textureType, WaveVRMultiLayerRef->currentLayerWidth, WaveVRMultiLayerRef->currentLayerHeight, 0);
		WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Right] = WVR()->ObtainTextureQueue(WaveVRMultiLayerRef->textureTarget, WaveVRMultiLayerRef->textureFormat, WaveVRMultiLayerRef->textureType, WaveVRMultiLayerRef->currentLayerWidth, WaveVRMultiLayerRef->currentLayerHeight, 0);

		if (WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Left] && WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Right])
		{
			LOGD(WVR_MultiLayer, "ObtainTextureQueue Left: Handle: %p Width: %u, Height: %u", WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Left], WaveVRMultiLayerRef->currentLayerWidth, WaveVRMultiLayerRef->currentLayerHeight);
			LOGD(WVR_MultiLayer, "ObtainTextureQueue Right: Handle: %p Width: %u, Height: %u", WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Right], WaveVRMultiLayerRef->currentLayerWidth, WaveVRMultiLayerRef->currentLayerHeight);

			//2. Get Texture Queue Length
			WaveVRMultiLayerRef->textureQueueLengths[Eye::Left] = WVR()->GetTextureQueueLength(WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Left]);
			WaveVRMultiLayerRef->textureQueueLengths[Eye::Right] = WVR()->GetTextureQueueLength(WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Right]);
			LOGD(WVR_MultiLayer, "ObtainTextureQueue Left: Length: %u", WaveVRMultiLayerRef->textureQueueLengths[Eye::Left]);
			LOGD(WVR_MultiLayer, "ObtainTextureQueue Right: Length: %u", WaveVRMultiLayerRef->textureQueueLengths[Eye::Right]);

			//3. Setup Texture ID arrays according to Texture Queue Length
			WaveVRMultiLayerRef->textureIDs = new GLuint*[2];
			WaveVRMultiLayerRef->textureIDs[Eye::Left] = new GLuint[WaveVRMultiLayerRef->textureQueueLengths[Eye::Left]];
			WaveVRMultiLayerRef->textureIDs[Eye::Right] = new GLuint[WaveVRMultiLayerRef->textureQueueLengths[Eye::Right]];

			//4. Setup layer texture arrays
			WaveVRMultiLayerRef->layerTextures = new FTexture2DRHIRef*[2];
			WaveVRMultiLayerRef->layerTextures[Eye::Left] = new FTexture2DRHIRef[WaveVRMultiLayerRef->textureQueueLengths[Eye::Left]];
			WaveVRMultiLayerRef->layerTextures[Eye::Right] = new FTexture2DRHIRef[WaveVRMultiLayerRef->textureQueueLengths[Eye::Right]];
			
			//5. Optional: Setup Blit Status for static layers
			WaveVRMultiLayerRef->dynamicLayer = InLayerDesc.Flags & IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE;

			WaveVRMultiLayerRef->layerTextureBlitCompleted = new bool*[2];
			WaveVRMultiLayerRef->layerTextureBlitCompleted[Eye::Left] = new bool[WaveVRMultiLayerRef->textureQueueLengths[Eye::Left]]{ false };
			WaveVRMultiLayerRef->layerTextureBlitCompleted[Eye::Right] = new bool[WaveVRMultiLayerRef->textureQueueLengths[Eye::Right]]{ false };


			WaveVRMultiLayerRef->initializationCompleted = true;
		}
		else
		{
			LOGD(WVR_MultiLayer, "ObtainTextureQueue failed");
		}
	});
}

void FWaveVRMultiLayer::DestroyMultiLayer()
{
	LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);
	FWaveVRMultiLayer* WaveVRMultiLayerRef = this;
	ENQUEUE_RENDER_COMMAND(ReleaseTextureQueue) (
		[WaveVRMultiLayerRef](FRHICommandListImmediate& RHICmdList)
	{
		if (WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Left] && WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Right])
		{
			WVR()->ReleaseTextureQueue(WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Left]);
			WVR()->ReleaseTextureQueue(WaveVRMultiLayerRef->layerTextureQueueHandles[Eye::Right]);
			LOGD(WVR_MultiLayer, "Texture queue released");
		}
	});
}

bool FWaveVRMultiLayer::GetAvaliableLayerTexture()
{
	//LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);
	if (!initializationCompleted) return false;

	//TODO: Check texture size change

	//Get avaliable texture index
	for (int eyeIndex = 0; eyeIndex < 2; eyeIndex++)
	{
		if (layerTextureQueueHandles[eyeIndex])
		{
			currentAvaliableTextureIndex[eyeIndex] = WVR()->GetAvailableTextureIndex(layerTextureQueueHandles[eyeIndex]);

			if (currentAvaliableTextureIndex[eyeIndex] == -1)
			{
				LOGD(WVR_MultiLayer, "GetAvaliableLayerTexture: There are no available textures. LayerID: %u", layerID);
				return false;
			}

			//LOGD(WVR_MultiLayer, "GetAvaliableLayerTexture: Available Index: %u", currentAvaliableTextureIndex[eyeIndex]);

			bool textureIDUpdated = false;
			WVR_TextureParams_t currentAvaliableTextureParams = WVR()->GetTexture(layerTextureQueueHandles[eyeIndex], currentAvaliableTextureIndex[eyeIndex]);
			GLuint existingTextureID = textureIDs[eyeIndex][currentAvaliableTextureIndex[eyeIndex]];
			GLuint currentAvaliableTextureID = (GLuint)PTR_TO_INT(currentAvaliableTextureParams.id);
			if (!existingTextureID || existingTextureID != currentAvaliableTextureID)
			{
				//LOGD(WVR_MultiLayer, "GetAvaliableLayerTexture: Available Texture ID: %u, Pointer: %p", currentAvaliableTextureID, currentAvaliableTextureParams.id);
				textureIDs[eyeIndex][currentAvaliableTextureIndex[eyeIndex]] = currentAvaliableTextureID;
				textureIDUpdated = true;
			}

			//Create Texture 2D from id
			if (textureIDUpdated || !layerTextures[eyeIndex][currentAvaliableTextureIndex[eyeIndex]])
			{
				auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
				check(GLRHI);

				layerTextures[eyeIndex][currentAvaliableTextureIndex[eyeIndex]] = GLRHI->RHICreateTexture2DFromResource(
					EPixelFormat::PF_B8G8R8A8, currentLayerWidth, currentLayerHeight,
					1/*Mips*/, 1, 1/*SamplesTileMem*/,
					FClearValueBinding::Black, textureIDs[eyeIndex][currentAvaliableTextureIndex[eyeIndex]], TexCreate_SRGB);

				if (!layerTextures[eyeIndex][currentAvaliableTextureIndex[eyeIndex]])
				{
					LOGD(WVR_MultiLayer, "GetAvaliableLayerTexture: Failed to create Texture from resource. LayerID: %u", layerID);
					return false;
				}
			}
		}
	}

	return true;
}

bool FWaveVRMultiLayer::SetTextureContent(FRHICommandListImmediate& RHICmdList) //Call in Render Thread
{
	check(IsInRenderingThread());

	//LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);

	FRHITexture* SrcTexture[2] = { layerDesc.LeftTexture ? layerDesc.LeftTexture : layerDesc.Texture, layerDesc.Texture };
	if (!SrcTexture[0] || !SrcTexture[1])
	{
		LOGD(WVR_MultiLayer, "%s, LayerID: %u, no SrcTextures", __func__, layerID);
		return false;
	}
	FRHITexture2D* SrcTexture2D[2] = { SrcTexture[Eye::Left]->GetTexture2D(), SrcTexture[Eye::Right]->GetTexture2D() };
	FRHITexture2D* DstTexture2D[2] = { layerTextures[Eye::Left][currentAvaliableTextureIndex[Eye::Left]], layerTextures[Eye::Right][currentAvaliableTextureIndex[Eye::Right]] };

	for (int eyeIndex = 0; eyeIndex < 2; eyeIndex++)
	{
		FIntPoint DstSize;
		FIntPoint SrcSize;

		if (DstTexture2D[eyeIndex] && SrcTexture2D[eyeIndex])
		{
			DstSize = FIntPoint(DstTexture2D[eyeIndex]->GetSizeX(), DstTexture2D[eyeIndex]->GetSizeY());
			SrcSize = FIntPoint(SrcTexture2D[eyeIndex]->GetSizeX(), SrcTexture2D[eyeIndex]->GetSizeY());
		}
		else
		{
			LOGD(WVR_MultiLayer, "Textures not found. LayerID: %u", layerID);
			return false;
		}

		FIntRect DstRect = FIntRect(FIntPoint::ZeroValue, DstSize);
		FIntRect SrcRect = FIntRect(FIntPoint::ZeroValue, SrcSize);

		const uint32 ViewportWidth = DstRect.Width();
		const uint32 ViewportHeight = DstRect.Height();
		const FIntPoint TargetSize(ViewportWidth, ViewportHeight);
		float U = SrcRect.Min.X / (float)SrcSize.X;
		float V = SrcRect.Min.Y / (float)SrcSize.Y;
		float USize = SrcRect.Width() / (float)SrcSize.X;
		float VSize = SrcRect.Height() / (float)SrcSize.Y;
		RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);

		FRHIRenderPassInfo RPInfo(DstTexture2D[eyeIndex], ERenderTargetActions::DontLoad_Store);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("BlitMultiLayerTexture"));
		{
			DrawClearQuad(RHICmdList, FLinearColor::Transparent);

			if (!isDebugCanvas)
			{
				// Y flip for Android
				V = 1.0f - V;
				VSize = -VSize;
			}

			FRHITexture* SrcTextureRHI = SrcTexture[eyeIndex];
			auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
			TShaderMapRef<FScreenPSsRGBSource> PixelShader(ShaderMap);
			//TShaderMapRef<FScreenPS> PixelShader(ShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

			//GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, true, false>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			FRHISamplerState* SamplerState = DstRect.Size() == SrcRect.Size() ? TStaticSamplerState<SF_Point>::GetRHI() : TStaticSamplerState<SF_Bilinear>::GetRHI();
			PixelShader->SetParameters(RHICmdList, SamplerState, SrcTextureRHI);

			//LOGD(WVR_MultiLayer, "%s: DstRect: %d %d %d %d", __func__, DstRect.Min.X, DstRect.Min.Y, DstRect.Max.X, DstRect.Max.Y);

			RendererModule->DrawRectangle(
				RHICmdList,
				0, 0, ViewportWidth, ViewportHeight,
				U, V, USize, VSize,
				TargetSize,
				FIntPoint(1, 1),
				VertexShader,
				EDRF_Default);
		}
		RHICmdList.EndRenderPass();
	}

	return true;
}

void FWaveVRMultiLayer::GetLayerPose(const IStereoLayers::FLayerDesc& InLayerDesc, WVR_Pose_t *currentLayerPose, WVR_PoseState_t *currentLayerPoseState)
{
	//LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);
	switch (InLayerDesc.PositionType)
	{
	case IStereoLayers::ELayerType::TrackerLocked: //Not supported, fall back to World Lock
	case IStereoLayers::ELayerType::WorldLocked:
	{
		currentLayerPose->position = FWaveVRMultiLayerManager::GetVector3(InLayerDesc.Transform.GetLocation(), managerInstance->GetCachedWorldToMeterScale());
		currentLayerPose->rotation = FWaveVRMultiLayerManager::GetQuaternion(InLayerDesc.Transform.GetRotation());

		FQuat* poseStateRot = new FQuat();
		FVector* poseStateTranslation = new FVector();
		if (FWaveVRHMD::GetInstance()->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, *poseStateRot, *poseStateTranslation))
		{
			FTransform poseStateTranform;
			FVector poseStateScale;
			poseStateScale.X = 1;
			poseStateScale.Y = 1;
			poseStateScale.Z = 1;
			poseStateTranform.SetComponents(*poseStateRot, *poseStateTranslation, poseStateScale);

			FTransform trackingToWorldTransform = FWaveVRHMD::GetInstance()->GetTrackingToWorldTransform();
			FTransform::Multiply(&poseStateTranform, &poseStateTranform, &trackingToWorldTransform);
			FRotator poseStateRotator = poseStateTranform.GetRotation().Rotator();
			*poseStateTranslation = poseStateTranform.GetLocation();

			currentLayerPoseState->isValidPose = true;
			currentLayerPoseState->poseMatrix = FWaveVRMultiLayerManager::GetMatrix44(*poseStateTranslation, poseStateRotator, 100);
		}
		else
		{
			WVR()->GetPoseState(WVR_DeviceType::WVR_DeviceType_HMD, WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnGround, 0, currentLayerPoseState);
			currentLayerPoseState->isValidPose = true;
		}

		break;
	}
	case IStereoLayers::ELayerType::FaceLocked:
	{
		currentLayerPose->position = FWaveVRMultiLayerManager::GetVector3(InLayerDesc.Transform.GetLocation(), managerInstance->GetCachedWorldToMeterScale());
		currentLayerPose->rotation = FWaveVRMultiLayerManager::GetQuaternion(InLayerDesc.Transform.GetRotation());
		break;
	}
	}

	//LOGD(WVR_MultiLayer, "%s, LayerID: %u Pos: V3 %f %f %f", __func__, layerID, currentLayerPose->position.v[0], currentLayerPose->position.v[1], currentLayerPose->position.v[2]);
	//LOGD(WVR_MultiLayer, "%s, LayerID: %u Rot: Q4 %f %f %f %f", __func__, layerID, currentLayerPose->rotation.w, currentLayerPose->rotation.x, currentLayerPose->rotation.y, currentLayerPose->rotation.z);
}

bool FWaveVRMultiLayer::AssignLayerParamsFromLayerDesc(const IStereoLayers::FLayerDesc& InLayerDesc, WVR_Pose_t *currentLayerPose, WVR_PoseState_t *currentLayerPoseState, WVR_LayerSetParams_t **OutLayerParams)
{
	//LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);

	if (!currentLayerPose || !currentLayerPoseState)
	{
		return false;
	}

	for (int eyeIndex = 0; eyeIndex < 2; eyeIndex++)
	{
		currentLayerParamsPtr->textures[eyeIndex].eye = (WVR_Eye)eyeIndex;
		//LOGD(WVR_MultiLayer, "%s, Eye: %d", __func__, (int)currentLayerParamsPtr->textures[eyeIndex].eye);
		if (textureIDs[eyeIndex][currentAvaliableTextureIndex[eyeIndex]] <= 0) //invalid texture id
		{
			return false;
		}
		currentLayerParamsPtr->textures[eyeIndex].id = (WVR_Texture_t)INT_TO_PTR(textureIDs[eyeIndex][currentAvaliableTextureIndex[eyeIndex]]);
		//LOGD(WVR_MultiLayer, "%s, Texture id: %p", __func__, currentLayerParamsPtr->textures[eyeIndex].id);
		currentLayerParamsPtr->textures[eyeIndex].target = textureTarget;
		currentLayerParamsPtr->textures[eyeIndex].layout.leftLowUVs.v[0] = InLayerDesc.UVRect.Min.X;
		currentLayerParamsPtr->textures[eyeIndex].layout.leftLowUVs.v[1] = InLayerDesc.UVRect.Min.Y;
		currentLayerParamsPtr->textures[eyeIndex].layout.rightUpUVs.v[0] = InLayerDesc.UVRect.Max.X;
		currentLayerParamsPtr->textures[eyeIndex].layout.rightUpUVs.v[1] = InLayerDesc.UVRect.Max.Y;
		//LOGD(WVR_MultiLayer, "%s, Layout: %f %f %f %f", __func__, currentLayerParamsPtr->textures[eyeIndex].layout.leftLowUVs.v[0], currentLayerParamsPtr->textures[eyeIndex].layout.leftLowUVs.v[1], currentLayerParamsPtr->textures[eyeIndex].layout.rightUpUVs.v[0], currentLayerParamsPtr->textures[eyeIndex].layout.rightUpUVs.v[1]);

		switch (InLayerDesc.PositionType)
		{
		case IStereoLayers::ELayerType::TrackerLocked: //Not supported, fall back to World Lock
		case IStereoLayers::ELayerType::WorldLocked:
		{
			//LOGD(WVR_MultiLayer, "%s, WorldLock", __func__);
			currentLayerParamsPtr->textures[eyeIndex].opts = WVR_TextureOptions::WVR_TextureOption_None;
			currentLayerParamsPtr->textures[eyeIndex].pose = currentLayerPoseState;
			break;
		}
		case IStereoLayers::ELayerType::FaceLocked:
		{
			//LOGD(WVR_MultiLayer, "%s, HeadLock", __func__);
			currentLayerParamsPtr->textures[eyeIndex].opts = WVR_TextureOptions::WVR_TextureOption_HeadLocked;
			break;
		}
		}

		if (textureShape == WVR_TextureShape::WVR_TextureShape_Cylinder)
		{
			const FCylinderLayer& CylinderProps = InLayerDesc.GetShape<FCylinderLayer>();

			//LOGD(WVR_MultiLayer, "%s, LayerID: %u CylinderLayer", __func__, layerID);
			currentLayerParamsPtr->textures[eyeIndex].shape = WVR_TextureShape::WVR_TextureShape_Cylinder;
			layerSizeParamPtr->v[0] = CylinderProps.OverlayArc / managerInstance->GetCachedWorldToMeterScale(); //Arc Length
			layerSizeParamPtr->v[1] = CylinderProps.Height / managerInstance->GetCachedWorldToMeterScale(); //Height
			layerSizeParamPtr->v[2] = CylinderProps.Radius / managerInstance->GetCachedWorldToMeterScale(); //Radius
		}
		else //Quad and other shapes
		{
			//LOGD(WVR_MultiLayer, "%s, LayerID: %u QuadLayer", __func__, layerID);
			currentLayerParamsPtr->textures[eyeIndex].shape = WVR_TextureShape::WVR_TextureShape_Quad;
			layerSizeParamPtr->v[0] = InLayerDesc.QuadSize.X / managerInstance->GetCachedWorldToMeterScale(); //Width
			layerSizeParamPtr->v[1] = InLayerDesc.QuadSize.Y / managerInstance->GetCachedWorldToMeterScale(); //Height
			layerSizeParamPtr->v[2] = 0;
		}

		currentLayerParamsPtr->textures[eyeIndex].size = layerSizeParamPtr;

		currentLayerParamsPtr->textures[eyeIndex].type = (InLayerDesc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH) ? WVR_TextureLayerType::WVR_TextureLayerType_Underlay : WVR_TextureLayerType::WVR_TextureLayerType_Overlay;
		currentLayerParamsPtr->textures[eyeIndex].compositionDepth = InLayerDesc.Priority;
		currentLayerParamsPtr->textures[eyeIndex].poseTransform = currentLayerPose;
		currentLayerParamsPtr->textures[eyeIndex].width = currentLayerWidth;
		currentLayerParamsPtr->textures[eyeIndex].height = currentLayerHeight;
		currentLayerParamsPtr->textures[eyeIndex].depth = nullptr;
		currentLayerParamsPtr->textures[eyeIndex].projectionMatrix = nullptr;

		//LOGD(WVR_MultiLayer, "%s, LayerID: %u Pos: V3 %f %f %f", __func__, layerID, currentLayerParamsPtr->textures[eyeIndex].poseTransform->position.v[0], currentLayerParamsPtr->textures[eyeIndex].poseTransform->position.v[1], currentLayerParamsPtr->textures[eyeIndex].poseTransform->position.v[2]);
		//LOGD(WVR_MultiLayer, "%s, LayerID: %u Rot: Q4 %f %f %f %f", __func__, layerID, currentLayerParamsPtr->textures[eyeIndex].poseTransform->rotation.w, currentLayerParamsPtr->textures[eyeIndex].poseTransform->rotation.x, currentLayerParamsPtr->textures[eyeIndex].poseTransform->rotation.y, currentLayerParamsPtr->textures[eyeIndex].poseTransform->rotation.z);
		//LOGD(WVR_MultiLayer, "%s, compositionDepth %u", __func__, currentLayerParamsPtr->textures[eyeIndex].compositionDepth);
		//LOGD(WVR_MultiLayer, "%s, LayerID: %u width %u height %u", __func__, layerID, currentLayerParamsPtr->textures[eyeIndex].width, currentLayerParamsPtr->textures[eyeIndex].height);
		//LOGD(WVR_MultiLayer, "%s, LayerID: %u SizeX %f SizeY %f", __func__, layerID, currentLayerParamsPtr->textures[eyeIndex].size->v[0], currentLayerParamsPtr->textures[eyeIndex].size->v[1]);
	}

	*OutLayerParams = currentLayerParamsPtr;

	return true;
}

void FWaveVRMultiLayer::SubmitLayer() //Call in Render Thread
{
	check(IsInRenderingThread());
	//LOGD(WVR_MultiLayer, "%s, LayerID: %u", __func__, layerID);
	if (!(initializationCompleted && isReadyForSubmit))
	{
		//Only submit if layer in initialized and ready for submit
		return;
	}
	GetLayerPose(layerDesc, currentLayerPosePtr, currentLayerPoseStatePtr);
	WVR_LayerSetParams_t* submitParams = nullptr;
	if (AssignLayerParamsFromLayerDesc(layerDesc, currentLayerPosePtr, currentLayerPoseStatePtr, &submitParams))
	{
		WVR_SubmitError result = WVR()->SubmitCompositionLayers(submitParams);

		LOGD(WVR_MultiLayer, "WaveVRMultiLayer::SubmitLayer() Result: %d", result);
	}
	isReadyForSubmit = false;
}

bool FWaveVRMultiLayer::ReinitCheck(const IStereoLayers::FLayerDesc& InLayerDesc)
{
	FIntPoint currTextureSize_R = layerDesc.Texture.IsValid() ? layerDesc.Texture->GetTexture2D()->GetSizeXY() : FIntPoint::ZeroValue;
	FIntPoint newTextureSize_R = InLayerDesc.Texture.IsValid() ? InLayerDesc.Texture->GetTexture2D()->GetSizeXY() : FIntPoint::ZeroValue;

	FIntPoint currTextureSize_L = layerDesc.LeftTexture.IsValid() ? layerDesc.LeftTexture->GetTexture2D()->GetSizeXY() : FIntPoint::ZeroValue;
	FIntPoint newTextureSize_L = InLayerDesc.LeftTexture.IsValid() ? InLayerDesc.LeftTexture->GetTexture2D()->GetSizeXY() : FIntPoint::ZeroValue;

	bool textureSizeChanged =  (currTextureSize_R.X != newTextureSize_R.X ||
								currTextureSize_R.Y != newTextureSize_R.Y ||
								currTextureSize_L.X != newTextureSize_L.X ||
								currTextureSize_L.Y != newTextureSize_L.Y);

	WVR_TextureShape currentShape, newShape;
	if (layerDesc.HasShape<FCylinderLayer>())
	{
		currentShape = WVR_TextureShape::WVR_TextureShape_Cylinder;
	}
	else
	{
		currentShape = WVR_TextureShape::WVR_TextureShape_Quad;
	}

	if (InLayerDesc.HasShape<FCylinderLayer>())
	{
		newShape = WVR_TextureShape::WVR_TextureShape_Cylinder;
	}
	else
	{
		newShape = WVR_TextureShape::WVR_TextureShape_Quad;
	}

	bool isShapeChanged = (currentShape != newShape);

	bool isShapeParamChanged = false;

	if (!isShapeChanged) //Check shape params if shape is not changed
	{
		if (currentShape == WVR_TextureShape::WVR_TextureShape_Cylinder)
		{
			const FCylinderLayer& currCylinderProps = layerDesc.GetShape<FCylinderLayer>();
			const FCylinderLayer& newCylinderProps = InLayerDesc.GetShape<FCylinderLayer>();

			isShapeParamChanged = (currCylinderProps.Height != newCylinderProps.Height || currCylinderProps.OverlayArc != newCylinderProps.OverlayArc || currCylinderProps.Radius != newCylinderProps.Radius);
		}
		else
		{
			isShapeParamChanged = (layerDesc.QuadSize.X != InLayerDesc.QuadSize.X || layerDesc.QuadSize.Y != InLayerDesc.QuadSize.Y);
		}
	}

	return (textureSizeChanged || isShapeChanged || isShapeParamChanged);
}

void FWaveVRMultiLayer::UnderlayMeshActorProcessor(const IStereoLayers::FLayerDesc& InLayerDesc)
{
	FString underlayAlphaComponentNameStr = FString::Printf(TEXT("WaveUnderlayAlpha_%d"), InLayerDesc.Id);
	FString underlayClearColorComponentNameStr = FString::Printf(TEXT("WaveUnderlayClearColor_%d"), InLayerDesc.Id);

	FName underlayAlphaComponentName(*underlayAlphaComponentNameStr);
	FName underlayClearColorComponentName(*underlayClearColorComponentNameStr);

	//LOGD(WVR_MultiLayer, "%s Alpha Comp %s", __func__, TCHAR_TO_UTF8(*underlayAlphaComponentNameStr));
	//LOGD(WVR_MultiLayer, "%s CC Comp %s", __func__, TCHAR_TO_UTF8(*underlayClearColorComponentNameStr));

	if (!underlayMeshBaseActor) //Create Actor
	{
		//Spawn Actor
		UWorld* currentWorld = nullptr;
		for (const FWorldContext& worldContext : GEngine->GetWorldContexts())
		{
			if (worldContext.WorldType == EWorldType::Game || worldContext.WorldType == EWorldType::PIE)
			{
				currentWorld = worldContext.World();
			}
		}

		if (!currentWorld)
		{
			LOGD(WVR_MultiLayer, "%s world not found", __func__);
			return;
		}

		//LOGD(WVR_MultiLayer, "%s world found", __func__);
		underlayMeshBaseActor = currentWorld->SpawnActor<AActor>();

	}

	if (!underlayAlphaMeshPtr || !underlayClearColorMeshPtr)
	{
		//Create Components
		underlayAlphaMeshPtr = NewObject<UProceduralMeshComponent>(underlayMeshBaseActor, underlayAlphaComponentName);
		underlayAlphaMeshPtr->RegisterComponent();
		underlayClearColorMeshPtr = NewObject<UProceduralMeshComponent>(underlayMeshBaseActor, underlayClearColorComponentName);
		underlayClearColorMeshPtr->RegisterComponent();

		//LOGD(WVR_MultiLayer, "%s Components created", __func__);

		//Create Mesh
		TArray<FVector> verts;
		TArray<int32> tris;
		TArray<FVector> norms;
		TArray<FVector2D> UV0;
		TArray<FLinearColor> vertCol;
		TArray<FProcMeshTangent> tans;

		if (!GenerateUnderlayMesh(verts, tris, UV0))
		{
			LOGD(WVR_MultiLayer, "GenerateUnderlayMesh failed.");
		}

		underlayAlphaMeshPtr->CreateMeshSection_LinearColor(0, verts, tris, norms, UV0, vertCol, tans, false);
		underlayClearColorMeshPtr->CreateMeshSection_LinearColor(0, verts, tris, norms, UV0, vertCol, tans, false);

		//LOGD(WVR_MultiLayer, "%s mesh created", __func__);

		multilayerResource = NewObject<UWaveVRMultiLayerResource>();
		multilayerResource->AddToRoot();

		UMaterialInstanceDynamic* underlayAlphaMaterialDynamic = nullptr, * underlayColorClearMaterialDynamic = nullptr;

		//Create materials
		if (multilayerResource->underlayAlphaMaterial)
		{
			underlayAlphaMaterialDynamic = UMaterialInstanceDynamic::Create(multilayerResource->underlayAlphaMaterial, NULL);
		}
		else
		{
			LOGD(WVR_MultiLayer, "underlayAlphaMaterial UNAVAILABLE");
			return;
		}


		if (multilayerResource->underlayColorClearMaterial)
		{
			underlayColorClearMaterialDynamic = UMaterialInstanceDynamic::Create(multilayerResource->underlayColorClearMaterial, NULL);
		}
		else
		{
			LOGD(WVR_MultiLayer, "underlayColorClearMaterial UNAVAILABLE");
			return;
		}

		//LOGD(WVR_MultiLayer, "%s materials created", __func__);

		underlayAlphaMeshPtr->SetMaterial(0, underlayAlphaMaterialDynamic);
		underlayClearColorMeshPtr->SetMaterial(0, underlayColorClearMaterialDynamic);
	}

	if (underlayAlphaMeshPtr)
	{
		underlayAlphaMeshPtr->SetWorldTransform(layerDesc.Transform);
	}

	if (underlayClearColorMeshPtr)
	{
		underlayClearColorMeshPtr->SetWorldTransform(layerDesc.Transform);
	}
}

bool FWaveVRMultiLayer::GenerateUnderlayMesh(TArray<FVector>& verts, TArray<int32>& tris, TArray<FVector2D>& UV0)
{
	const float underlayMeshScale = 0.99f;

	if (layerDesc.HasShape<FQuadLayer>())
	{
		verts.Init(FVector::ZeroVector, 4);
		verts[0] = FVector(0.0, -layerDesc.QuadSize.X / 2, -layerDesc.QuadSize.Y / 2) * underlayMeshScale;
		verts[1] = FVector(0.0, layerDesc.QuadSize.X / 2, -layerDesc.QuadSize.Y / 2) * underlayMeshScale;
		verts[2] = FVector(0.0, -layerDesc.QuadSize.X / 2, layerDesc.QuadSize.Y / 2) * underlayMeshScale;
		verts[3] = FVector(0.0, layerDesc.QuadSize.X / 2, layerDesc.QuadSize.Y / 2) * underlayMeshScale;

		UV0.Init(FVector2D::ZeroVector, 4);
		UV0[0] = FVector2D(0, 0);
		UV0[1] = FVector2D(0, 1);
		UV0[2] = FVector2D(1, 0);
		UV0[3] = FVector2D(1, 1);

		tris.Reserve(6);
		tris.Add(0);
		tris.Add(1);
		tris.Add(2);

		tris.Add(1);
		tris.Add(3);
		tris.Add(2);
	}
	else if (layerDesc.HasShape<FCylinderLayer>())
	{
		const FCylinderLayer& cylinderParams = layerDesc.GetShape<FCylinderLayer>();

		//Generate Vertices
		float arcAngleDeg = FMath::RadiansToDegrees(cylinderParams.OverlayArc / cylinderParams.Radius);

		float angleUpperLimitDeg = arcAngleDeg / 2; //Degrees
		float angleLowerLimitDeg = -angleUpperLimitDeg; //Degrees

		float angleLowerLimitRad = FMath::DegreesToRadians(angleLowerLimitDeg); //Radians

		int arcSegments = FMath::RoundToInt(arcAngleDeg / 5);

		float anglePerArcSegmentRad = FMath::DegreesToRadians(arcAngleDeg / arcSegments);

		verts.Init(FVector::ZeroVector, 2 * (arcSegments + 1)); //Top and bottom lines * Vertex count per line

		int vertCount = 0;
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < arcSegments + 1; j++)
			{
				float currentVertexAngleRad = angleLowerLimitRad + anglePerArcSegmentRad * j;
				float y = cylinderParams.Radius * FMath::Sin(currentVertexAngleRad);
				float z = 0;
				float x = cylinderParams.Radius * FMath::Cos(currentVertexAngleRad);

				if (i == 1) //Top
				{
					z = z + cylinderParams.Height / 2;

				}
				else //Bottom
				{
					z = z - cylinderParams.Height / 2;
				}

				verts[vertCount] = FVector(x, y, z) * underlayMeshScale;
				vertCount++;
			}
		}

		//Generate Triangles
		tris.Reserve(arcSegments * 6);
		for (int currentTriangleIndex = 0, currentVertexIndex = 0, y = 0; y < 1; y++, currentVertexIndex++)
		{
			for (int x = 0; x < arcSegments; x++, currentTriangleIndex += 6, currentVertexIndex++)
			{
				tris.Add(currentVertexIndex); //0
				tris.Add(currentVertexIndex + 1); //1
				tris.Add(currentVertexIndex + arcSegments + 1); //2

				tris.Add(currentVertexIndex + 1); //1
				tris.Add(currentVertexIndex + arcSegments + 2); //3
				tris.Add(currentVertexIndex + arcSegments + 1); //2
			}
		}

		UV0.Init(FVector2D::ZeroVector, 4);
		UV0[0] = FVector2D(0, 0);
		UV0[1] = FVector2D(0, 1);
		UV0[2] = FVector2D(1, 0);
		UV0[3] = FVector2D(1, 1);
	}
	else
	{
		LOGD(WVR_MultiLayer, "GenerateUnderlayMesh: invalid shape");
		return false;
	}

	return true;
}

#pragma endregion

#pragma region Debug Canvas Implementation

FDebugCanvasHandler* FDebugCanvasHandler::GetInstance()
{
	LOG_FUNC();
	static FDebugCanvasHandler* mInst = new FDebugCanvasHandler();
	return mInst;
}

FDebugCanvasHandler::FDebugCanvasHandler()
	: DebugCanvasOverlayId(-1)
	, DebugCanvasFlipTextureId(-1)
	, bNeedResetDebugCanvasOverlay(true)
	, bShouldShowDebugCanvas(false)
	, DebugCanvasLayerID(0)
{
}

void FDebugCanvasHandler::PostRenderView_RenderThread(FSceneView& InView)
{
	LOG_FUNC();

	FWaveVRMultiLayerManager* WaveVRMultiLayerManager = FWaveVRMultiLayerManager::GetInstance();
	FWaveVRHMD* hmd = FWaveVRHMD::GetInstance();

	if (InView.StereoPass == EStereoscopicPass::eSSP_RIGHT_EYE || (hmd != nullptr && !hmd->IsRenderInitialized()) || WaveVRMultiLayerManager == nullptr || !bShouldShowDebugCanvas)
		return;

	if (bShouldShowDebugCanvas && (!WVR()->IsOverlayValid(DebugCanvasOverlayId) || bNeedResetDebugCanvasOverlay))
	{
		WVR()->GenOverlay(&DebugCanvasOverlayId);
		LOGD(WVR_MultiLayer, "Generate DebugCanvasOverlayId(%d)", DebugCanvasOverlayId);
	}

	if(WVR()->IsInputFocusCapturedBySystem()) {
		Hide();
		return;
	} else if(bShouldShowDebugCanvas) {
		Show();
	}

	IStereoLayers::FLayerDesc LayerDesc;
	if (DebugCanvasTexturePtr->IsValid()) {
		if (!DebugCanvasFlipRHIRef.IsValid() || bNeedResetDebugCanvasOverlay) {
			LOGD(WVR_MultiLayer, "PostRenderView_RenderThread: DebugCanvasFlipRHIRef is not valid, now allocate");
			AllocateDebugCanvasFlip_RenderThread(DebugCanvasTexturePtr); //Generate DebugCanvasFlipRHIRef and DebugCanvasFlipTextureId

			uint32_t textureWidth = DebugCanvasFlipRHIRef->GetSizeX();
			uint32_t textureHeight = DebugCanvasFlipRHIRef->GetSizeY();
			const WVR_OverlayPosition position = {0.0f, 0.0f, -0.9f};
			const WVR_OverlayTexture_t texture = { (uint32_t) DebugCanvasFlipTextureId, textureWidth, textureHeight };
			LOGI(WVR_MultiLayer, "DebugCanvasFlipTextureId(%u) textureWidth(%d) textureHeight(%d) DebugCanvasOverlayId(%u)", DebugCanvasFlipTextureId, textureWidth, textureHeight, DebugCanvasOverlayId);
			WVR()->SetOverlayTextureId(DebugCanvasOverlayId, &texture);
			WVR()->SetOverlayFixedPosition(DebugCanvasOverlayId, &position);
			WVR()->ShowOverlay(DebugCanvasOverlayId);
			bNeedResetDebugCanvasOverlay = false;
		}
		//LOGI(WVR_MultiLayer, "[DEBUG] UnrealDebugCanvasTextureId(%u)", static_cast<uint32_t>(*(GLuint*)DebugCanvasTexturePtr->GetNativeResource()) );
		RenderDebugCanvasFlip_RenderThread(DebugCanvasTexturePtr);
	}
}

bool FDebugCanvasHandler::AllocateDebugCanvasFlip_RenderThread(const FRHITexture2D* DebugCanvasTexture)
{
	bool ret = false;

	auto OpenGLDynamicRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
	check(OpenGLDynamicRHI);

	uint32 InSizeX = DebugCanvasTexture->GetSizeX();
	uint32 InSizeY = DebugCanvasTexture->GetSizeY();

	EPixelFormat InFormat = EPixelFormat::PF_A2B10G10R10;
	uint32 InNumMips = 1;
	uint32 InNumSamples = 1;
	ETextureCreateFlags InFlags = TexCreate_RenderTargetable;//0;
	FRHIResourceCreateInfo CreateInfo;
	DebugCanvasFlipRHIRef = OpenGLDynamicRHI->RHICreateTexture2D(InSizeX, InSizeY, (uint8)InFormat, InNumMips, InNumSamples, InFlags, ERHIAccess::CopySrc, CreateInfo);

	if (DebugCanvasFlipRHIRef.IsValid()) {
		DebugCanvasFlipTextureId = static_cast<uint32_t>(*(GLuint*)DebugCanvasFlipRHIRef->GetNativeResource());
		LOGI(WVR_MultiLayer, "DebugCanvasFlipTextureId(%u) SizeX(%u) SizeY(%u)", DebugCanvasFlipTextureId, InSizeX, InSizeY);
		ret = true;
	} else {
		LOGW(WVR_MultiLayer, "DebugCanvasFlipTexture should not be invalid!");
	}
	return ret;
}

void FDebugCanvasHandler::RenderDebugCanvasFlip_RenderThread(FRHITexture2D* DebugCanvasTexture)
{
	LOG_FUNC();
	check(IsInRenderingThread());
	if (!DebugCanvasFlipRHIRef.IsValid()) {
		LOGD(WVR_MultiLayer, "RenderDebugCanvasFlip_RenderThread: DebugCanvasFlipRHIRef is not Initialized");
		return;
	}

	static const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	const uint32 ViewportWidth = DebugCanvasFlipRHIRef->GetSizeX();
	const uint32 ViewportHeight = DebugCanvasFlipRHIRef->GetSizeY();
	FIntRect DstRect = FIntRect(0, 0, ViewportWidth, ViewportHeight);
	RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);

	FRHIRenderPassInfo RPInfo(DebugCanvasFlipRHIRef, ERenderTargetActions::Clear_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("FlipDebugCanvas"));
	{
		if (DebugCanvasTexture != nullptr)
		{
			auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
			TShaderMapRef<FScreenPS> PixelShader(ShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, true, false>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			//Apply DebugCanvas texture texels
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Trilinear>::GetRHI(), DebugCanvasTexture);

			RendererModule->DrawRectangle(
				RHICmdList,
				0, 0,         //top left corner of the quad
				ViewportWidth, ViewportHeight,
#if !PLATFORM_ANDROID
				0.0,  0.0,                              //U, V
				1.0,  1.0,                              //USize, VSize,
#else //Android should flip due to the UV coordinate definition.
				0.0,  1.0,
				1.0, -1.0,
#endif
				FIntPoint(ViewportWidth, ViewportHeight),
				FIntPoint(1, 1),
				VertexShader,
				EDRF_Default);
		}
	}
	RHICmdList.EndRenderPass();
}
void FDebugCanvasHandler::Show()
{
	if(WVR()->IsOverlayValid(DebugCanvasOverlayId) && !WVR()->IsOverlayShow(DebugCanvasOverlayId))
	{
		WVR()->ShowOverlay(DebugCanvasOverlayId);
		LOGD(WVR_MultiLayer, "Show DebugCanvas");
	}
}
void FDebugCanvasHandler::Hide()
{
	if(WVR()->IsOverlayValid(DebugCanvasOverlayId) && WVR()->IsOverlayShow(DebugCanvasOverlayId))
	{
		WVR()->HideOverlay(DebugCanvasOverlayId);
		LOGD(WVR_MultiLayer, "Hide DebugCanvas");
	}
}
#pragma endregion
