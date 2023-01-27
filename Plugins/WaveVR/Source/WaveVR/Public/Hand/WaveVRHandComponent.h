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
#include "Hand/WaveVRHandEnums.h"
#include "Components/PoseableMeshComponent.h"
#include "WaveVRHandComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWaveVRHandComp, Log, All);

const int kJointCount = 26;
const FName SkeletalMesh_Hand_new_1105CX[kJointCount] = {
	FName(TEXT("WaveBone_0")),
	FName(TEXT("WaveBone_1")),
	FName(TEXT("WaveBone_2")),
	FName(TEXT("WaveBone_3")),
	FName(TEXT("WaveBone_4")),
	FName(TEXT("WaveBone_5")),
	FName(TEXT("WaveBone_6")),
	FName(TEXT("WaveBone_7")),
	FName(TEXT("WaveBone_8")),
	FName(TEXT("WaveBone_9")),
	FName(TEXT("WaveBone_10")),
	FName(TEXT("WaveBone_11")),
	FName(TEXT("WaveBone_12")),
	FName(TEXT("WaveBone_13")),
	FName(TEXT("WaveBone_14")),
	FName(TEXT("WaveBone_15")),
	FName(TEXT("WaveBone_16")),
	FName(TEXT("WaveBone_17")),
	FName(TEXT("WaveBone_18")),
	FName(TEXT("WaveBone_19")),
	FName(TEXT("WaveBone_20")),
	FName(TEXT("WaveBone_21")),
	FName(TEXT("WaveBone_22")),
	FName(TEXT("WaveBone_23")),
	FName(TEXT("WaveBone_24")),
	FName(TEXT("WaveBone_25")),
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = WaveVR)
class WAVEVR_API UWaveVRHandComponent : public UPoseableMeshComponent
{
	GENERATED_BODY()

public:
	UWaveVRHandComponent();
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Specify the player index of your pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	int32 PlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	EWaveVRTrackerType handTrackerType = EWaveVRTrackerType::Natural;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	bool isLeft = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	bool alwaysShow = false;

	void SetPlayerIndex(int32 index);
	void SetTrackerType(EWaveVRTrackerType trackerType);
	void SetIsLeft(bool left);
	void SetAlwaysShow(bool show);
	void UpdateBoneIndex();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	bool updateWristPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	bool showConfidenceAsAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR")
	class UMaterialInterface* CustomizedMaterial;

	void SetSkeletonMesh(USkeletalMesh* mesh);
	void SetMaterial(UMaterialInstance* material);*/

private:
	class UMaterialInterface *DynamicMaterial;
	EWaveVRHandType handType = EWaveVRHandType::Right;
	FName jointName[kJointCount];
	int jointIndex[kJointCount];

	APawn* m_Pawn;
	FVector pawnPosition = FVector::ZeroVector;

	int logCount = 0;
	bool printable = false;

	bool IsShowHand();
};
