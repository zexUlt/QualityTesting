// Copyright (c) 2022 HTC Corporation. All Rights Reserved.


#include "FT_AvatarSample.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AFT_AvatarSample::AFT_AvatarSample()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	// Head
	HeadModel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadModel"));
	HeadModel->SetupAttachment(RootComponent);
	// Eye_L
	EyeAnchor_L = CreateDefaultSubobject<USceneComponent>(TEXT("EyeAnchor_L"));
	EyeAnchor_L->SetupAttachment(RootComponent);
	EyeAnchor_L->SetRelativeLocation(FVector(3.448040f, 7.892285f, 0.824235f));
	EyeModel_L = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EyeModel_L"));
	EyeModel_L->SetupAttachment(EyeAnchor_L);
	EyeModel_L->SetRelativeLocation(FVector(-3.448040f, -7.892285f, -0.824235f));
	// Eye_R
	EyeAnchor_R = CreateDefaultSubobject<USceneComponent>(TEXT("EyeAnchor_R"));
	EyeAnchor_R->SetupAttachment(RootComponent);
	EyeAnchor_R->SetRelativeLocation(FVector(-3.448040f, 7.892285f, 0.824235f));
	EyeModel_R = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EyeModel_R"));
	EyeModel_R->SetupAttachment(EyeAnchor_R);
	EyeModel_R->SetRelativeLocation(FVector(3.448040f, -7.892285f, -0.824235f));

	EyeAnchors.AddUnique(EyeAnchor_L);
	EyeAnchors.AddUnique(EyeAnchor_R);
	
	InitializeEyeLip();
}

// Called when the game starts or when spawned
void AFT_AvatarSample::BeginPlay()
{
	Super::BeginPlay();
	this->SetOwner(GetWorld()->GetFirstPlayerController()->GetPawn());
	// CreateFacialTracker_Eye
	if (EnableEye)
	{
		UWaveVREyeExpBPLibrary::StartEyeExp();
	}

	// CreateFacialTracker_Lip
	if (EnableLip)
	{
		UWaveVRLipExpBPLibrary::StartLipExp();
	}

	EyeShapeTable = {
		{"Eye_Left_Blink", EEyeShape::Eye_Left_Blink},
		{"Eye_Left_Wide", EEyeShape::Eye_Left_Wide},
		{"Eye_Left_Right",EEyeShape::Eye_Left_Right},
		{"Eye_Left_Left",EEyeShape::Eye_Left_Left},
		{"Eye_Left_Up", EEyeShape::Eye_Left_Up},
		{"Eye_Left_Down", EEyeShape::Eye_Left_Down},
		{"Eye_Right_Blink", EEyeShape::Eye_Right_Blink},
		{"Eye_Right_Wide", EEyeShape::Eye_Right_Wide},
		{"Eye_Right_Right",EEyeShape::Eye_Right_Right},
		{"Eye_Right_Left", EEyeShape::Eye_Right_Left},
		{"Eye_Right_Up", EEyeShape::Eye_Right_Up},
		{"Eye_Right_Down", EEyeShape::Eye_Right_Down},
		//{"Eye_Frown", EEyeShape::Eye_Frown},
		{"Eye_Left_squeeze", EEyeShape::Eye_Left_Squeeze},
		{"Eye_Right_squeeze", EEyeShape::Eye_Right_Squeeze}
	};

	LipShapeTable = {
		{"Jaw_Right", ELipShape::Jaw_Right},
		{"Jaw_Left", ELipShape::Jaw_Left},
		{"Jaw_Forward", ELipShape::Jaw_Forward},
		{"Jaw_Open", ELipShape::Jaw_Open},
		{"Mouth_Ape_Shape", ELipShape::Mouth_Ape_Shape},
		{"Mouth_Upper_Right", ELipShape::Mouth_Upper_Right},
		{"Mouth_Upper_Left", ELipShape::Mouth_Upper_Left},
		{"Mouth_Lower_Right", ELipShape::Mouth_Lower_Right},
		{"Mouth_Lower_Left", ELipShape::Mouth_Lower_Left},
		{"Mouth_Upper_Overturn", ELipShape::Mouth_Upper_Overturn},
		{"Mouth_Lower_Overturn", ELipShape::Mouth_Lower_Overturn},
		{"Mouth_Pout", ELipShape::Mouth_Pout},
		{"Mouth_Smile_Right", ELipShape::Mouth_Smile_Right},
		{"Mouth_Smile_Left", ELipShape::Mouth_Smile_Left},
		{"Mouth_Sad_Right", ELipShape::Mouth_Sad_Right},
		{"Mouth_Sad_Left", ELipShape::Mouth_Sad_Left},
		{"Cheek_Puff_Right", ELipShape::Cheek_Puff_Right},
		{"Cheek_Puff_Left", ELipShape::Cheek_Puff_Left},
		{"Cheek_Suck", ELipShape::Cheek_Suck},
		{"Mouth_Upper_UpRight", ELipShape::Mouth_Upper_UpRight},
		{"Mouth_Upper_UpLeft", ELipShape::Mouth_Upper_UpLeft},
		{"Mouth_Lower_DownRight", ELipShape::Mouth_Lower_DownRight},
		{"Mouth_Lower_DownLeft", ELipShape::Mouth_Lower_DownLeft},
		{"Mouth_Upper_Inside", ELipShape::Mouth_Upper_Inside},
		{"Mouth_Lower_Inside", ELipShape::Mouth_Lower_Inside},
		{"Mouth_Lower_Overlay", ELipShape::Mouth_Lower_Overlay},
		{"Tongue_LongStep1", ELipShape::Tongue_LongStep1},
		{"Tongue_Left", ELipShape::Tongue_Left},
		{"Tongue_Right", ELipShape::Tongue_Right},
		{"Tongue_Up", ELipShape::Tongue_Up},
		{"Tongue_Down", ELipShape::Tongue_Down},
		{"Tongue_Roll", ELipShape::Tongue_Roll},
		{"Tongue_LongStep2", ELipShape::Tongue_LongStep2},
		{"Tongue_UpRight_Morph", ELipShape::Tongue_UpRight_Morph},
		{"Tongue_UpLeft_Morph", ELipShape::Tongue_UpLeft_Morph},
		{"Tongue_DownRight_Morph", ELipShape::Tongue_DownRight_Morph},
		{"Tongue_DownLeft_Morph", ELipShape::Tongue_DownLeft_Morph}
	};
}

void AFT_AvatarSample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// DestroyFacialTracker_Eye
	UWaveVREyeExpBPLibrary::StopEyeExp();
	UWaveVRLipExpBPLibrary::StopLipExp();

	EyeShapeTable.Empty();
	LipShapeTable.Empty();

}

// Called every frame
void AFT_AvatarSample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Update Eye Shapes
	if (EnableEye)
	{
		for (int32_t i = 0; i < (int32_t)EEyeShape::Max; i++)
		{
			if (eyeShapeMap.Contains(s_EyeShapes[i]))
			{
				EyeWeighting[s_EyeShapes[i]] = UWaveVREyeExpBPLibrary::GetEyeExpValue(eyeShapeMap[s_EyeShapes[i]]);
			}
		}
		RenderModelShape(HeadModel, EyeShapeTable, EyeWeighting);
		UpdateGazeRay();
	}
	
	//Update Lip Shapes
	if (EnableLip)
	{
		for (int32_t i = 0; i < (int32_t)ELipShape::Max; i++)
		{
			if (lipShapeMap.Contains(s_LipExps[i]))
			{
				LipWeighting[s_LipExps[i]] = UWaveVRLipExpBPLibrary::GetLipExpValue(lipShapeMap[s_LipExps[i]]);
			}
		}
		RenderModelShape(HeadModel, LipShapeTable, LipWeighting);
	}
	
}

template<typename T>
void AFT_AvatarSample::RenderModelShape(USkeletalMeshComponent* model, TMap<FName, T> shapeTable, TMap<T, float> weighting)
{
	if (shapeTable.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RenderModelShape] shapeTable.Num <= 0."))
		return;
	}
	if (weighting.Num() <= 0) return;
	for (auto &table : shapeTable)
	{
		if ((int)table.Value != (int)EEyeShape::None || (int)table.Value != (int)ELipShape::None)
		{
			model->SetMorphTarget(table.Key, weighting[table.Value]);
		}
	}
}

void AFT_AvatarSample::UpdateGazeRay()
{
	FVector gazeDirectionCombinedLocal_L;
	FVector modelGazeOrigin_L, modelGazeTarget_L;
	FRotator lookAtRotation_L, eyeRotator_L;
	if (EyeWeighting.Num() <= 0) return;
	if (EyeWeighting[EEyeShape::Eye_Left_Right] > EyeWeighting[EEyeShape::Eye_Left_Left])
	{
		gazeDirectionCombinedLocal_L.X = EyeWeighting[EEyeShape::Eye_Left_Right];
	}
	else
	{
		gazeDirectionCombinedLocal_L.X = -EyeWeighting[EEyeShape::Eye_Left_Left];
	}
	if (EyeWeighting[EEyeShape::Eye_Left_Up] > EyeWeighting[EEyeShape::Eye_Left_Down])
	{
		gazeDirectionCombinedLocal_L.Y = EyeWeighting[EEyeShape::Eye_Left_Up];
	}
	else
	{
		gazeDirectionCombinedLocal_L.Y = -EyeWeighting[EEyeShape::Eye_Left_Down];
	}
	gazeDirectionCombinedLocal_L.Z = 1.0f;
	modelGazeOrigin_L = EyeAnchors[0]->GetRelativeLocation();
	modelGazeTarget_L = EyeAnchors[0]->GetRelativeLocation() + gazeDirectionCombinedLocal_L;
	lookAtRotation_L = UKismetMathLibrary::FindLookAtRotation(ConvetToUnrealVector(modelGazeOrigin_L), ConvetToUnrealVector(modelGazeTarget_L));
	eyeRotator_L = FRotator(lookAtRotation_L.Roll, lookAtRotation_L.Yaw, -lookAtRotation_L.Pitch);
	EyeAnchors[0]->SetRelativeRotation(eyeRotator_L);

	FVector gazeDirectionCombinedLocal_R;
	FVector modelGazeOrigin_R, modelGazeTarget_R;
	FRotator lookAtRotation_R, eyeRotator_R;
	if (EyeWeighting[EEyeShape::Eye_Right_Left] > EyeWeighting[EEyeShape::Eye_Right_Right])
	{
		gazeDirectionCombinedLocal_R.X = -EyeWeighting[EEyeShape::Eye_Right_Left];
	}
	else
	{
		gazeDirectionCombinedLocal_R.X = EyeWeighting[EEyeShape::Eye_Right_Right];
	}
	if (EyeWeighting[EEyeShape::Eye_Right_Up] > EyeWeighting[EEyeShape::Eye_Right_Down])
	{
		gazeDirectionCombinedLocal_R.Y = EyeWeighting[EEyeShape::Eye_Right_Up];
	}
	else
	{
		gazeDirectionCombinedLocal_R.Y = -EyeWeighting[EEyeShape::Eye_Right_Down];
	}
	gazeDirectionCombinedLocal_R.Z = 1.0f;
	modelGazeOrigin_R = EyeAnchors[0]->GetRelativeLocation();
	modelGazeTarget_R = EyeAnchors[0]->GetRelativeLocation() + gazeDirectionCombinedLocal_R;

	lookAtRotation_R = UKismetMathLibrary::FindLookAtRotation(ConvetToUnrealVector(modelGazeOrigin_R), ConvetToUnrealVector(modelGazeTarget_R));
	eyeRotator_R = FRotator(lookAtRotation_R.Roll, lookAtRotation_R.Yaw, -lookAtRotation_R.Pitch);
	EyeAnchors[1]->SetRelativeRotation(eyeRotator_R);
}

void AFT_AvatarSample::InitializeEyeLip()
{
	/** ­We will use this variable to set eye expressions weighting. */
	for (int32_t i = 0; i < (int32_t)EEyeShape::Max; i++)
	{
		if (eyeShapeMap.Contains(s_EyeShapes[i]))
			EyeWeighting.Add(s_EyeShapes[i], 0);
	}
	/** ­We will use this variable to set lip expressions weighting. */
	for (int32_t i = 0; i < (int32_t)ELipShape::Max; i++)
	{
		if (lipShapeMap.Contains(s_LipExps[i]))
			LipWeighting.Add(s_LipExps[i], 0);
	}
}
