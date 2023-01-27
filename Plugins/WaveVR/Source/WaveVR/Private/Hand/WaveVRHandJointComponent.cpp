// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/WaveVRHandJointComponent.h"

#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandJointComponent, Log, All);

// Sets default values for this component's properties
UWaveVRHandJointComponent::UWaveVRHandJointComponent()
	: location(FVector::ZeroVector)
	, rotation(FRotator::ZeroRotator)
	, validPose(false)
	, frameCount(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	LOGD(LogWaveVRHandJointComponent, "UWaveVRHandJointComponent() Tracker %d Hand %d Joint %d.", (uint8)Tracker, (uint8)Hand, (uint8)Joint);
}


// Called when the game starts
void UWaveVRHandJointComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWaveVRHandJointComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

uint8 boneTick = 0;
// Called every frame
void UWaveVRHandJointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UWaveVRHandBPLibrary::IsHandTrackingAvailable(Tracker))
	{
		validPose = UWaveVRHandBPLibrary::GetHandJointPose(Tracker, Hand, Joint, location, rotation);

		if (validPose)
		{
			boneTick++;
			boneTick %= 300;
			if (boneTick == 0)
			{
				LOGD(LogWaveVRHandJointComponent, "Tracker %d Hand %d Joint %d position (%f, %f, %f).",
					(uint8)Tracker,
					(uint8)Hand,
					(uint8)Joint,
					location.X, location.Y, location.Z);
			}
		}
	}
	else
	{
		validPose = false;
	}
}

FVector UWaveVRHandJointComponent::GetLocation()
{
	return location;
}

FRotator UWaveVRHandJointComponent::GetRotation()
{
	return rotation;
}

bool UWaveVRHandJointComponent::IsValidPose()
{
	return validPose;
}
