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
#include "GameFramework/PlayerController.h"
#include "wvr_types.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogPoseSimulator, Log, All);

class PoseSimulator
{
public:
	PoseSimulator();
	~PoseSimulator();

	void TickData();
	bool GetSimulationPose(FVector& OutPosition, FRotator& OutOrientation, WVR_DeviceType type);
	bool GetSimulationPose(FVector& OutPosition, FQuat& OutOrientation, WVR_DeviceType type);
	bool GetSimulationPressState(WVR_DeviceType device, WVR_InputId id);

private:
	bool IsPlayInEditor();
	bool Initialize();
	bool Validate();
	bool bInitialized;

	UWorld * World;
	APlayerController* PlayerController;
	FVector2D viewportSize;
	FVector2D viewportCenter;
	FVector position_right;
	FVector euler_right;
	FVector position_left;
	FVector euler_left;
	FVector position_head;
	FVector euler_head;
};
