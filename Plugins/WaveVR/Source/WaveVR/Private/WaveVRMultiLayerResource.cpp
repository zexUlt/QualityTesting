// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveVRMultiLayerResource.h"

#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

UWaveVRMultiLayerResource::UWaveVRMultiLayerResource(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterial> underlayAlphaMaterialAsset(TEXT("Material'/WaveVR/Materials/UnderlayAlpha.UnderlayAlpha'"));
	if (underlayAlphaMaterialAsset.Object != NULL)
	{
		underlayAlphaMaterial = (UMaterial*)underlayAlphaMaterialAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> underlayColorClearMaterialAsset(TEXT("Material'/WaveVR/Materials/UnderlayColorClear.UnderlayColorClear'"));
	if (underlayColorClearMaterialAsset.Object != NULL)
	{
		underlayColorClearMaterial = (UMaterial*)underlayColorClearMaterialAsset.Object;
	}
}
