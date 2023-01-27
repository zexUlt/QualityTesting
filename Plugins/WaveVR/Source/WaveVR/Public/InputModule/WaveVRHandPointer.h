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
#include "Components/SceneComponent.h"

#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Math/Color.h"
#include "Engine/StaticMesh.h"
#include "Components/WidgetInteractionComponent.h"

#include "Hand/WaveVRHandBPLibrary.h"

#include "WaveVRHandPointer.generated.h"

/**
 * Hand input is used only for the natural hand.
 **/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UWaveVRHandPointer : public USceneComponent
{
	GENERATED_BODY()

public:	
	UWaveVRHandPointer();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	uint32_t logFrame = 0;
	const uint32_t klogFrameCount = 300;
	bool printIntervalLog = false;
	void CheckLogInterval();

#pragma region
protected:
	/** Always enable the Hand Input no matter what the interaction  mode is. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	bool AlwaysEnable = false;

	EWaveVRTrackerType m_TrackerType = EWaveVRTrackerType::Natural;
	/** Set up the hand tracker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	EWaveVRTrackerType TrackerType = EWaveVRTrackerType::Natural;

	EWaveVRHandType m_Hand = EWaveVRHandType::Right;
	/** Set up the tracking hand. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	EWaveVRHandType Hand = EWaveVRHandType::Right;

	/** Set up the available distance of hand LineTrace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	float TraceDistance = 10000.f;

	bool m_UseDefaultPinch = false;
	/** "Use default pinch threshold." */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	bool UseDefaultPinch = false;

	float m_PinchOnThreshold = 0.7f;
	/** When the Pinch strength value is bigger than the Pinch On Threshold, the WaveVRHandPointer will start sending events. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule", meta = (UIMin = "0.0", UIMax = "1.0"))
	float PinchOnThreshold = 0.7f;

	/** Set up the color of beam generated by Particle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FLinearColor FadingParticleColor = FLinearColor::Green;
	/** Set up the EmitterIndex of beam generated by Particle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int BeamEmitterIndex = 0;
	/** Set up the SourceIndex of beam generated by Particle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int BeamSourceIndex = 0;
	/** Set up the TargetIndex of beam generated by Particle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int BeamTargetIndex = 0;
	/** Set up the vector parameter of the Particle's size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FVector FadingParticleSize = FVector(1, 1, 1);

	/** Set up the scale of the controller spot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FVector SpotScale = FVector(0.015f, 0.015f, 0.015f);
	/** Set up the rotation offset of the controller spot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FRotator SpotRotationOffset = FRotator(0, 0, 0);

	/** Set up the VirtualUserIndex of the controller widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int32 WidgetVirtualUserIndex = 0;
	/** Set up the PointerIndex of the controller widget. Note: the Virtual User Index should be different in both hand. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int32 WidgetPointerIndex = 3;
#pragma endregion Customized Variables

public:
	void SetTracker(EWaveVRTrackerType tracker);
	void SetHand(EWaveVRHandType hand);
	void SetWidgetPointerIndex(int32 index);
	void SetUseDefaultPinch(bool use);
	void SetPinchOnThreshold(float strength);

private:
	void UpdateCustomizedVariables();

// Resources
protected:
	UParticleSystemComponent* FadingParticle;
	UStaticMeshComponent* SpotPointer;
	UStaticMeshComponent* SpotPointerBlue;
	UWidgetInteractionComponent* WidgetInteraction;

private:
	void ConfigureResources();
	void ActivateBeamPointer();
	bool IsHandInteractable();

private:
	UParticleSystem* fadingParticleSystemTemplate;
	const int fadingParticleMaterialIndex = 0;
	UMaterialInterface* fadingParticleMaterial;
	const FName k_InitialColor = FName(TEXT("ParticleInitialColor"));
	const FName k_InitialSize = FName(TEXT("ParticleInitialSize"));

	UStaticMesh* spotPointerMesh;
	UMaterialInterface* spotPointerMaterial;
	UStaticMesh* spotPointerBlueMesh;
	UMaterialInterface* spotPointerBlueMaterial;

// Trace
private:
	FVector worldLocation;
	FRotator worldRotation;

	void SetIgnoreActors(FCollisionQueryParams& collisionParams);

	FVector targetPoint;

	bool tracedPhysics;
	//FHitResult hit;
	TArray<FHitResult> outHits;
	AActor* focusActor;
	const float k_DefaultPointerDistance = 100;
	float targetDistance;

	bool tracedGraphics;
	FVector tracedWidgetPosition;

// Pinch
private:
	bool IsClicked();
	bool pressState;
};