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
#include "Components/ActorComponent.h"

#include "WaveVRHandBPLibrary.h"
#include "WaveVRHandEnums.h"
#include "WaveVRHandJointComponent.generated.h"

UCLASS( ClassGroup=(WaveVR), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UWaveVRHandJointComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWaveVRHandJointComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Set up the tracker used for hand tracking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|Hand|Tracking")
	EWaveVRTrackerType Tracker = EWaveVRTrackerType::Natural;
	/** Set up the tracking hand. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|Hand|Tracking")
	EWaveVRHandType Hand = EWaveVRHandType::Right;
	/** Set up the tracking joint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveVR|Hand|Tracking")
	EWaveVRHandJoint Joint = EWaveVRHandJoint::Wrist;

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand|Tracking",
		meta = (ToolTip = "To get the joint location."))
	FVector GetLocation();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand|Tracking",
		meta = (ToolTip = "To get the joint rotation. (only the Wrist is available)"))
	FRotator GetRotation();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Hand|Tracking",
		meta = (ToolTip = "To check if the joint pose is valid."))
	bool IsValidPose();

private:
	FVector location;
	FRotator rotation;
	bool validPose;
	int frameCount;
};
