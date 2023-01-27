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

// Line trace and GUI
#include "GameFramework/Actor.h"
#include "Components/WidgetInteractionComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "EyeTrackerTypes.h"

// Blueprint interface
#include "WaveVRInteractInterface.h"
#include "WaveVRBlueprintFunctionLibrary.h"

#include "WaveVRGazePointer.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UWaveVRGazePointer : public USceneComponent
{
	GENERATED_BODY()

public:	
	UWaveVRGazePointer();

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
	/** Always enable the Gaze Input no matter what the interaction mode is. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	bool AlwaysEnable = false;

	/** True to use eye tracking for gaze. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	bool EyeTracking = false;

	/** Specify the player index of your pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int32 PlayerIndex = 0;

	bool m_TimerControl = true;
	/** True to enable timer control for gaze. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	bool TimerControl = true;
	const float kGazeTimerMin = 1.f, kGazeTimerMax = 5.f;
	float m_GazeTimer = 1.5f;
	/** Set up the timer count down duration in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float GazeTimer = 1.5f;

	bool m_ButtonControl = true;
	/** True to enable button control for gaze. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	bool ButtonControl = true;
	const TArray<EWVR_InputId> kClickButtons = { EWVR_InputId::Touchpad, EWVR_InputId::Trigger, EWVR_InputId::Thumbstick };
	TArray<EWVR_InputId> m_RightClickButtons = kClickButtons;
	/** Set up the right controller buttons for click. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	TArray<EWVR_InputId> RightClickButtons = kClickButtons;

	TArray<EWVR_InputId> m_LeftClickButtons = kClickButtons;
	/** Set up the left controller buttons for click. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	TArray<EWVR_InputId> LeftClickButtons = kClickButtons;

	/** Set up the available distance of gaze LineTrace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WaveVR|InputModule")
	float TraceDistance = 10000.f;

	/** Set up the scale of the gaze spot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FVector SpotScale = FVector(1.5f, 1.5f, 1.5f);
	/** Set up the rotation offset of the gaze spot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FRotator SpotRotationOffset = FRotator(0, 0, 0);
	/** Set up the initial distance of the gaze spot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	float SpotDistance = 100.f;

	/** Set up the scale of the gaze ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FVector RingScale = FVector(3, 3, 3);
	/** Set up the rotation offset of the gaze ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FRotator RingRotationOffset = FRotator(90, 0, 0);
	/** Set up the initial distance of the gaze ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	float RingDistance = 100.f;

	/** Set up the VirtualUserIndex of the gaze widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int32 WidgetVirtualUserIndex = 0;
	/** Set up the PointerIndex of the gaze widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int32 WidgetPointerIndex = 0;
#pragma endregion Customized Variables

public:
	void SetWidgetPointerIndex(int32 pointerIndex);
	void SetTimerEnable(bool enable);
	void SetupGazeTimer(float duration);
	void SetButtonEnable(bool enable);
	void SetupGazeButton(const TArray<EWVR_InputId> leftButtons, const TArray<EWVR_InputId> rightButtons);

private:
	void UpdateCustomizedVariables();
	EWVRGazeTriggerType m_GazeTypeEx = EWVRGazeTriggerType::TimeoutButton;
	EWVRGazeTriggerType m_GazeType = EWVRGazeTriggerType::TimeoutButton;

// Resources
protected:
	UStaticMeshComponent* SpotPointer;
	UStaticMeshComponent* RingPointer;
	UStaticMeshComponent* RollingRingPointer;
	UWidgetInteractionComponent* WidgetInteraction;

private:
	void ConfigureResources();
	void ActivatePointer();
	bool IsGazeInteractable();

private:
	UStaticMesh* spotPointerMesh;
	UMaterialInterface* spotPointerMaterial;
	UStaticMesh* ringPointerMesh;
	UMaterialInterface* ringPointerMaterial;
	UStaticMesh* rollingRingPointerMesh;
	UMaterialInterface* rollingRingPointerMaterial;

// Trace
private:
	FVector worldLocation;
	FRotator worldRotation;

	void SetIgnoreActors(FCollisionQueryParams& collisionParams);

	bool tracedPhysics;
	FHitResult hit;
	AActor* focusActor;

	bool tracedWidget;
	FVector tracedWidgetPosition;

	FEyeTrackerGazeData m_GazeData;
	FVector m_EyeVector = FVector(1, 0, 0);
	FVector CameraPosition = FVector::ZeroVector;
	FQuat CameraRotation = FQuat::Identity;
	FVector GetEyeVector();
	FRotator GetEyeRotation();

// Trigger type
private:
	void ForceUpdateGazeType();
	void UpdateGazeType();

	bool IsTimeout();
	float gazeOnTime;

	bool IsClicked();
	bool pressState;
};
