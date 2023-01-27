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
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"

#include "Hand/WaveVRHandEnums.h"
#include "InputModule/WaveVRGazePointer.h"
#include "InputModule/WaveVRControllerPointer.h"
#include "InputModule/WaveVRHandPointer.h"
#include "WaveVRControllerModel.h"
#include "Hand/WaveVRHandComponent.h"

#include "WaveVRInputManager.generated.h"

UCLASS()
class WAVEVR_API AWaveVRInputManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveVRInputManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

#pragma region
protected:
	/** Specify the player index of your pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	int32 PlayerIndex = 0;

	/** Set the camera height of your pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	FVector CameraHeight = FVector(0, 0, 180);

	const TArray<EWVR_InputId> kClickButtons = { EWVR_InputId::Touchpad, EWVR_InputId::Trigger, EWVR_InputId::Thumbstick };
	/** Set up the right controller buttons for click. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	TArray<EWVR_InputId> RightClickButtons = kClickButtons;
	/** Set up the left controller buttons for click. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule")
	TArray<EWVR_InputId> LeftClickButtons = kClickButtons;

	/** True to enable timer control for gaze. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Gaze")
	bool TimerControl = true;
	/** Set up the gaze timer count down duration in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Gaze", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float GazeTimer = 2.f;
	/** True to enable button control for gaze. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Gaze")
	bool ButtonControl = true;

	/** Enable the natural hand tracker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	bool EnableNaturalHand = false;
	/** Enable the electronic hand tracker. */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	bool EnableElectronicHand = false;*/

	/** Select the hand tracker to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	EWaveVRTrackerType TrackerType = EWaveVRTrackerType::Natural;

	/** "Use default pinch threshold." */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	bool UseDefaultPinch = false;
	/** When the Pinch strength value is bigger than the Pinch On Threshold, the WaveVRHandPointer will start sending events. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand", meta = (UIMin = "0.0", UIMax = "1.0"))
	float PinchOnThreshold = 0.7f;

	/** Set up the PointerIndex of the gaze widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Gaze")
	int32 WidgetPointerIndexGaze = 0;
	/** Set up the PointerIndex of the left controller widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Controller")
	int32 WidgetPointerIndexControllerL = 1;
	/** Set up the PointerIndex of the right controller widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Controller")
	int32 WidgetPointerIndexControllerR = 2;
	/** Set up the PointerIndex of the left hand widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	int32 WidgetPointerIndexHandL = 3;
	/** Set up the PointerIndex of the right hand widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	int32 WidgetPointerIndexHandR = 4;

	/** Always show the hand model. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	bool AlwaysShowHand = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	USkeletalMesh *rightSkeleton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	UMaterialInterface *rightSkeletonMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	USkeletalMesh *leftSkeleton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|InputModule|Hand")
	UMaterialInterface *leftSkeletonMaterial;

#pragma endregion Customized Variables

private:
	void UpdateTransform();

private:
	APawn* m_Pawn;
	APlayerController* m_Player;

	USceneComponent* DefaultSceneRoot;
	USceneComponent* VRCameraRoot;

	// GazeInput is binding with VRCamera.
	USceneComponent* VRCamera;
	UWaveVRGazePointer* GazeInput;

	// ControllerInput is binding with ControllerTransform.
	USceneComponent *ControllerTransformL, *ControllerTransformR;
	UWaveVRControllerPointer *ControllerInputL, *ControllerInputR;
    UWaveVRControllerModel *ControllerModelL, *ControllerModelR;
	UWaveVRHandComponent *HandModelL, *HandModelR;

	// HandInput is binding with HandTransform
	USceneComponent *HandTransformL, *HandTransformR;
	UWaveVRHandPointer *HandInputL, *HandInputR;

	// Buttons
private:
	bool triggerLeft = false, triggerRight = false;
};
