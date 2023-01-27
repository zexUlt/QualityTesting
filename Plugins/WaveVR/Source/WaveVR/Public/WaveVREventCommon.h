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
#include "WaveVRBlueprintFunctionLibrary.h"
#include "WaveVREventCommon.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREventCommon, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSettingsControllerRoleChangeNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSettingsControllerRoleChangeBp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputRequestChangeNative, uint8, device);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputRequestChangeBp, uint8, device);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResumeNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResumeBp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTrackingModeChangeNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTrackingModeChangeBp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FControllerPoseModeChangedNative, uint8, Device, uint8, Mode, FTransform, Transform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FControllerPoseModeChangedBp, uint8, Device, uint8, Mode, FTransform, Transform);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOEMConfigChangeNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOEMConfigChangeBp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAllCommonEventNative, int32, event_num);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAllCommonEventBp, int32, event_num);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonEventBp, EWVR_EventType, eventId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAllButtonPressNative, uint8, button, bool, down);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAllButtonPressBp, uint8, button, bool, down);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAllButtonTouchNative, uint8, button, bool, down);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAllButtonTouchBp, uint8, button, bool, down);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHandChanging, bool, changing);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FConnectionChangeNative, uint8, device, bool, connection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FConnectionChangeBp, uint8, device, bool, connection);


static const int EventCommonButtonCount = 32;

UCLASS( ClassGroup=(WaveVR), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UWaveVREventCommon : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWaveVREventCommon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	static bool bInitialized;

#pragma region
/// Call flow:
/// 1. Register handling function(s) to OnControllerRoleChangeNative
///		- function 1: OnControllerRoleChangeHandling
/// 2. WaveVRHMD calls OnControllerRoleChangeNative
/// 3. OnControllerRoleChangeNative calls handling function(s)
///		- function 1: OnControllerRoleChangeHandling
/// 4. OnControllerRoleChangeHandling is called, then call to OnControllerRoleChangeBp
public:
	static FSettingsControllerRoleChangeNative OnControllerRoleChangeNative;

	// OnControllerRoleChangeNative -> OnControllerRoleChangeHandling
	UFUNCTION()
	void OnControllerRoleChangeHandling();

	// OnControllerRoleChangeHandling -> OnControllerRoleChangeBp
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Event")
	FSettingsControllerRoleChangeBp OnControllerRoleChangeBp;
#pragma endregion Role Change

#pragma region
public:
	static FOnResumeNative OnResumeNative;

	UFUNCTION()
	void OnResumeHandling();

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Application")
	FOnResumeBp OnResumeBp;
#pragma endregion On Application Resume

#pragma region
public:
	static FTrackingModeChangeNative OnTrackingModeChangeNative;

	UFUNCTION()
	void OnTrackingModeChangeHandling();

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Event")
	FTrackingModeChangeBp OnTrackingModeChangeBp;

	static FControllerPoseModeChangedNative OnControllerPoseModeChangedNative;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Event")
	FControllerPoseModeChangedBp OnControllerPoseModeChangedBp;
	UFUNCTION()
	void OnControllerPoseModeChangedHandling(uint8 Device, uint8 Mode, FTransform Transform);

	static FOEMConfigChangeNative OnOEMConfigChangeNative;

	UFUNCTION()
	void OnOEMConfigChangeHandling();

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Event")
	FOEMConfigChangeBp OnOEMConfigChangeBp;

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Event",
		meta = (ToolTip = "To notify the engine to check the Degree-Of-Freedom on resume."))
	static void ShouldCheckDoFOnResume(bool check);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Event",
		meta = (ToolTip = "To get the Degree-Of-Freedom."))
	static EWVR_DOF GetDegreeOfFreedom();

private:
	static bool bCheckTrackingMode;
	static EWVR_DOF currentDoF;
#pragma endregion Tracking Mode Change

#pragma region
public:
	static FAllCommonEventNative OnAllEventNative;

	UFUNCTION()
	void OnAllEventHandling(int32 event_num);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Event")
	FAllCommonEventBp OnAllEventBp;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Event")
	FCommonEventBp OnCommonEventBp;
#pragma endregion All Common Event

#pragma region
public:
	static FConnectionChangeNative OnConnectionChangeNative;
	UFUNCTION()
	void OnConnectionChangeHandling(uint8 device, bool connection);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Connection")
	FConnectionChangeBp OnConnectionChangeBp;
#pragma endregion Connection Change

#pragma region
public:
	static FAllButtonPressNative OnAllButtonPressNative_HMD;
	static FAllButtonPressNative OnAllButtonPressNative_Right;
	static FAllButtonPressNative OnAllButtonPressNative_Left;

	UFUNCTION()
	void OnAllButtonPressHandling_HMD(uint8 button, bool down);
	UFUNCTION()
	void OnAllButtonPressHandling_Right(uint8 button, bool down);
	UFUNCTION()
	void OnAllButtonPressHandling_Left(uint8 button, bool down);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Button")
	FAllButtonPressBp OnAllButtonPressBp_HMD;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Button")
	FAllButtonPressBp OnAllButtonPressBp_Right;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Button")
	FAllButtonPressBp OnAllButtonPressBp_Left;

	static FAllButtonTouchNative OnAllButtonTouchNative_HMD;
	static FAllButtonTouchNative OnAllButtonTouchNative_Right;
	static FAllButtonTouchNative OnAllButtonTouchNative_Left;

	UFUNCTION()
	void OnAllButtonTouchHandling_HMD(uint8 button, bool down);
	UFUNCTION()
	void OnAllButtonTouchHandling_Right(uint8 button, bool down);
	UFUNCTION()
	void OnAllButtonTouchHandling_Left(uint8 button, bool down);

	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Button")
	FAllButtonTouchBp OnAllButtonTouchBp_HMD;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Button")
	FAllButtonTouchBp OnAllButtonTouchBp_Right;
	UPROPERTY(BlueprintAssignable, Category = "WaveVR|Button")
	FAllButtonTouchBp OnAllButtonTouchBp_Left;
#pragma endregion Button
};
