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

#include "Platforms/WaveVRLogWrapper.h"
#include "../WaveVRHandEnums.h"
#include "WaveVRCustomGestureUtils.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandHelper, Log, All);

UENUM(BlueprintType, Category = "WaveVR|Hand|CustomGesture")
enum class EWaveVRThumbState : uint8
{
	None = 0,
	Close = 1,
	Open = 2,
};

UENUM(BlueprintType, Category = "WaveVR|Hand|CustomGesture")
enum class EWaveVRFingerState : uint8
{
	None = 0,
	Close = 1,
	Relax = 2,
	Open = 4,
};

UENUM(BlueprintType, Category = "WaveVR|Hand|CustomGesture")
enum class EWaveVRJointDistance : uint8
{
	Near = 0,
	Far,
};

USTRUCT(BlueprintType)
struct FThumbState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	bool Close = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	bool Open = false;

public:
	uint64_t State()
	{
		return (Close ? 1 << (uint8)EWaveVRThumbState::Close : 0)
			| (Open ? 1 << (uint8)EWaveVRThumbState::Open : 0);
	}
};

USTRUCT(BlueprintType)
struct FFingerState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	bool Close = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	bool Relax = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	bool Open = false;

public:
	uint64_t State()
	{
		return (Close ? 1 << (uint8)EWaveVRFingerState::Close : 0)
			| (Relax ? 1 << (uint8)EWaveVRFingerState::Relax : 0)
			| (Open ? 1 << (uint8)EWaveVRFingerState::Open : 0);
	}
};

USTRUCT(BlueprintType)
struct FSingleHandNodeDistance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	EWaveVRHandJoint Node1 = EWaveVRHandJoint::Palm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	EWaveVRHandJoint Node2 = EWaveVRHandJoint::Palm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	EWaveVRJointDistance Distance = EWaveVRJointDistance::Near;
};

USTRUCT(BlueprintType)
struct FSingleHandSetting
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FThumbState Thumb;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FFingerState Index;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FFingerState Middle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FFingerState Ring;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FFingerState Pinky;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	TArray< FSingleHandNodeDistance > SingleHandNodeDistances;
};

USTRUCT(BlueprintType)
struct FSingleHandGesture
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FString Name = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FSingleHandSetting Setting;
};

USTRUCT(BlueprintType)
struct FDualHandNodeDistance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	EWaveVRHandJoint LeftNode = EWaveVRHandJoint::Palm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	EWaveVRHandJoint RightNode = EWaveVRHandJoint::Palm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	EWaveVRJointDistance Distance = EWaveVRJointDistance::Near;
};

USTRUCT(BlueprintType)
struct FDualHandSetting
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FSingleHandSetting LeftHand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FSingleHandSetting RightHand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	TArray< FDualHandNodeDistance > DualHandNodeDistances;
};

USTRUCT(BlueprintType)
struct FDualHandGesture
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FString Name = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	FDualHandSetting Setting;
};

namespace wvr {
	namespace hand {
		class WaveVRHandHelper {
		public:
			static float VectorAngle(FVector a, FVector b)
			{
				float angleCosine = FVector::DotProduct(a, b) / (a.Size() * b.Size());
				float angleRadians = FMath::Acos(angleCosine);
				return FMath::RadiansToDegrees(angleRadians);
			}
			static EWaveVRThumbState GetThumbState(FVector root, FVector node1, FVector top)
			{
				FVector a = node1 - root, b = top - node1;
				float angle = VectorAngle(a, b);

				return angle < 15 ? EWaveVRThumbState::Open : EWaveVRThumbState::Close;
			}
			static EWaveVRFingerState GetFingerState(FVector root, FVector node1, FVector top)
			{
				FVector a = node1 - root, b = top - node1;
				float angle = VectorAngle(a, b);

				if (angle < 25) return EWaveVRFingerState::Open;
				if (angle > 75) return EWaveVRFingerState::Close;

				return EWaveVRFingerState::Relax;
			}
			static bool MatchThumbState(FThumbState state, EWaveVRThumbState thumb)
			{
				uint32_t value = 1 << (uint8)thumb;
				if ((state.State() & value) != 0) { return true; }
				return false;
			}
			static bool MatchFingerState(FFingerState state, EWaveVRFingerState finger)
			{
				uint32_t value = 1 << (uint8)finger;
				if ((state.State() & value) != 0) { return true; }
				return false;
			}
		};
	}
}
