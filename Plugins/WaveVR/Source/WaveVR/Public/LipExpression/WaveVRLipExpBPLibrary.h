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
#include "LipExpression/WaveVRLipExpUtils.h"
#include "LipExpression/WaveVRLipExpImpl.h"
#include "WaveVRLipExpBPLibrary.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRLipExpBPLibrary, Log, All);

UCLASS()
class WAVEVR_API UWaveVRLipExpBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Lip",
		meta = (ToolTip = "To enable the Lip Expression component."))
	static void StartLipExp();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Lip",
		meta = (ToolTip = "To disable the Lip Expression component."))
	static void StopLipExp();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Lip",
		meta = (ToolTip = "Retrieves the Lip Expression component status."))
	static EWaveVRLipExpStatus GetLipExpStatus();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Lip",
		meta = (ToolTip = "Check if the Lip Expression is available to use."))
	static bool IsLipExpAvailable();

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Lip",
		meta = (ToolTip = "Retrieves a lip expression value."))
	static float GetLipExpValue(EWaveVRLipExp lipExp);

	UFUNCTION(
		BlueprintCallable,
		Category = "WaveVR|Lip",
		meta = (ToolTip = "Retrieve all lip expression data in a float array sorted in the order as EWaveVRLipExp enum."))
	static bool GetLipExpData(TArray<float>& OutValue);
};
