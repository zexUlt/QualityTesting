// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "InputModule/WaveVRInputManager.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "Hand/WaveVRHandBPLibrary.h"
#include "WaveVRBlueprintFunctionLibrary.h"

#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRInputManager, Log, All);

AWaveVRInputManager::AWaveVRInputManager()
	: m_Pawn(nullptr)
	, m_Player(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	/**
	 * DefaultSceneRoot
	 *   |-- VRCameraRoot
	 *     |-- VRCamera
	 *     |-- ControllerTransformR
	 *     |-- ControllerTransformL
	 *     |-- HandTransformR
	 *     |-- HandTransformL
	 **/

	VRCameraRoot = NewObject<USceneComponent>(this, "VRCameraRoot");
	VRCameraRoot->SetupAttachment(RootComponent);

	VRCamera = NewObject<USceneComponent>(this, TEXT("VRCamera"));
	VRCamera->SetupAttachment(VRCameraRoot);

	ControllerTransformR = NewObject<USceneComponent>(this, TEXT("ControllerTransformR"));
	ControllerTransformR->SetupAttachment(VRCameraRoot);

	ControllerTransformL = NewObject<USceneComponent>(this, TEXT("ControllerTransformL"));
	ControllerTransformL->SetupAttachment(VRCameraRoot);

	HandTransformR = NewObject<USceneComponent>(this, TEXT("HandTransformR"));
	HandTransformR->SetupAttachment(VRCameraRoot);

	HandTransformL = NewObject<USceneComponent>(this, TEXT("HandTransformL"));
	HandTransformL->SetupAttachment(VRCameraRoot);


	/**
	 * DefaultSceneRoot
	 *   |-- VRCameraRoot
	 *     |-- VRCamera
	 *     |  |-- GazeInput
	 *     |-- ControllerTransformR
	 *     |  |-- ControllerInputR
     *     |  |-- ControllerModelR
	 *     |-- ControllerTransformL
	 *     |  |-- ControllerInputL
     *     |  |-- ControllerModelL
	 *     |-- HandTransformR
	 *     |  |-- HandInputR
	 *     |  |-- HandModelR
	 *     |-- HandTransformL
	 *     |  |-- HandInputL
	 *     |  |-- HandModelL
	 **/
	GazeInput = NewObject<UWaveVRGazePointer>(this, TEXT("GazeInput"));
	GazeInput->SetupAttachment(VRCamera);

	ControllerInputR = NewObject<UWaveVRControllerPointer>(this, TEXT("ControllerInputR"));
	ControllerInputR->SetupAttachment(ControllerTransformR);

	ControllerModelR = NewObject<UWaveVRControllerModel>(this, TEXT("ControllerModelR"));
	ControllerModelR->SetupAttachment(ControllerTransformR);

	ControllerInputL = NewObject<UWaveVRControllerPointer>(this, TEXT("ControllerInputL"));
	ControllerInputL->SetupAttachment(ControllerTransformL);

	ControllerModelL = NewObject<UWaveVRControllerModel>(this, TEXT("ControllerModelL"));
	ControllerModelL->SetupAttachment(ControllerTransformL);

	HandInputR = NewObject<UWaveVRHandPointer>(this, TEXT("HandInputR"));
	HandInputR->SetupAttachment(HandTransformR);

	HandModelR = NewObject<UWaveVRHandComponent>(this, TEXT("HandModelR"));
	HandModelR->SetupAttachment(HandTransformR);

	HandInputL = NewObject<UWaveVRHandPointer>(this, TEXT("HandInputL"));
	HandInputL->SetupAttachment(HandTransformL);

	HandModelL = NewObject<UWaveVRHandComponent>(this, TEXT("HandModelL"));
	HandModelL->SetupAttachment(HandTransformL);
}

void AWaveVRInputManager::BeginPlay()
{
	Super::BeginPlay();

	if (GazeInput)
	{
		GazeInput->SetWidgetPointerIndex(WidgetPointerIndexGaze);
		GazeInput->SetTimerEnable(TimerControl);
		GazeInput->SetupGazeTimer(GazeTimer);
		GazeInput->SetButtonEnable(ButtonControl);
		GazeInput->SetupGazeButton(LeftClickButtons, RightClickButtons);
	}

	if (ControllerInputL)
	{
		ControllerInputL->SetDeviceType(EWVR_DeviceType::DeviceType_Controller_Left);
		ControllerInputL->SetWidgetPointerIndex(WidgetPointerIndexControllerL);
		ControllerInputL->SetupControllerButton(LeftClickButtons, RightClickButtons);
		ControllerInputL->SetUseInputManager(true);
	}
	if (ControllerInputR)
	{
		ControllerInputR->SetDeviceType(EWVR_DeviceType::DeviceType_Controller_Right);
		ControllerInputR->SetWidgetPointerIndex(WidgetPointerIndexControllerR);
		ControllerInputR->SetupControllerButton(LeftClickButtons, RightClickButtons);
		ControllerInputR->SetUseInputManager(true);
	}

	if (ControllerModelL)
	{
		ControllerModelL->SetDeviceType(EWVR_DeviceType::DeviceType_Controller_Left);
	}

	if (ControllerModelR)
	{
		ControllerModelR->SetDeviceType(EWVR_DeviceType::DeviceType_Controller_Right);
	}

	if (HandModelL)
	{
		// For UPoseableMeshComponent
		if (leftSkeleton != nullptr)
		{
			//FString skeletal_name = leftSkeleton->GetName();
			LOGD(LogWaveVRInputManager, "BeginPlay() SetSkeletalMesh HandModelL.");

			HandModelL->SetSkeletalMesh(leftSkeleton);
		}
		if (leftSkeletonMaterial != nullptr)
		{
			FString material_name = leftSkeletonMaterial->GetName();
			LOGD(LogWaveVRInputManager, "BeginPlay() SetMaterial %s of HandModelL.", PLATFORM_CHAR((*material_name)));

			HandModelL->SetMaterial(0, leftSkeletonMaterial);
		}

		// For UWaveVRHandComponent
		HandModelL->SetPlayerIndex(PlayerIndex);
		HandModelL->SetTrackerType(TrackerType);
		HandModelL->SetIsLeft(true);
		HandModelL->SetAlwaysShow(AlwaysShowHand);
		HandModelL->UpdateBoneIndex();
	}
	if (HandModelR)
	{
		// For UPoseableMeshComponent
		if (rightSkeleton != nullptr)
		{
			//FString skeletal_name = rightSkeleton->GetName();
			LOGD(LogWaveVRInputManager, "BeginPlay() SetSkeletalMesh of HandModelR.");

			HandModelR->SetSkeletalMesh(rightSkeleton);
		}
		if (rightSkeletonMaterial != nullptr)
		{
			FString material_name = rightSkeletonMaterial->GetName();
			LOGD(LogWaveVRInputManager, "BeginPlay() SetMaterial %s of HandModelR.", PLATFORM_CHAR((*material_name)));

			HandModelR->SetMaterial(0, rightSkeletonMaterial);
		}

		// For UWaveVRHandComponent
		HandModelR->SetPlayerIndex(PlayerIndex);
		HandModelR->SetTrackerType(TrackerType);
		HandModelR->SetIsLeft(false);
		HandModelR->SetAlwaysShow(AlwaysShowHand);
		HandModelR->UpdateBoneIndex();
	}

	EWaveVRHandGestureStatus gesture_status = UWaveVRHandBPLibrary::GetHandGestureStatus();
	// Starts Hand Gesture.
	if (gesture_status == EWaveVRHandGestureStatus::NOT_START || gesture_status == EWaveVRHandGestureStatus::START_FAILURE)
	{
		UWaveVRHandBPLibrary::StartHandGesture();
		gesture_status = UWaveVRHandBPLibrary::GetHandGestureStatus();
		LOGD(LogWaveVRInputManager, "BeginPlay() Starts Hand Gesture status %d", (uint8)gesture_status);
	}
	// Starts Hand Tracking
	if (EnableNaturalHand)
	{
		EWaveVRHandTrackingStatus tracking_status = UWaveVRHandBPLibrary::GetHandTrackingStatus(EWaveVRTrackerType::Natural);
		if (tracking_status == EWaveVRHandTrackingStatus::NOT_START || tracking_status == EWaveVRHandTrackingStatus::START_FAILURE)
		{
			UWaveVRHandBPLibrary::StartHandTracking(EWaveVRTrackerType::Natural);
			tracking_status = UWaveVRHandBPLibrary::GetHandTrackingStatus(EWaveVRTrackerType::Natural);
			LOGD(LogWaveVRInputManager, "BeginPlay() Starts natural hand tracker status %d", (uint8)tracking_status);
		}
	}
	/*if (EnableElectronicHand)
	{
		EWaveVRHandTrackingStatus tracking_status = UWaveVRHandBPLibrary::GetHandTrackingStatus(EWaveVRTrackerType::Electronic);
		if (tracking_status == EWaveVRHandTrackingStatus::NOT_START || tracking_status == EWaveVRHandTrackingStatus::START_FAILURE)
		{
			UWaveVRHandBPLibrary::StartHandTracking(EWaveVRTrackerType::Electronic);
			tracking_status = UWaveVRHandBPLibrary::GetHandTrackingStatus(EWaveVRTrackerType::Electronic);
			LOGD(LogWaveVRInputManager, "BeginPlay() Starts electronic hand tracker status %d", (uint8)tracking_status);
		}
	}*/

	if (HandInputL)
	{
		HandInputL->SetTracker(TrackerType);
		HandInputL->SetHand(EWaveVRHandType::Left);
		HandInputL->SetWidgetPointerIndex(WidgetPointerIndexHandL);
		HandInputL->SetUseDefaultPinch(UseDefaultPinch);
		HandInputL->SetPinchOnThreshold(PinchOnThreshold);
	}
	if (HandInputR)
	{
		HandInputR->SetTracker(TrackerType);
		HandInputR->SetHand(EWaveVRHandType::Right);
		HandInputR->SetWidgetPointerIndex(WidgetPointerIndexHandR);
		HandInputR->SetUseDefaultPinch(UseDefaultPinch);
		HandInputR->SetPinchOnThreshold(PinchOnThreshold);
	}
}

void AWaveVRInputManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTransform();

	bool pressLeft = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_Controller_Left, EWVR_InputId::Trigger);
	bool pressRight = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_Controller_Right, EWVR_InputId::Trigger);

	bool pressDownLeft = (!triggerLeft && pressLeft);
	triggerLeft = pressLeft;

	bool pressDownRight = (!triggerRight) && pressRight;
	triggerRight = pressRight;

	EWVR_DeviceType focused_controller = UWaveVRBlueprintFunctionLibrary::GetFocusController();
	if (focused_controller == EWVR_DeviceType::DeviceType_Controller_Right)
	{
		if (pressDownLeft)
			UWaveVRBlueprintFunctionLibrary::SetFocusController(EWVR_DeviceType::DeviceType_Controller_Left);
	}
	if (focused_controller == EWVR_DeviceType::DeviceType_Controller_Left)
	{
		if (pressDownRight)
			UWaveVRBlueprintFunctionLibrary::SetFocusController(EWVR_DeviceType::DeviceType_Controller_Right);
	}
}

void AWaveVRInputManager::UpdateTransform()
{
	/**
	 * DefaultSceneRoot (world transform)
	 *   |-- VRCameraRoot (world transform)
	 *     |-- VRCamera
	 *     |-- ControllerTransformR (relative transform)
	 *     |-- ControllerTransformL (relative transform)
	 *     |-- HandTransformR
	 *     |-- HandTransformL
	 **/
	/// DefaultSceneRoot transform
	m_Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), PlayerIndex);
	if (m_Pawn)
	{
		//LOGD(LogWaveVRInputManager, "UpdateTransform() m_Pawn %s", *m_Pawn->GetFullName());
		RootComponent->SetWorldTransform(m_Pawn->GetActorTransform());
	}

	if (VRCameraRoot)
		VRCameraRoot->SetRelativeLocation(CameraHeight);

	/// VRCamera transform
	m_Player = UGameplayStatics::GetPlayerController(GetWorld(), PlayerIndex);
	if (m_Player)
	{
		//LOGD(LogWaveVRInputManager, "UpdateTransform() Player %s", *m_Player->GetFullName());
		if (VRCamera)
		{
			//LOGD(LogWaveVRInputManager, "UpdateTransform() Camera %s", *m_Player->PlayerCameraManager->GetFullName());
			VRCamera->SetWorldLocationAndRotation(
				m_Player->PlayerCameraManager->GetCameraLocation(),
				m_Player->PlayerCameraManager->GetCameraRotation().Quaternion()
			);
		}
	}

	FVector pos = FVector::ZeroVector;
	FRotator rot = FRotator::ZeroRotator;

	/// Left controller transform
	if (ControllerTransformL)
	{
		if (UWaveVRBlueprintFunctionLibrary::GetDevicePose(pos, rot, EWVR_DeviceType::DeviceType_Controller_Left))
			ControllerTransformL->SetRelativeLocationAndRotation(pos, rot.Quaternion());
	}
	/// Right controller transform
	if (ControllerTransformR)
	{
		if (UWaveVRBlueprintFunctionLibrary::GetDevicePose(pos, rot, EWVR_DeviceType::DeviceType_Controller_Right))
			ControllerTransformR->SetRelativeLocationAndRotation(pos, rot.Quaternion());
	}

	/// We don't change the transform of HandTransformR and HandTransformL.
	/// HandTransformR has the children, HandModel and HandInput.
	/// The HandInput(WaveVRHandPointer) uses GetComponentLocation() to apply the parent transform.
	/// The HandModel(WaveVRHandComponent) uses GetPlayerPawn() and CameraHeight to apply the Pawn transform.
}
