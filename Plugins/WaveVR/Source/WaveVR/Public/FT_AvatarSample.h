// Copyright (c) 2022 HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "EyeExpression/WaveVREyeExpBPLibrary.h"
#include "LipExpression/WaveVRLipExpBPLibrary.h"
#include "FT_AvatarSample.generated.h"

/** The avatar's eye relative blend shape.*/
enum class EEyeShape :uint8 {
	Eye_Left_Blink = 0,
	Eye_Left_Wide = 1,
	Eye_Left_Right = 2,
	Eye_Left_Left = 3,
	Eye_Left_Up = 4,
	Eye_Left_Down = 5,
	Eye_Right_Blink = 6,
	Eye_Right_Wide = 7,
	Eye_Right_Right = 8,
	Eye_Right_Left = 9,
	Eye_Right_Up = 10,
	Eye_Right_Down = 11,
	Eye_Frown = 12,
	Eye_Left_Squeeze = 13,
	Eye_Right_Squeeze = 14,
	Max = 15,
	None = 63
};

/** The prediction result relative blend shape.*/
enum class ELipShape :uint8 {
	Jaw_Right = 0,
	Jaw_Left = 1,
	Jaw_Forward = 2,
	Jaw_Open = 3,
	Mouth_Ape_Shape = 4,
	Mouth_Upper_Right = 5,
	Mouth_Upper_Left = 6,
	Mouth_Lower_Right = 7,
	Mouth_Lower_Left = 8,
	Mouth_Upper_Overturn = 9,
	Mouth_Lower_Overturn = 10,
	Mouth_Pout = 11,
	Mouth_Smile_Right = 12,
	Mouth_Smile_Left = 13,
	Mouth_Sad_Right = 14,
	Mouth_Sad_Left = 15,
	Cheek_Puff_Right = 16,
	Cheek_Puff_Left = 17,
	Cheek_Suck = 18,
	Mouth_Upper_UpRight = 19,
	Mouth_Upper_UpLeft = 20,
	Mouth_Lower_DownRight = 21,
	Mouth_Lower_DownLeft = 22,
	Mouth_Upper_Inside = 23,
	Mouth_Lower_Inside = 24,
	Mouth_Lower_Overlay = 25,
	Tongue_LongStep1 = 26,
	Tongue_Left = 27,
	Tongue_Right = 28,
	Tongue_Up = 29,
	Tongue_Down = 30,
	Tongue_Roll = 31,
	Tongue_LongStep2 = 32,
	Tongue_UpRight_Morph = 33,
	Tongue_UpLeft_Morph = 34,
	Tongue_DownRight_Morph = 35,
	Tongue_DownLeft_Morph = 36,
	Max = 37,
	None = 63
};

UCLASS()
class AFT_AvatarSample : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFT_AvatarSample();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Avatar head mesh. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* HeadModel;
	/** Avatar left eye mesh. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* EyeModel_L;
	/** Avatar right eye mesh. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* EyeModel_R;
	/** Left eye's anchor. */
	UPROPERTY(VisibleDefaultsOnly, Category = Anchor)
	USceneComponent* EyeAnchor_L;
	/** Right eye's anchor. */
	UPROPERTY(VisibleDefaultsOnly, Category = Anchor)
	USceneComponent* EyeAnchor_R;

	/** If true, this Actor will update Eye expressions every frame. */
	UPROPERTY(EditAnywhere, Category = FacialTrackingSettings)
	bool EnableEye = true;
	/** If true, this Actor will update Lip expressions every frame. */
	UPROPERTY(EditAnywhere, Category = FacialTrackingSettings)
	bool EnableLip = true;

	bool isEyeActive = false;
	bool isLipActive = false;
private:
	/** ­We will use this variable to set both eyes anchor. */
	TArray<USceneComponent*> EyeAnchors;
	/** ­We will use this variable to set eye expressions weighting. */
	TMap<EEyeShape, float> EyeWeighting;
	/** ­We will use this variable to set lip expressions weighting. */
	TMap<ELipShape, float> LipWeighting;
	
	/** ­This TMap variable is used to store the corresponding result of Avatar's eye blend shapes(key)
			and OpenXRFacialTracking eye expressions(value). */
	TMap<FName, EEyeShape> EyeShapeTable;
	/** ­This TMap variable is used to store the corresponding result of Avatar's lip blend shapes(key)
			and OpenXRFacialTracking lip expressions(value). */
	TMap<FName, ELipShape> LipShapeTable;

	const EEyeShape s_EyeShapes[(int32_t)EEyeShape::Max] = {
		EEyeShape::Eye_Left_Blink, // 0
		EEyeShape::Eye_Left_Wide,
		EEyeShape::Eye_Left_Right,
		EEyeShape::Eye_Left_Left,
		EEyeShape::Eye_Left_Up,
		EEyeShape::Eye_Left_Down, // 5
		EEyeShape::Eye_Right_Blink,
		EEyeShape::Eye_Right_Wide,
		EEyeShape::Eye_Right_Right,
		EEyeShape::Eye_Right_Left,
		EEyeShape::Eye_Right_Up, // 10
		EEyeShape::Eye_Right_Down,
		EEyeShape::Eye_Frown,
		EEyeShape::Eye_Left_Squeeze,
		EEyeShape::Eye_Right_Squeeze,
	};
	const ELipShape s_LipExps[(int32_t)ELipShape::Max] = {
		ELipShape::Jaw_Right,
		ELipShape::Jaw_Left,
		ELipShape::Jaw_Forward,
		ELipShape::Jaw_Open,
		ELipShape::Mouth_Ape_Shape,
		ELipShape::Mouth_Upper_Right,		// 5
		ELipShape::Mouth_Upper_Left,
		ELipShape::Mouth_Lower_Right,
		ELipShape::Mouth_Lower_Left,
		ELipShape::Mouth_Upper_Overturn,
		ELipShape::Mouth_Lower_Overturn,	// 10
		ELipShape::Mouth_Pout,
		ELipShape::Mouth_Smile_Right,
		ELipShape::Mouth_Smile_Left,
		ELipShape::Mouth_Sad_Right,
		ELipShape::Mouth_Sad_Left,			// 15
		ELipShape::Cheek_Puff_Right,
		ELipShape::Cheek_Puff_Left,
		ELipShape::Cheek_Suck,
		ELipShape::Mouth_Upper_UpRight,
		ELipShape::Mouth_Upper_UpLeft,		// 20
		ELipShape::Mouth_Lower_DownRight,
		ELipShape::Mouth_Lower_DownLeft,
		ELipShape::Mouth_Upper_Inside,
		ELipShape::Mouth_Lower_Inside,
		ELipShape::Mouth_Lower_Overlay,		// 25
		ELipShape::Tongue_LongStep1,
		ELipShape::Tongue_Left,
		ELipShape::Tongue_Right,
		ELipShape::Tongue_Up,
		ELipShape::Tongue_Down,				// 30
		ELipShape::Tongue_Roll,
		ELipShape::Tongue_LongStep2,
		ELipShape::Tongue_UpRight_Morph,
		ELipShape::Tongue_UpLeft_Morph,
		ELipShape::Tongue_DownRight_Morph,	// 35
		ELipShape::Tongue_DownLeft_Morph,
	};
	// total 14 items = EyeShapeTable.size() = EyeWeighting.size()
	TMap<EEyeShape, EWaveVREyeExp> eyeShapeMap = {
		{EEyeShape::Eye_Left_Blink, EWaveVREyeExp::LEFT_BLINK},			// 0
		{EEyeShape::Eye_Left_Wide, EWaveVREyeExp::LEFT_WIDE},
		{EEyeShape::Eye_Right_Blink, EWaveVREyeExp::RIGHT_BLINK},
		{EEyeShape::Eye_Right_Wide, EWaveVREyeExp::RIGHT_WIDE},
		{EEyeShape::Eye_Left_Squeeze, EWaveVREyeExp::LEFT_SQUEEZE},
		{EEyeShape::Eye_Right_Squeeze, EWaveVREyeExp::RIGHT_SQUEEZE},	// 5
		{EEyeShape::Eye_Left_Down, EWaveVREyeExp::LEFT_DOWN},
		{EEyeShape::Eye_Right_Down, EWaveVREyeExp::RIGHT_DOWN},
		{EEyeShape::Eye_Left_Left, EWaveVREyeExp::LEFT_OUT},
		{EEyeShape::Eye_Right_Left, EWaveVREyeExp::RIGHT_IN},
		{EEyeShape::Eye_Left_Right, EWaveVREyeExp::LEFT_IN},			// 10
		{EEyeShape::Eye_Right_Right, EWaveVREyeExp::RIGHT_OUT},
		{EEyeShape::Eye_Left_Up, EWaveVREyeExp::LEFT_UP},
		{EEyeShape::Eye_Right_Up, EWaveVREyeExp::RIGHT_UP},
	};
	// total 37 items = LipShapeTable.size() = LipWeighting.size()
	TMap<ELipShape, EWaveVRLipExp> lipShapeMap = {
		{ELipShape::Jaw_Right, EWaveVRLipExp::Jaw_Right},							// 0
		{ELipShape::Jaw_Left, EWaveVRLipExp::Jaw_Left},
		{ELipShape::Jaw_Forward, EWaveVRLipExp::Jaw_Forward},
		{ELipShape::Jaw_Open, EWaveVRLipExp::Jaw_Open},
		{ELipShape::Mouth_Ape_Shape, EWaveVRLipExp::Mouth_Ape_Shape},
		{ELipShape::Mouth_Upper_Right, EWaveVRLipExp::Mouth_Upper_Right},			// 5
		{ELipShape::Mouth_Upper_Left, EWaveVRLipExp::Mouth_Upper_Left},
		{ELipShape::Mouth_Lower_Right, EWaveVRLipExp::Mouth_Lower_Right},
		{ELipShape::Mouth_Lower_Left, EWaveVRLipExp::Mouth_Lower_Left},
		{ELipShape::Mouth_Upper_Overturn, EWaveVRLipExp::Mouth_Upper_Overturn},
		{ELipShape::Mouth_Lower_Overturn, EWaveVRLipExp::Mouth_Lower_Overturn},		// 10
		{ELipShape::Mouth_Pout, EWaveVRLipExp::Mouth_Pout},
		{ELipShape::Mouth_Smile_Right, EWaveVRLipExp::Mouth_Smile_Right},
		{ELipShape::Mouth_Smile_Left, EWaveVRLipExp::Mouth_Smile_Left},
		{ELipShape::Mouth_Sad_Right, EWaveVRLipExp::Mouth_Sad_Right},
		{ELipShape::Mouth_Sad_Left, EWaveVRLipExp::Mouth_Sad_Left},					// 15
		{ELipShape::Cheek_Puff_Right, EWaveVRLipExp::Cheek_Puff_Right},
		{ELipShape::Cheek_Puff_Left, EWaveVRLipExp::Cheek_Puff_Left},
		{ELipShape::Cheek_Suck, EWaveVRLipExp::Cheek_Suck},
		{ELipShape::Mouth_Upper_UpRight, EWaveVRLipExp::Mouth_Upper_UpRight},
		{ELipShape::Mouth_Upper_UpLeft, EWaveVRLipExp::Mouth_Upper_UpLeft},			// 20
		{ELipShape::Mouth_Lower_DownRight, EWaveVRLipExp::Mouth_Lower_DownRight},
		{ELipShape::Mouth_Lower_DownLeft, EWaveVRLipExp::Mouth_Lower_DownLeft},
		{ELipShape::Mouth_Upper_Inside, EWaveVRLipExp::Mouth_Upper_Inside},
		{ELipShape::Mouth_Lower_Inside, EWaveVRLipExp::Mouth_Lower_Inside},
		{ELipShape::Mouth_Lower_Overlay, EWaveVRLipExp::Mouth_Lower_Overlay},		// 25
		{ELipShape::Tongue_LongStep1, EWaveVRLipExp::Tongue_Longstep1},
		{ELipShape::Tongue_Left, EWaveVRLipExp::Tongue_Left},
		{ELipShape::Tongue_Right, EWaveVRLipExp::Tongue_Right},
		{ELipShape::Tongue_Up, EWaveVRLipExp::Tongue_Up},
		{ELipShape::Tongue_Down, EWaveVRLipExp::Tongue_Down},						// 30
		{ELipShape::Tongue_Roll, EWaveVRLipExp::Tongue_Roll},
		{ELipShape::Tongue_LongStep2, EWaveVRLipExp::Tongue_Longstep2},
		{ELipShape::Tongue_UpRight_Morph, EWaveVRLipExp::Tongue_UpRight_Morph},
		{ELipShape::Tongue_UpLeft_Morph, EWaveVRLipExp::Tongue_UpLeft_Morph},
		{ELipShape::Tongue_DownRight_Morph, EWaveVRLipExp::Tongue_DownRight_Morph},	// 35
		{ELipShape::Tongue_DownLeft_Morph, EWaveVRLipExp::Tongue_DownLeft_Morph},
	};
	void InitializeEyeLip();

	/** This function will calculate the gaze direction  to update the eye's anchors rotation
			to represent the direction of the eye's gaze. */
	void UpdateGazeRay();

	FORCEINLINE FVector ConvetToUnrealVector(FVector Vector, float Scale = 1.0f)
	{
		return FVector(Vector.Z * Scale, Vector.X * Scale, Vector.Y * Scale);
	}

	/** Render the result of face tracking to the avatar's blend shapes. */
	template<typename T>
	void RenderModelShape(USkeletalMeshComponent* model, TMap<FName, T> shapeTable, TMap<T, float> weighting);
};
