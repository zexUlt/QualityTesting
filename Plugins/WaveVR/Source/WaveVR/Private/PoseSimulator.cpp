// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "PoseSimulator.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

PoseSimulator::PoseSimulator()
	: bInitialized(false)
	, World(nullptr)
	, PlayerController(nullptr)
	, viewportSize(FVector2D::ZeroVector)
	, viewportCenter(FVector2D::ZeroVector)
	, position_right(FVector::ZeroVector)
	, euler_right(FVector::ZeroVector)
	, position_left(FVector::ZeroVector)
	, euler_left(FVector::ZeroVector)
	, position_head(FVector::ZeroVector)
	, euler_head(FVector::ZeroVector)
{
}

PoseSimulator::~PoseSimulator()
{
}

bool PoseSimulator::IsPlayInEditor()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World()->IsPlayInEditor())
		{
			World = Context.World();
			return true;
		}
	}
	World = nullptr;
	return false;
}

bool PoseSimulator::Initialize()
{
	if (World)
		PlayerController = UGameplayStatics::GetPlayerController(World, 0);

	if (!PlayerController)
		return false;

	PlayerController->bEnableClickEvents = true;

	// In PIE, checks a newly created viewport's existence.
	if (GEngine->GameViewport->Viewport)
	{
		viewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		viewportCenter = FVector2D(viewportSize.X / 2, viewportSize.Y / 2);
	}

	return true;
}

bool PoseSimulator::Validate()
{
	if (!IsPlayInEditor())
		return false;

	if (!Initialize())
		return false;

	return true;
}

void PoseSimulator::TickData()
{
	if (!Validate()) { return; }

	if (PlayerController != nullptr)
	{
		float fX = 0, fY = 0;
		if (!PlayerController->GetMousePosition(fX, fY)) { return; }
		//LOGD(LogPoseSimulator, "GetSimulationPose() mouse:(%f, %f)", fX, fY);

		// Right Hand
		if (PlayerController->IsInputKeyDown(EKeys::RightShift))
		{
			position_right.Z = (viewportCenter.Y - fY) / 3;
			position_right.Y = (fX - viewportCenter.X) / 5;
			//LOGD(LogPoseSimulator, "GetSimulationPose() position_right:(%f, %f, %f)", position_right.X, position_right.Y, position_right.Z);
		}

		if (PlayerController->IsInputKeyDown(EKeys::RightAlt))
		{
			euler_right.Y = ((viewportSize.Y - fY) / viewportSize.Y) * 120;
			euler_right.Z = ((fX - viewportCenter.X) / viewportCenter.X) * 90;
			//LOGD(LogPoseSimulator, "GetSimulationPose() fY: %f, viewportSize.Y: %f, euler_right:(%f, %f, %f)", fY, viewportSize.Y, euler_right.X, euler_right.Y, euler_right.Z);
		}
		/*
		// Left Hand
		if (PlayerController->IsInputKeyDown(EKeys::LeftShift))
		{
			position_left.Z = (viewportCenter.Y - fY) / 3;
			position_left.Y = (fX - viewportCenter.X) / 5;
		}

		if (PlayerController->IsInputKeyDown(EKeys::LeftAlt))
		{
			euler_left.Y = ((viewportSize.Y - fY) / viewportSize.Y) * 120;
			euler_left.Z = ((fX - viewportCenter.X) / viewportCenter.X) * 90;
		}
		*/
		// Head
		if (PlayerController->IsInputKeyDown(EKeys::LeftShift))
		{
			position_head.Z = (viewportCenter.Y - fY) / 3;
			position_head.Y = (fX - viewportCenter.X) / 5;
			//LOGD(LogPoseSimulator, "GetSimulationPose() position_head:(%f, %f, %f)", position_head.X, position_head.Y, position_head.Z);
		}

		if (PlayerController->IsInputKeyDown(EKeys::LeftAlt))
		{
			euler_head.Y = ((viewportCenter.Y - fY) / viewportCenter.Y) * 90;
			euler_head.Z = ((fX - viewportCenter.X) / viewportCenter.X) * 180;
			//LOGD(LogPoseSimulator, "GetSimulationPose() euler_head:(%f, %f, %f)", euler_head.X, euler_head.Y, euler_head.Z);
		}
	}
}

bool PoseSimulator::GetSimulationPressState(WVR_DeviceType device, WVR_InputId id)
{
	if (!Validate())
		return false;

	bool pressed = false;

	if (PlayerController != nullptr)
	{
		switch (device)
		{
		case WVR_DeviceType::WVR_DeviceType_Controller_Left:
			if (id == WVR_InputId::WVR_InputId_Alias1_Touchpad)
				pressed = PlayerController->IsInputKeyDown(EKeys::LeftMouseButton);
			if (id == WVR_InputId::WVR_InputId_Alias1_X)
				pressed = PlayerController->IsInputKeyDown(EKeys::X);
			if (id == WVR_InputId::WVR_InputId_Alias1_Y)
				pressed = PlayerController->IsInputKeyDown(EKeys::Y);
			if (id == WVR_InputId::WVR_InputId_Alias1_Trigger)
				pressed = PlayerController->IsInputKeyDown(EKeys::R);
			break;
		case WVR_DeviceType::WVR_DeviceType_Controller_Right:
			if (id == WVR_InputId::WVR_InputId_Alias1_Touchpad)
				pressed = PlayerController->IsInputKeyDown(EKeys::RightMouseButton);
			if (id == WVR_InputId::WVR_InputId_Alias1_A)
				pressed = PlayerController->IsInputKeyDown(EKeys::A);
			if (id == WVR_InputId::WVR_InputId_Alias1_B)
				pressed = PlayerController->IsInputKeyDown(EKeys::B);
			if (id == WVR_InputId::WVR_InputId_Alias1_Trigger)
				pressed = PlayerController->IsInputKeyDown(EKeys::T);
			break;
		default:
			break;
		}
	}

	return pressed;
}

/// Output pose is UE-bases coordinate.
bool PoseSimulator::GetSimulationPose(FVector& OutPosition, FRotator& OutOrientation, WVR_DeviceType type)
{
	if (type == WVR_DeviceType::WVR_DeviceType_Controller_Right)
	{
		OutPosition = position_right;
		OutOrientation = FRotator::MakeFromEuler(euler_right);
		return true;
	}
	else if (type == WVR_DeviceType::WVR_DeviceType_Controller_Left)
	{
		OutPosition = position_left;
		OutOrientation = FRotator::MakeFromEuler(euler_left);
		return true;
	}
	else if (type == WVR_DeviceType::WVR_DeviceType_HMD)
	{
		OutPosition = position_head;
		OutOrientation = FRotator::MakeFromEuler(euler_head);
		return true;
	}

	return false;
}

bool PoseSimulator::GetSimulationPose(FVector& OutPosition, FQuat& OutOrientation, WVR_DeviceType type)
{
	if (type == WVR_DeviceType::WVR_DeviceType_Controller_Right)
	{
		OutPosition = position_right;
		OutOrientation = FQuat::MakeFromEuler(euler_right);
		return true;
	}
	else if (type == WVR_DeviceType::WVR_DeviceType_Controller_Left)
	{
		OutPosition = position_left;
		OutOrientation = FQuat::MakeFromEuler(euler_left);
		return true;
	}
	else if (type == WVR_DeviceType::WVR_DeviceType_HMD)
	{
		OutPosition = position_head;
		OutOrientation = FQuat::MakeFromEuler(euler_head);
		return true;
	}

	return false;
}
