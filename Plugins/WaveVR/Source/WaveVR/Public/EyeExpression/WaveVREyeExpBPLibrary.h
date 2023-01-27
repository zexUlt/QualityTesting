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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EyeExpression/WaveVREyeExpUtils.h"
#include "EyeExpression/WaveVREyeExpImpl.h"
#include "WaveVREyeExpBPLibrary.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREyeExpBPLibrary, Log, All);

UCLASS()
class WAVEVR_API UWaveVREyeExpBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye|Expression",
		meta = (ToolTip = "To enable the Eye Expression component."))
	static void StartEyeExp();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye|Expression",
		meta = (ToolTip = "To disable the Eye Expression component."))
	static void StopEyeExp();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye|Expression",
		meta = (ToolTip = "Retrieves the Eye Expression component status."))
	static EWaveVREyeExpStatus GetEyeExpStatus();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye|Expression",
		meta = (ToolTip = "Checks if the Eye Expression is available to use."))
	static bool IsEyeExpAvailable();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye|Expression",
		meta = (ToolTip = "Retrieves an eye expression value."))
	static float GetEyeExpValue(EWaveVREyeExp lipExp);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Eye|Expression",
		meta = (ToolTip = "Retrieves all eye expression data in a float array sorted in the order as EWaveVREyeExp emum."))
	static bool GetEyeExpData(TArray<float>& OutValue);
};
