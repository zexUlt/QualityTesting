// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/WaveVRHandGestureComponent.h"

FStaticGestureNative UWaveVRHandGestureComponent::OnStaticGestureNative_Right;
FStaticGestureNative UWaveVRHandGestureComponent::OnStaticGestureNative_Left;

FCustomGestureNative UWaveVRHandGestureComponent::OnCustomGestureNative_Right;
FCustomGestureNative UWaveVRHandGestureComponent::OnCustomGestureNative_Left;

FDualGestureNative UWaveVRHandGestureComponent::OnCustomGestureNative_Dual;

// Sets default values for this component's properties
UWaveVRHandGestureComponent::UWaveVRHandGestureComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UWaveVRHandGestureComponent::BeginPlay()
{
	Super::BeginPlay();

	UWaveVRHandGestureComponent::OnStaticGestureNative_Right.AddDynamic(this, &UWaveVRHandGestureComponent::OnStaticGestureHandling_Right);
	UWaveVRHandGestureComponent::OnStaticGestureNative_Left.AddDynamic(this, &UWaveVRHandGestureComponent::OnStaticGestureHandling_Left);

	UWaveVRHandGestureComponent::OnCustomGestureNative_Right.AddDynamic(this, &UWaveVRHandGestureComponent::OnCustomGestureHandling_Right);
	UWaveVRHandGestureComponent::OnCustomGestureNative_Left.AddDynamic(this, &UWaveVRHandGestureComponent::OnCustomGestureHandling_Left);

	UWaveVRHandGestureComponent::OnCustomGestureNative_Dual.AddDynamic(this, &UWaveVRHandGestureComponent::OnDualGestureHandling);
}

void UWaveVRHandGestureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

// Called every frame
void UWaveVRHandGestureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#pragma region
void UWaveVRHandGestureComponent::OnStaticGestureHandling_Right(EWaveVRGestureType type)
{
	UE_LOG(LogWaveVRHandGestureComponent, Log, TEXT("OnStaticGestureHandling_Right() type: %d"), (uint8)type);
	OnStaticGestureBp_Right.Broadcast(type);
}

void UWaveVRHandGestureComponent::OnStaticGestureHandling_Left(EWaveVRGestureType type)
{
	UE_LOG(LogWaveVRHandGestureComponent, Log, TEXT("OnStaticGestureHandling_Left() type: %d"), (uint8)type);
	OnStaticGestureBp_Left.Broadcast(type);
}
#pragma endregion Default Gesture

#pragma region
void UWaveVRHandGestureComponent::OnCustomGestureHandling_Right(FString gesture)
{
	UE_LOG(LogWaveVRHandGestureComponent, Log, TEXT("OnCustomGestureHandling_Right() gesture: %s"), PLATFORM_CHAR((*gesture)));
	OnCustomGestureBp_Right.Broadcast(gesture);
}

void UWaveVRHandGestureComponent::OnCustomGestureHandling_Left(FString gesture)
{
	UE_LOG(LogWaveVRHandGestureComponent, Log, TEXT("OnCustomGestureHandling_Left() gesture: %s"), PLATFORM_CHAR((*gesture)));
	OnCustomGestureBp_Left.Broadcast(gesture);
}

void UWaveVRHandGestureComponent::OnDualGestureHandling(FString gesture)
{
	UE_LOG(LogWaveVRHandGestureComponent, Log, TEXT("OnDualGestureHandling() gesture: %s"), PLATFORM_CHAR((*gesture)));
	OnCustomGestureBp_Dual.Broadcast(gesture);
}
#pragma endregion Custom Gesture

