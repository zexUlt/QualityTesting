// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/WaveVRHandComponent.h"
#include "Hand/WaveVRHandBPLibrary.h"
#include "WaveVRHMD.h"

#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogWaveVRHandComp);

#if PLATFORM_ANDROID
#define PLATFORM_CHAR(str) TCHAR_TO_UTF8(str)
#else
#define PLATFORM_CHAR(str) str
#endif

UWaveVRHandComponent::UWaveVRHandComponent()
	: m_Pawn(nullptr)
{
	//CustomizedMaterial = nullptr;
}

void UWaveVRHandComponent::BeginPlay()
{
	Super::BeginPlay();

	handType = (isLeft) ? EWaveVRHandType::Left : EWaveVRHandType::Right;

	LOGD(LogWaveVRHandComp, "isLeft(%d), BeginPlay() handType: %d, TrackerType: %d", (uint8)isLeft, (uint8)handType, (int)handTrackerType);

	for (int i = 0; i < kJointCount; i++) {
		jointName[i] = SkeletalMesh_Hand_new_1105CX[i];
	}
	UpdateBoneIndex();

	SetHiddenInGame(false, true);
}

/// Must be called after the SkeletalMesh of UPoseableMeshComponent is updated.
void UWaveVRHandComponent::UpdateBoneIndex()
{
	for (int i = 0; i < kJointCount; i++) {
		LOGD(LogWaveVRHandComp, "isLeft(%d), UpdateBoneIndex() BoneName: %s, Index: %d"
			, (uint8)isLeft, PLATFORM_CHAR((*jointName[i].ToString())), GetBoneIndex(jointName[i])
		);
	}
}

void UWaveVRHandComponent::SetPlayerIndex(int32 index)
{
	LOGD(LogWaveVRHandComp, "isLeft(%d), SetPlayerIndex() %d", (uint8)isLeft, index);
	PlayerIndex = index;
}
void UWaveVRHandComponent::SetTrackerType(EWaveVRTrackerType trackerType)
{
	LOGD(LogWaveVRHandComp, "isLeft(%d), SetTrackerType() %d", (uint8)isLeft, (int)handTrackerType);
	handTrackerType = trackerType;
}
void UWaveVRHandComponent::SetIsLeft(bool left)
{
	isLeft = left;
	handType = (isLeft) ? EWaveVRHandType::Left : EWaveVRHandType::Right;

	LOGD(LogWaveVRHandComp, "isLeft(%d), SetIsLeft() %d", (uint8)isLeft, left);
}
void UWaveVRHandComponent::SetAlwaysShow(bool show)
{
	LOGD(LogWaveVRHandComp, "isLeft(%d), SetAlwaysShow() %d", (uint8)isLeft, show);
	alwaysShow = show;
}
bool UWaveVRHandComponent::IsShowHand()
{
	bool hand_mode = (UWaveVRBlueprintFunctionLibrary::GetInteractionMode() == EWVRInteractionMode::Hand);
	bool valid_pose = UWaveVRHandBPLibrary::IsHandPoseValid(handTrackerType, handType);
	bool focusCapturedBySystem = UWaveVRBlueprintFunctionLibrary::IsInputFocusCapturedBySystem();

	bool interactable = (alwaysShow || hand_mode)
		&& valid_pose
		&& (!focusCapturedBySystem);

	return interactable;
}

void UWaveVRHandComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	logCount++;
	logCount %= 300;
	printable = (logCount == 0);

	FVector scale = UWaveVRHandBPLibrary::GetHandScale(handTrackerType, (isLeft ? EWaveVRHandType::Left : EWaveVRHandType::Right));
	if (printable)
		LOGD(LogWaveVRHandComp, "isLeft(%d), TickComponent() scale (%f, %f, %f)", (uint8)isLeft, scale.X, scale.Y, scale.Z);
	SetRelativeScale3D(scale);

	bool isShowHand = IsShowHand();

	FTransform TrackingToWorld = GEngine->XRSystem->GetTrackingToWorldTransform();
	SetWorldRotation(TrackingToWorld.Rotator());

	if (isShowHand) {
		SetHiddenInGame(false);
		FVector wristPos = FVector::ZeroVector;
		FRotator wristRot = FRotator::ZeroRotator;
		bool ret;
		ret = UWaveVRHandBPLibrary::GetHandJointPose(handTrackerType, handType, (EWaveVRHandJoint)1, wristPos, wristRot);
		if (ret) {
			FVector VRCameraRootLocation = TrackingToWorld.GetLocation();
			SetWorldLocation(VRCameraRootLocation + TrackingToWorld.GetRotation() * wristPos);
			if (printable) {
				LOGD(LogWaveVRHandComp, "isLeft(%d), VRCameraRootLocation(%f, %f, %f)",
					(uint8)isLeft,
					VRCameraRootLocation.X, VRCameraRootLocation.Y, VRCameraRootLocation.Z);

				LOGD(LogWaveVRHandComp, "isLeft(%d), TickComponent() wrist Location %f, %f, %f", (uint8)isLeft, wristPos.X, wristPos.Y, wristPos.Z);
				LOGD(LogWaveVRHandComp, "isLeft(%d), TickComponent()       Rotation %f, %f, %f", (uint8)isLeft, wristRot.Roll, wristRot.Pitch, wristRot.Yaw);
			}
		}

		wristPos.X /= scale.X;
		wristPos.Y /= scale.Y;
		wristPos.Z /= scale.Z;
		for (int i = 0; i < kJointCount; i++) {
			FVector jointPos = FVector(0, 0, 0);
			FRotator jointRot = FRotator(0, 0, 0);
			if (i == 1) {
				jointPos = wristPos;
				jointRot = wristRot;
				ret = true;
			} else { 
				ret = UWaveVRHandBPLibrary::GetHandJointPose(handTrackerType, handType, (EWaveVRHandJoint)i, jointPos, jointRot);
				jointPos.X /= scale.X;
				jointPos.Y /= scale.Y;
				jointPos.Z /= scale.Z;
			}

			if (ret) {
				SetBoneLocationByName(jointName[i], jointPos - wristPos, EBoneSpaces::ComponentSpace);
				SetBoneRotationByName(jointName[i], jointRot, EBoneSpaces::ComponentSpace);
			}
			else {
				if (i == 1) {
					if (printable) {
						LOGD(LogWaveVRHandComp, "isLeft(%d), TickComponent() GetHandJointPose ret = false", (uint8)isLeft);
					}
				}
			}
		}
	}
	else {
		//SetVisibility(false);
		SetHiddenInGame(true);
	}

	if (printable) {
		LOGD(LogWaveVRHandComp, "isLeft(%d), TickComponent() alwaysshow(%d) isShowHand(%d)", (uint8)isLeft, (uint8)alwaysShow, (uint8)isShowHand);
	}
}
