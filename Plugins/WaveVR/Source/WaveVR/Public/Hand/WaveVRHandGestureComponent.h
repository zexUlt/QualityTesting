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
#include "WaveVRHandEnums.h"
#include "WaveVRHandGestureComponent.generated.h"


DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandGestureComponent, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStaticGestureNative, EWaveVRGestureType, type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStaticGestureBp, EWaveVRGestureType, type);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomGestureNative, FString, gesture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomGestureBp, FString, gesture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDualGestureNative, FString, gesture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDualGestureBp, FString, gesture);

UCLASS( ClassGroup=(WaveVR), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UWaveVRHandGestureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWaveVRHandGestureComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region
public:
	static FStaticGestureNative OnStaticGestureNative_Left;
	static FStaticGestureNative OnStaticGestureNative_Right;

	UFUNCTION()
	void OnStaticGestureHandling_Left(EWaveVRGestureType type);
	UFUNCTION()
	void OnStaticGestureHandling_Right(EWaveVRGestureType type);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Hand|DefaultGesture")
	FStaticGestureBp OnStaticGestureBp_Left;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Hand|DefaultGesture")
	FStaticGestureBp OnStaticGestureBp_Right;
#pragma endregion Default Gesture

#pragma region
	/// Single Hand
	static FCustomGestureNative OnCustomGestureNative_Left;
	static FCustomGestureNative OnCustomGestureNative_Right;

	UFUNCTION()
	void OnCustomGestureHandling_Left(FString gesture);
	UFUNCTION()
	void OnCustomGestureHandling_Right(FString gesture);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Hand|CustomGesture")
	FCustomGestureBp OnCustomGestureBp_Left;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Hand|CustomGesture")
	FCustomGestureBp OnCustomGestureBp_Right;

	/// Dual Hand
	static FDualGestureNative OnCustomGestureNative_Dual;

	UFUNCTION()
	void OnDualGestureHandling(FString gesture);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Hand|CustomGesture")
	FDualGestureBp OnCustomGestureBp_Dual;
#pragma endregion Custom Gesture

};
