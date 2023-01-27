// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "ProceduralMeshComponent.h"

// Wave
#include "WaveVRBlueprintFunctionLibrary.h"
#include "WaveVRNativeModel.h"

// Always last
#include "WaveVRControllerModel.generated.h"

UCLASS( ClassGroup=(WaveVR), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UWaveVRControllerModel : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWaveVRControllerModel();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|ControllerModel")
	EWVR_DeviceType deviceType = EWVR_DeviceType::DeviceType_Controller_Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|ControllerModel")
	bool alwaysShow = false;

	/** Hide the ray when the controller is idle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|ControllerModel")
	bool HideWhenIdle = true;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetDeviceType(EWVR_DeviceType devType);

private:
	void SpawnControllerModel();
	void DestroyController();
	void checkIfLeftHand();

	bool preConnection;
	bool currConnection;

	AWaveVRNativeModel *SpawnController;

	EWVR_DeviceType realDeviceType;
	bool isLeftHand;
	bool deviceTypeChanged;
	int ddelay;
};
