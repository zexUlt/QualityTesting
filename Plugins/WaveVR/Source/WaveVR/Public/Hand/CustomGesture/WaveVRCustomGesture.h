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
#include "Math/Vector.h"

#include "../WaveVRHandBPLibrary.h"
#include "../WaveVRHandGestureComponent.h"
#include "WaveVRCustomGestureUtils.h"

#include "WaveVRCustomGesture.generated.h"

using namespace wvr::hand;

UCLASS()
class WAVEVR_API AWaveVRCustomGesture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaveVRCustomGesture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	const FString kUnknownGesture = TEXT("Unknown");

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	TArray< FSingleHandGesture > LeftGestures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	TArray< FSingleHandGesture > RightGestures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|Hand|CustomGesture")
	TArray< FDualHandGesture > DualHandGestures;

private:
	bool validPoseLeft = false, validPoseRight = false;
	TArray< FVector > s_JointPositionsLeft, s_JointPositionsRight;
	TArray< FQuat > s_JointRotationsLeft, s_JointRotationsRight;

	EWaveVRThumbState m_ThumbStateLeft = EWaveVRThumbState::None, m_ThumbStateRight = EWaveVRThumbState::None;
	EWaveVRFingerState m_IndexStateLeft = EWaveVRFingerState::None, m_IndexStateRight = EWaveVRFingerState::None;
	EWaveVRFingerState m_MiddleStateLeft = EWaveVRFingerState::None, m_MiddleStateRight = EWaveVRFingerState::None;
	EWaveVRFingerState m_RingStateLeft = EWaveVRFingerState::None, m_RingStateRight = EWaveVRFingerState::None;
	EWaveVRFingerState m_PinkyStateLeft = EWaveVRFingerState::None, m_PinkyStateRight = EWaveVRFingerState::None;

	void UpdateFingerState();
	EWaveVRThumbState GetThumbState(FVector root, FVector node1, FVector top, bool isLeft);
	EWaveVRFingerState GetFingerState(FVector root, FVector node1, FVector top, bool isLeft);
	bool MatchThumbState(FThumbState state);
	bool MatchFingerStaet(FFingerState state);

	bool MatchDistanceSingle(EWaveVRHandJoint node1, EWaveVRHandJoint node2, EWaveVRJointDistance distance, bool isLeft);
	bool MatchDistanceDual(EWaveVRHandJoint leftNode, EWaveVRHandJoint rightNode, EWaveVRJointDistance distance);

	FString m_LeftHandGesture = TEXT("Default"), m_RightHandGesture = TEXT("Default");
	bool MatchGestureSingle(FSingleHandSetting gesture, bool isLeft);
	FString m_DualHandGesture = TEXT("Default");
	bool MatchGestureDual(FDualHandSetting gesture);
};
