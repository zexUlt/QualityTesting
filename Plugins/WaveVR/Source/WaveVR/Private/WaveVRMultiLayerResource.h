// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WaveVRMultiLayerResource.generated.h"

/**
 * 
 */
UCLASS()
class UWaveVRMultiLayerResource : public UObject
{
	GENERATED_BODY()
public:

	UWaveVRMultiLayerResource(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	UMaterial* underlayAlphaMaterial = nullptr, * underlayColorClearMaterial = nullptr;
};
