// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/CustomGesture/WaveVRCustomGesture.h"

#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRCustomGesture, Log, All);

// Sets default values
AWaveVRCustomGesture::AWaveVRCustomGesture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	s_JointPositionsLeft.Init(FVector::ZeroVector, EWaveVRHandJointCount); // count of WVR_HandJoint
	s_JointPositionsRight.Init(FVector::ZeroVector, EWaveVRHandJointCount); // count of WVR_HandJoint
	s_JointRotationsLeft.Init(FQuat::Identity, EWaveVRHandJointCount); // count of WVR_HandJoint
	s_JointRotationsRight.Init(FQuat::Identity, EWaveVRHandJointCount); // count of WVR_HandJoint
}

// Called when the game starts or when spawned
void AWaveVRCustomGesture::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWaveVRCustomGesture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	validPoseLeft = UWaveVRHandBPLibrary::GetAllHandJointPoses(EWaveVRTrackerType::Natural, EWaveVRHandType::Left, s_JointPositionsLeft, s_JointRotationsLeft);
	validPoseRight = UWaveVRHandBPLibrary::GetAllHandJointPoses(EWaveVRTrackerType::Natural, EWaveVRHandType::Right, s_JointPositionsRight, s_JointRotationsRight);

	/// Updates all fingers' states.
	UpdateFingerState();

	/// Checks left gestures.
	bool matchLeft = false;
	for (int i = 0; i < LeftGestures.Num(); i++)
	{
		if (MatchGestureSingle(LeftGestures[i].Setting, true))
		{
			if (!m_LeftHandGesture.Equals(LeftGestures[i].Name))
			{
				m_LeftHandGesture = LeftGestures[i].Name;
				LOGD(LogWaveVRCustomGesture, "Tick() broadcast Left custome gesture %s", PLATFORM_CHAR((*m_LeftHandGesture)));
				UWaveVRHandGestureComponent::OnCustomGestureNative_Left.Broadcast(m_LeftHandGesture);
			}

			matchLeft = true;
			break; // Leaves if matched gesture.
		}
	}
	if (!matchLeft && !m_LeftHandGesture.Equals(kUnknownGesture))
	{
		m_LeftHandGesture = kUnknownGesture;
		LOGD(LogWaveVRCustomGesture, "Tick() broadcast Left custome gesture %s", PLATFORM_CHAR((*m_LeftHandGesture)));
		UWaveVRHandGestureComponent::OnCustomGestureNative_Left.Broadcast(m_LeftHandGesture);
	}

	/// Checks right gestures.
	bool matchRight = false;
	for (int i = 0; i < RightGestures.Num(); i++)
	{
		if (MatchGestureSingle(RightGestures[i].Setting, false))
		{
			if (!m_RightHandGesture.Equals(RightGestures[i].Name))
			{
				m_RightHandGesture = RightGestures[i].Name;
				LOGD(LogWaveVRCustomGesture, "Tick() broadcast Right custome gesture %s", PLATFORM_CHAR((*m_RightHandGesture)));
				UWaveVRHandGestureComponent::OnCustomGestureNative_Right.Broadcast(m_RightHandGesture);
			}

			matchRight = true;
			break; // Leaves if matched gesture.
		}
	}
	if (!matchRight && !m_RightHandGesture.Equals(kUnknownGesture))
	{
		m_RightHandGesture = kUnknownGesture;
		LOGD(LogWaveVRCustomGesture, "Tick() broadcast Right custome gesture %s", PLATFORM_CHAR((*m_RightHandGesture)));
		UWaveVRHandGestureComponent::OnCustomGestureNative_Right.Broadcast(m_RightHandGesture);
	}

	bool matchDual = false;
	for (int i = 0; i < DualHandGestures.Num(); i++)
	{
		if (MatchGestureDual(DualHandGestures[i].Setting))
		{
			if (!m_DualHandGesture.Equals(DualHandGestures[i].Name))
			{
				m_DualHandGesture = DualHandGestures[i].Name;
				LOGD(LogWaveVRCustomGesture, "Tick() broadcast Dual Hand custome gesture %s", PLATFORM_CHAR((*m_DualHandGesture)));
				UWaveVRHandGestureComponent::OnCustomGestureNative_Dual.Broadcast(m_DualHandGesture);
			}

			matchDual = true;
			break; // Leaves if matched gesture.
		}
	}

	if (!matchDual && !m_DualHandGesture.Equals(kUnknownGesture))
	{
		m_DualHandGesture = kUnknownGesture;
		LOGD(LogWaveVRCustomGesture, "Tick() broadcast Dual Hand custome gesture %s", PLATFORM_CHAR((*m_DualHandGesture)));
		UWaveVRHandGestureComponent::OnCustomGestureNative_Dual.Broadcast(m_DualHandGesture);
	}
}

void AWaveVRCustomGesture::UpdateFingerState()
{
	if (validPoseLeft)
	{
		m_ThumbStateLeft = GetThumbState(
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Thumb_Joint1],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Thumb_Joint2],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Thumb_Tip],
			true
		);

		m_IndexStateLeft = GetFingerState(
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Index_Joint1],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Index_Joint2],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Index_Tip],
			true
		);

		m_MiddleStateLeft = GetFingerState(
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Middle_Joint1],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Middle_Joint2],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Middle_Tip],
			true
		);

		m_RingStateLeft = GetFingerState(
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Ring_Joint1],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Ring_Joint2],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Ring_Tip],
			true
		);

		m_PinkyStateLeft = GetFingerState(
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Pinky_Joint1],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Pinky_Joint2],
			s_JointPositionsLeft[(uint8)EWaveVRHandJoint::Pinky_Tip],
			true
		);
	}
	if (validPoseRight)
	{
		m_ThumbStateRight = GetThumbState(
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Thumb_Joint1],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Thumb_Joint2],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Thumb_Tip],
			true
		);

		m_IndexStateRight = GetFingerState(
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Index_Joint1],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Index_Joint2],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Index_Tip],
			true
		);

		m_MiddleStateRight = GetFingerState(
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Middle_Joint1],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Middle_Joint2],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Middle_Tip],
			true
		);

		m_RingStateRight = GetFingerState(
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Ring_Joint1],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Ring_Joint2],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Ring_Tip],
			true
		);

		m_PinkyStateRight = GetFingerState(
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Pinky_Joint1],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Pinky_Joint2],
			s_JointPositionsRight[(uint8)EWaveVRHandJoint::Pinky_Tip],
			true
		);
	}
}
EWaveVRThumbState AWaveVRCustomGesture::GetThumbState(FVector root, FVector node1, FVector top, bool isLeft)
{
	if (isLeft && !validPoseLeft) { return EWaveVRThumbState::None; }
	if (!isLeft && !validPoseRight) { return EWaveVRThumbState::None; }

	return WaveVRHandHelper::GetThumbState(root, node1, top);
}
EWaveVRFingerState AWaveVRCustomGesture::GetFingerState(FVector root, FVector node1, FVector top, bool isLeft)
{
	if (isLeft && !validPoseLeft) { return EWaveVRFingerState::None; }
	if (!isLeft && !validPoseRight) { return EWaveVRFingerState::None; }

	return WaveVRHandHelper::GetFingerState(root, node1, top);
}
bool AWaveVRCustomGesture::MatchThumbState(FThumbState state)
{
	return false;
}

bool AWaveVRCustomGesture::MatchDistanceSingle(EWaveVRHandJoint node1, EWaveVRHandJoint node2, EWaveVRJointDistance distance, bool isLeft)
{
	if (isLeft && !validPoseLeft) { return false; }
	if (!isLeft && !validPoseRight) { return false; }

	float node_dist = isLeft ?
		FVector::Distance(s_JointPositionsLeft[(uint8)node1], s_JointPositionsLeft[(uint8)node2]) :
		FVector::Distance(s_JointPositionsRight[(uint8)node1], s_JointPositionsRight[(uint8)node2]);

	if (distance == EWaveVRJointDistance::Near) { return (node_dist < 2.5f); }
	return (node_dist > 5);
}
bool AWaveVRCustomGesture::MatchDistanceDual(EWaveVRHandJoint leftNode, EWaveVRHandJoint rightNode, EWaveVRJointDistance distance)
{
	if (!validPoseLeft || !validPoseRight) { return false; }

	float node_dist = FVector::Distance(s_JointPositionsLeft[(uint8)leftNode], s_JointPositionsRight[(uint8)rightNode]);

	if (distance == EWaveVRJointDistance::Near) { return (node_dist < 10); }
	return (node_dist > 20);
}

bool AWaveVRCustomGesture::MatchGestureSingle(FSingleHandSetting setting, bool isLeft)
{
	if (isLeft)
	{
		if (!validPoseLeft) { return false; }

		if (!WaveVRHandHelper::MatchThumbState(setting.Thumb, m_ThumbStateLeft)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Index, m_IndexStateLeft)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Middle, m_MiddleStateLeft)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Ring, m_RingStateLeft)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Pinky, m_PinkyStateLeft)) { return false; }
	}
	else
	{
		if (!validPoseRight) { return false; }

		if (!WaveVRHandHelper::MatchThumbState(setting.Thumb, m_ThumbStateRight)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Index, m_IndexStateRight)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Middle, m_MiddleStateRight)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Ring, m_RingStateRight)) { return false; }
		if (!WaveVRHandHelper::MatchFingerState(setting.Pinky, m_PinkyStateRight)) { return false; }
	}

	/// Checks all distance conditions.
	for (int i = 0; i < setting.SingleHandNodeDistances.Num(); i++)
	{
		if (!MatchDistanceSingle(setting.SingleHandNodeDistances[i].Node1, setting.SingleHandNodeDistances[i].Node2, setting.SingleHandNodeDistances[i].Distance, isLeft))
			return false;
	}

	return true;
}
bool AWaveVRCustomGesture::MatchGestureDual(FDualHandSetting setting)
{
	if (!validPoseLeft || !validPoseRight) { return false; }

	if (!MatchGestureSingle(setting.LeftHand, true)) { return false; }
	if (!MatchGestureSingle(setting.RightHand, false)) { return false; }

	/// Checks all distance conditions.
	for (int i = 0; i < setting.DualHandNodeDistances.Num(); i++)
	{
		if (!MatchDistanceDual(setting.DualHandNodeDistances[i].LeftNode, setting.DualHandNodeDistances[i].RightNode, setting.DualHandNodeDistances[i].Distance))
			return false;
	}

	return true;
}
