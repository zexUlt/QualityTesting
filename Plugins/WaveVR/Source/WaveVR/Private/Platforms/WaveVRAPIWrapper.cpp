// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRAPIWrapper.h"

class OpenglCoordinate {
public:
	static WVR_Vector3f_t GetVector3(const FVector& ue_pos) {
		WVR_Vector3f_t vec;

		vec.v[0] = ue_pos.Y / 100;
		vec.v[1] = ue_pos.Z / 100;
		vec.v[2] = -ue_pos.X / 100;
		return vec;
	}

	static WVR_Quatf_t GetQuaternion(const FRotator& ue_rot)
	{
		WVR_Quatf_t qat;
		FQuat ue_ori = ue_rot.Quaternion();
		qat.w = -ue_ori.W;
		qat.x = ue_ori.Y;
		qat.y = ue_ori.Z;
		qat.z = -ue_ori.X;
		return qat;
	}

	static WVR_Matrix4f_t GetMatrix44(const FVector& ue_pos, const FRotator& ue_rot)
	{
		WVR_Vector3f_t pos;
		pos.v[0] = ue_pos.Y / 100;
		pos.v[1] = ue_pos.Z / 100;
		pos.v[2] = -ue_pos.X / 100;

		FQuat ue_ori = ue_rot.Quaternion();
		WVR_Quatf_t rot;
		rot.w = -ue_ori.W;
		rot.x = ue_ori.Y;
		rot.y = ue_ori.Z;
		rot.z = -ue_ori.X;

		WVR_Matrix4f_t mat;

		// m[0, 0] = 1 - 2 * y^2 - 2 * z^2
		mat.m[0][0] = 1 - (2 * (rot.y * rot.y)) - (2 * (rot.z * rot.z));
		// m[0, 1] = 2xy - 2zw
		mat.m[0][1] = (2 * rot.x * rot.y) - (2 * rot.z * rot.w);
		// m[0, 2] = 2xz + 2yw
		mat.m[0][2] = (2 * rot.x * rot.z) + (2 * rot.y * rot.w);
		// m[0, 3] = X
		mat.m[0][3] = pos.v[0];
		// m[1, 0] = 2xy + 2zw
		mat.m[1][0] = (2 * rot.x * rot.y) + (2 * rot.z * rot.w);
		// m[1, 1] = 1 - 2 * x^2 - 2 * z^2
		mat.m[1][1] = 1 - (2 * (rot.x * rot.x)) - (2 * (rot.z * rot.z));
		// m[1, 2] = 2yz -2xw
		mat.m[1][2] = (2 * rot.y * rot.z) - (2 * rot.x * rot.w);
		// m[1, 3] = Y
		mat.m[1][3] = pos.v[1];
		// m[2, 0] = 2xz - 2yw
		mat.m[2][0] = (2 * rot.x * rot.z) - (2 * rot.y * rot.w);
		// m[2, 1] = 2yz + 2xw
		mat.m[2][1] = (2 * rot.y * rot.z) + (2 * rot.x * rot.w);
		// m[2, 2] = 1 - 2 * x^2 - 2 * y^2
		mat.m[2][2] = 1 - (2 * rot.x * rot.x) - (2 * rot.y * rot.y);
		// m[2, 3] = Z
		mat.m[2][3] = pos.v[2];
		// m[3, 0] = 0
		mat.m[3][0] = 0;
		// m[3, 1] = 0
		mat.m[3][1] = 0;
		// m[3, 2] = 0
		mat.m[3][2] = 0;
		// m[3, 3] = 1
		mat.m[3][3] = 1;

		return mat;
	}
};

#pragma region
FVector BONE_L_POS_OFFSET = FVector(15, 0, 0);
FVector BONE_L_POS_FUSION_OFFSET = FVector(5, 0, 0);
// Left wrist.
FVector BONE_HAND_WRIST_L_POS		= FVector(20.0f, -9, 0);
FRotator BONE_HAND_WRIST_L_ROT		= FRotator(165, 0, -7);
// Left thumb.
FVector BONE_THUMB_JOINT2_L_POS		= FVector(20.0f, -5, 2);
FVector BONE_THUMB_JOINT3_L_POS		= FVector(20.0f, -4, 3);
FVector BONE_THUMB_TIP_L_POS		= FVector(20.0f, -3, 4);
// Left index.
FVector BONE_INDEX_JOINT1_L_POS		= FVector(20.0f, -6, 4);
FVector BONE_INDEX_JOINT2_L_POS		= FVector(20.0f, -5.6f, 5);
FVector BONE_INDEX_JOINT3_L_POS		= FVector(20.0f, -5.2f, 6);
FVector BONE_INDEX_TIP_L_POS		= FVector(20.0f, -4.8f, 7);
// Left middle.
FVector BONE_MIDDLE_JOINT1_L_POS	= FVector(20.0f, -7.5f, 4.5f);
FVector BONE_MIDDLE_JOINT2_L_POS	= FVector(20.0f, -7.4f, 5.5f);
FVector BONE_MIDDLE_JOINT3_L_POS	= FVector(20.0f, -7.3f, 6.5f);
FVector BONE_MIDDLE_TIP_L_POS		= FVector(20.0f, -7.2f, 7.5f);
// Left ring.
FVector BONE_RING_JOINT1_L_POS		= FVector(20.0f, -8.7f, 4);
FVector BONE_RING_JOINT2_L_POS		= FVector(20.0f, -8.9f, 5);
FVector BONE_RING_JOINT3_L_POS		= FVector(20.0f, -9.1f, 6);
FVector BONE_RING_TIP_L_POS			= FVector(20.0f, -9.3f, 7);
// Left pinky.
FVector BONE_PINKY_JOINT1_L_POS		= FVector(20.0f, -9.9f, 3);
FVector BONE_PINKY_JOINT2_L_POS		= FVector(20.0f, -10.3f, 4);
FVector BONE_PINKY_JOINT3_L_POS		= FVector(20.0f, -10.6f, 5);
FVector BONE_PINKY_TIP_L_POS		= FVector(20.0f, -10.9f, 6);

FVector BONE_R_POS_OFFSET = FVector(15, 0, 0);
FVector BONE_R_POS_FUSION_OFFSET = FVector(5, 0, 0);
// Right wrist.
FVector BONE_HAND_WRIST_R_POS		= FVector(20.0f, 9, 0);
FRotator BONE_HAND_WRIST_R_ROT		= FRotator(165, 0, 7);
// Right thumb.
FVector BONE_THUMB_JOINT2_R_POS		= FVector(20.0f, 5, 2);
FVector BONE_THUMB_JOINT3_R_POS		= FVector(20.0f, 4, 3);
FVector BONE_THUMB_TIP_R_POS	= FVector(20.0f, 3, 4);
// Right index.
FVector BONE_INDEX_JOINT1_R_POS		= FVector(20.0f, 6, 4);
FVector BONE_INDEX_JOINT2_R_POS		= FVector(20.0f, 5.6f, 5);
FVector BONE_INDEX_JOINT3_R_POS		= FVector(20.0f, 5.2f, 6);
FVector BONE_INDEX_TIP_R_POS		= FVector(20.0f, 4.8f, 7);
// Right middle.
FVector BONE_MIDDLE_JOINT1_R_POS	= FVector(20.0f, 7.5f, 4.5f);
FVector BONE_MIDDLE_JOINT2_R_POS	= FVector(20.0f, 7.4f, 5.5f);
FVector BONE_MIDDLE_JOINT3_R_POS	= FVector(20.0f, 7.3f, 6.5f);
FVector BONE_MIDDLE_TIP_R_POS		= FVector(20.0f, 7.2f, 7.5f);
// Right ring.
FVector BONE_RING_JOINT1_R_POS		= FVector(20.0f, 8.7f, 4);
FVector BONE_RING_JOINT2_R_POS		= FVector(20.0f, 8.9f, 5);
FVector BONE_RING_JOINT3_R_POS		= FVector(20.0f, 9.1f, 6);
FVector BONE_RING_TIP_R_POS			= FVector(20.0f, 9.3f, 7);
// Right pinky.
FVector BONE_PINKY_JOINT1_R_POS		= FVector(20.0f, 9.9f, 3);
FVector BONE_PINKY_JOINT2_R_POS		= FVector(20.0f, 10.3f, 4);
FVector BONE_PINKY_JOINT3_R_POS		= FVector(20.0f, 10.6f, 5);
FVector BONE_PINKY_TIP_R_POS		= FVector(20.0f, 10.9f, 6);

FVector PINCH_DIRECTION_R = FVector(1, -.4f, .5f), PINCH_DIRECTION_L = FVector(1, .4f, .5f);
#pragma endregion Hand Tracking data

#pragma region
FVector eyeGazeOrigin = FVector::ZeroVector;
FVector eyeGazeDirection = FVector(1, -0.5, 0);
int gazeDirectionArrayIndex = 0;
FVector gazeDirectionArray[] = {
	FVector(100, -50, 0),
	FVector(100, 0, 0),
	FVector(100, 50, 0)
};
#pragma endregion Eye Tracking data

FWaveVRAPIWrapper * FWaveVRAPIWrapper::instance = nullptr;

FWaveVRAPIWrapper::FWaveVRAPIWrapper()
	: PlayerController(nullptr)
	, focusController(WVR_DeviceType::WVR_DeviceType_Controller_Right)
{
	s_ButtonPressed.Add(WVR_DeviceType::WVR_DeviceType_Controller_Left);
	s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left].Init(false, kButtonCount);
	s_ButtonPressed.Add(WVR_DeviceType::WVR_DeviceType_Controller_Right);
	s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right].Init(false, kButtonCount);

	s_TrackerPressed.Add(WVR_TrackerId::WVR_TrackerId_0);
	s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_0].Init(false, kButtonCount);
	s_TrackerPressed.Add(WVR_TrackerId::WVR_TrackerId_1);
	s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_1].Init(false, kButtonCount);

	// Static Gesture
	gestureData.left = WVR_HandGestureType::WVR_HandGestureType_IndexUp;
	gestureData.right = WVR_HandGestureType::WVR_HandGestureType_OK;
}

FWaveVRAPIWrapper::~FWaveVRAPIWrapper()
{
	if (FWaveVRAPIWrapper::instance == this)
		FWaveVRAPIWrapper::instance = nullptr;
}

void FWaveVRAPIWrapper::SetInstance(FWaveVRAPIWrapper * inst)
{
	FWaveVRAPIWrapper::instance = inst;
}

const uint32_t kGestureCount = 9;
const WVR_HandGestureType s_Gestures[kGestureCount] =
{
	WVR_HandGestureType::WVR_HandGestureType_Invalid,
	WVR_HandGestureType::WVR_HandGestureType_Unknown,
	WVR_HandGestureType::WVR_HandGestureType_Fist,
	WVR_HandGestureType::WVR_HandGestureType_Five,
	WVR_HandGestureType::WVR_HandGestureType_OK,
	WVR_HandGestureType::WVR_HandGestureType_ThumbUp,
	WVR_HandGestureType::WVR_HandGestureType_IndexUp,
	WVR_HandGestureType::WVR_HandGestureType_Palm_Pinch,
	WVR_HandGestureType::WVR_HandGestureType_Yeah,
};
uint32_t gestureIndex = 0;

int32 tickCount = 0, tickAdder = 1;
FRotator leftYawRotator = FRotator(0, 0.1f, 0), rightYawRotator = FRotator(0, -0.1f, 0);
FQuat leftYawOrientation = FQuat::Identity, rightYawOrientation = FQuat::Identity;
int32 gestureCount = 0, eyeDataCount = 0;;
void FWaveVRAPIWrapper::Tick()
{
	tickCount += tickAdder;
	if (tickCount == 100 || tickCount == -100)
	{
		tickAdder *= -1;
	}
	leftYawRotator.Yaw += 0.1f * tickAdder;
	rightYawRotator.Yaw += -0.1f * tickAdder;
	leftYawOrientation = leftYawRotator.Quaternion();
	rightYawOrientation = rightYawRotator.Quaternion();

	// Static Gesture
	gestureCount++;
	gestureCount %= 300;
	if (gestureCount == 0)
	{
		if (gestureData.left == WVR_HandGestureType::WVR_HandGestureType_IndexUp)
			gestureData.left = WVR_HandGestureType::WVR_HandGestureType_Five;
		else
			gestureData.left = WVR_HandGestureType::WVR_HandGestureType_IndexUp;

		gestureIndex++;
		gestureIndex %= kGestureCount;
		gestureData.right = s_Gestures[gestureIndex];
	}

	/**
	 * Eye Tracking Data
	 **/
	eyeDataCount++;
	eyeDataCount %= 300;
	if (eyeDataCount == 0)
	{
		gazeDirectionArrayIndex++;
		gazeDirectionArrayIndex %= UE_ARRAY_COUNT(gazeDirectionArray);
		eyeData.combined.eyeTrackingValidBitMask = (uint64_t)
			(
				WVR_EyeTrackingStatus::WVR_GazeOriginValid |
				WVR_EyeTrackingStatus::WVR_GazeDirectionNormalizedValid
				);
		eyeData.combined.gazeDirectionNormalized = OpenglCoordinate::GetVector3(gazeDirectionArray[gazeDirectionArrayIndex]);
	}
}

static const uint32_t m_JointCount = 20;
WVR_Result FWaveVRAPIWrapper::GetHandJointCount(WVR_HandTrackerType type, uint32_t* jointCount)
{
	*jointCount = m_JointCount;
	return WVR_Result::WVR_Success;
}

const uint64_t kModelTypeBitMask = (uint64_t)(WVR_HandModelType::WVR_HandModelType_WithoutController/* | WVR_HandModelType::WVR_HandModelType_WithController*/);
const uint64_t kJointValidFlag = (uint64_t)(WVR_HandJointValidFlag::WVR_HandJointValidFlag_PositionValid/* | WVR_HandJointValidFlag::WVR_HandJointValidFlag_RotationValid*/);
WVR_HandJoint s_HandJoints[m_JointCount];
uint64_t s_HandJointsFlag[m_JointCount];
WVR_Result FWaveVRAPIWrapper::GetHandTrackerInfo(WVR_HandTrackerType type, WVR_HandTrackerInfo_t* info)
{
	info->jointCount = m_JointCount;
	info->handModelTypeBitMask = kModelTypeBitMask;

	s_HandJoints[0] = WVR_HandJoint::WVR_HandJoint_Wrist;

	s_HandJoints[1] = WVR_HandJoint::WVR_HandJoint_Thumb_Joint1;
	s_HandJoints[2] = WVR_HandJoint::WVR_HandJoint_Thumb_Joint2;
	s_HandJoints[3] = WVR_HandJoint::WVR_HandJoint_Thumb_Tip;

	s_HandJoints[4] = WVR_HandJoint::WVR_HandJoint_Index_Joint1;
	s_HandJoints[5] = WVR_HandJoint::WVR_HandJoint_Index_Joint2;
	s_HandJoints[6] = WVR_HandJoint::WVR_HandJoint_Index_Joint3;
	s_HandJoints[7] = WVR_HandJoint::WVR_HandJoint_Index_Tip;

	s_HandJoints[8] = WVR_HandJoint::WVR_HandJoint_Middle_Joint1;
	s_HandJoints[9] = WVR_HandJoint::WVR_HandJoint_Middle_Joint2;
	s_HandJoints[10] = WVR_HandJoint::WVR_HandJoint_Middle_Joint3;
	s_HandJoints[11] = WVR_HandJoint::WVR_HandJoint_Middle_Tip;

	s_HandJoints[12] = WVR_HandJoint::WVR_HandJoint_Ring_Joint1;
	s_HandJoints[13] = WVR_HandJoint::WVR_HandJoint_Ring_Joint2;
	s_HandJoints[14] = WVR_HandJoint::WVR_HandJoint_Ring_Joint3;
	s_HandJoints[15] = WVR_HandJoint::WVR_HandJoint_Ring_Tip;

	s_HandJoints[16] = WVR_HandJoint::WVR_HandJoint_Pinky_Joint1;
	s_HandJoints[17] = WVR_HandJoint::WVR_HandJoint_Pinky_Joint2;
	s_HandJoints[18] = WVR_HandJoint::WVR_HandJoint_Pinky_Joint3;
	s_HandJoints[19] = WVR_HandJoint::WVR_HandJoint_Pinky_Tip;

	info->jointMappingArray = s_HandJoints;

	for (uint32_t i = 0; i < m_JointCount; i++)
		s_HandJointsFlag[i] = kJointValidFlag;

	info->jointValidFlagArray = s_HandJointsFlag;

	info->pinchTHR = 0.2f;

	return WVR_Result::WVR_Success;
}

WVR_Pose_t m_JointsPoseLeft[m_JointCount], m_JointsPoseRight[m_JointCount];
WVR_Result FWaveVRAPIWrapper::GetHandTrackingData(WVR_HandTrackerType trackerType,
	WVR_HandModelType modelType,
	WVR_PoseOriginModel originModel,
	WVR_HandTrackingData* skeleton,
	WVR_HandPoseData_t* pose)
{
	skeleton->left.isValidPose = true;
	skeleton->left.confidence = 0.8f;
	skeleton->left.jointCount = m_JointCount;

	FVector BONE_L_OFFSET = fusionBracelet ? BONE_L_POS_FUSION_OFFSET : BONE_L_POS_OFFSET;
	// Left wrist.
	m_JointsPoseLeft[0].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_HAND_WRIST_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[0].rotation = OpenglCoordinate::GetQuaternion(BONE_HAND_WRIST_L_ROT);
	// Left thumb.
	m_JointsPoseLeft[1].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_THUMB_JOINT2_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[2].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_THUMB_JOINT3_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[3].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_THUMB_TIP_L_POS + BONE_L_OFFSET));
	// Left index.
	m_JointsPoseLeft[4].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_INDEX_JOINT1_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[5].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_INDEX_JOINT2_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[6].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_INDEX_JOINT3_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[7].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_INDEX_TIP_L_POS + BONE_L_OFFSET));
	// Left middle.
	m_JointsPoseLeft[8].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_MIDDLE_JOINT1_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[9].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_MIDDLE_JOINT2_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[10].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_MIDDLE_JOINT3_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[11].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_MIDDLE_TIP_L_POS + BONE_L_OFFSET));
	// Left ring.
	m_JointsPoseLeft[12].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_RING_JOINT1_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[13].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_RING_JOINT2_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[14].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_RING_JOINT3_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[15].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_RING_TIP_L_POS + BONE_L_OFFSET));
	// Left pinky.
	m_JointsPoseLeft[16].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_PINKY_JOINT1_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[17].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_PINKY_JOINT2_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[18].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_PINKY_JOINT3_L_POS + BONE_L_OFFSET));
	m_JointsPoseLeft[19].position = OpenglCoordinate::GetVector3(leftYawOrientation * (BONE_PINKY_TIP_L_POS + BONE_L_OFFSET));

	skeleton->left.joints = m_JointsPoseLeft;
	skeleton->left.scale.v[0] = 1;
	skeleton->left.scale.v[1] = 1;
	skeleton->left.scale.v[2] = 1;

	skeleton->left.wristLinearVelocity.v[0] = 0.1f;
	skeleton->left.wristLinearVelocity.v[1] = 0.2f;
	skeleton->left.wristLinearVelocity.v[2] = 0.3f;
	skeleton->left.wristAngularVelocity.v[0] = 0.1f;
	skeleton->left.wristAngularVelocity.v[1] = 0.2f;
	skeleton->left.wristAngularVelocity.v[2] = 0.3f;

	skeleton->right.isValidPose = true;
	skeleton->right.confidence = 0.8f;
	skeleton->right.jointCount = m_JointCount;


	FVector BONE_R_OFFSET = fusionBracelet ? BONE_R_POS_FUSION_OFFSET : BONE_R_POS_OFFSET;
	// Right wrist.
	m_JointsPoseRight[0].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_HAND_WRIST_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[0].rotation = OpenglCoordinate::GetQuaternion(BONE_HAND_WRIST_R_ROT);
	// Right thumb.
	m_JointsPoseRight[1].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_THUMB_JOINT2_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[2].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_THUMB_JOINT3_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[3].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_THUMB_TIP_R_POS + BONE_R_OFFSET));
	// Right index.
	m_JointsPoseRight[4].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_INDEX_JOINT1_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[5].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_INDEX_JOINT2_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[6].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_INDEX_JOINT3_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[7].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_INDEX_TIP_R_POS + BONE_R_OFFSET));
	// Right middle.
	m_JointsPoseRight[8].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_MIDDLE_JOINT1_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[9].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_MIDDLE_JOINT2_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[10].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_MIDDLE_JOINT3_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[11].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_MIDDLE_TIP_R_POS + BONE_R_OFFSET));
	// Right ring.
	m_JointsPoseRight[12].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_RING_JOINT1_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[13].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_RING_JOINT2_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[14].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_RING_JOINT3_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[15].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_RING_TIP_R_POS + BONE_R_OFFSET));
	// Right pinky.
	m_JointsPoseRight[16].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_PINKY_JOINT1_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[17].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_PINKY_JOINT2_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[18].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_PINKY_JOINT3_R_POS + BONE_R_OFFSET));
	m_JointsPoseRight[19].position = OpenglCoordinate::GetVector3(rightYawOrientation * (BONE_PINKY_TIP_R_POS + BONE_R_OFFSET));

	skeleton->right.joints = m_JointsPoseRight;
	skeleton->right.scale.v[0] = 0.5f;
	skeleton->right.scale.v[1] = 0.5f;
	skeleton->right.scale.v[2] = 0.5f;

	skeleton->right.wristLinearVelocity.v[0] = 0.11f;
	skeleton->right.wristLinearVelocity.v[1] = 0.21f;
	skeleton->right.wristLinearVelocity.v[2] = 0.31f;
	skeleton->right.wristAngularVelocity.v[0] = 0.11f;
	skeleton->right.wristAngularVelocity.v[1] = 0.21f;
	skeleton->right.wristAngularVelocity.v[2] = 0.31f;

	/// Pinch data
	/*
	pose->left.base.type = WVR_HandPoseType::WVR_HandPoseType_Pinch;
	pose->left.pinch.strength = 0.6f;
	pose->left.pinch.origin = m_JointsPoseLeft[0].position;
	pose->left.pinch.direction = OpenglCoordinate::GetVector3(PINCH_DIRECTION_L);

	pose->right.base.type = WVR_HandPoseType::WVR_HandPoseType_Pinch;
	pose->right.pinch.base.type = WVR_HandPoseType::WVR_HandPoseType_Pinch;
	pose->right.pinch.finger = WVR_FingerType::WVR_FingerType_Index;
	pose->right.pinch.strength = 0.8f;
	pose->right.pinch.origin = m_JointsPoseRight[0].position;
	pose->right.pinch.direction = OpenglCoordinate::GetVector3(PINCH_DIRECTION_R);
	*/

	/// Hold data
	pose->left.base.type = WVR_HandPoseType::WVR_HandPoseType_Hold;
	pose->left.hold.role = WVR_HandHoldRoleType::WVR_HandHoldRoleType_SideHold;
	pose->left.hold.object = WVR_HandHoldObjectType::WVR_HandHoldObjectType_Gun;

	pose->right.base.type = WVR_HandPoseType::WVR_HandPoseType_Hold;
	pose->right.hold.role = WVR_HandHoldRoleType::WVR_HandHoldRoleType_MainHold;
	pose->right.hold.object = WVR_HandHoldObjectType::WVR_HandHoldObjectType_Gun;

	return WVR_Result::WVR_Success;
}

bool FWaveVRAPIWrapper::AllowPollEventQueue()
{
	if (framePollEventQueue != GFrameCounter)
	{
		framePollEventQueue = GFrameCounter;
		return true;
	}
	return false;
}
void FWaveVRAPIWrapper::CheckPlayerController()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World()->IsPlayInEditor())
		{
			PlayerController = UGameplayStatics::GetPlayerController(Context.World(), 0);
			return;
		}
	}

	PlayerController = nullptr;
}
bool FWaveVRAPIWrapper::PollEventQueue(WVR_Event_t* event)
{
	if (!AllowPollEventQueue()) { return false; }

	CheckPlayerController();
	if (PlayerController == nullptr || event == nullptr) { return false; }
	PlayerController->bEnableClickEvents = true;

	// Left buttons
	if (PlayerController->IsInputKeyDown(EKeys::LeftMouseButton))
	{
		if (s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left][(uint8)WVR_InputId::WVR_InputId_Alias1_Touchpad])
			return false;

		s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left][(uint8)WVR_InputId::WVR_InputId_Alias1_Touchpad] = true;

		event->input.device.common.type = WVR_EventType::WVR_EventType_ButtonPressed;
		event->input.device.common.timestamp = GFrameCounter;
		event->input.device.deviceType = WVR_DeviceType::WVR_DeviceType_Controller_Left;
		event->input.inputId = WVR_InputId::WVR_InputId_Alias1_Touchpad;
		return true;
	}
	if (PlayerController->IsInputKeyDown(EKeys::R))
	{
		if (s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left][(uint8)WVR_InputId::WVR_InputId_Alias1_Trigger])
			return false;

		s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left][(uint8)WVR_InputId::WVR_InputId_Alias1_Trigger] = true;

		event->input.device.common.type = WVR_EventType::WVR_EventType_ButtonPressed;
		event->input.device.common.timestamp = GFrameCounter;
		event->input.device.deviceType = WVR_DeviceType::WVR_DeviceType_Controller_Left;
		event->input.inputId = WVR_InputId::WVR_InputId_Alias1_Trigger;
		return true;
	}

	// Right buttons
	if (PlayerController->IsInputKeyDown(EKeys::RightMouseButton))
	{
		if (s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right][(uint8)WVR_InputId::WVR_InputId_Alias1_Touchpad])
			return false;

		s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right][(uint8)WVR_InputId::WVR_InputId_Alias1_Touchpad] = true;

		event->input.device.common.type = WVR_EventType::WVR_EventType_ButtonPressed;
		event->input.device.common.timestamp = GFrameCounter;
		event->input.device.deviceType = WVR_DeviceType::WVR_DeviceType_Controller_Right;
		event->input.inputId = WVR_InputId::WVR_InputId_Alias1_Touchpad;
		return true;
	}
	if (PlayerController->IsInputKeyDown(EKeys::T))
	{
		if (s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right][(uint8)WVR_InputId::WVR_InputId_Alias1_Trigger])
			return false;

		s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right][(uint8)WVR_InputId::WVR_InputId_Alias1_Trigger] = true;

		event->input.device.common.type = WVR_EventType::WVR_EventType_ButtonPressed;
		event->input.device.common.timestamp = GFrameCounter;
		event->input.device.deviceType = WVR_DeviceType::WVR_DeviceType_Controller_Right;
		event->input.inputId = WVR_InputId::WVR_InputId_Alias1_Trigger;
		return true;
	}

	// Tracker buttons
	if (PlayerController->IsInputKeyDown(EKeys::P))
	{
		if (s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_0][(uint8)WVR_InputId::WVR_InputId_Alias1_A])
			return false;

		s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_0][(uint8)WVR_InputId::WVR_InputId_Alias1_A] = true;

		event->trackerInput.tracker.common.type = WVR_EventType::WVR_EventType_TrackerButtonPressed;
		event->trackerInput.tracker.common.timestamp = GFrameCounter;
		event->trackerInput.tracker.trackerId = WVR_TrackerId::WVR_TrackerId_0;
		event->trackerInput.inputId = WVR_InputId::WVR_InputId_Alias1_A;
		return true;
	}
	if (PlayerController->IsInputKeyDown(EKeys::Q))
	{
		if (s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_1][(uint8)WVR_InputId::WVR_InputId_Alias1_X])
			return false;

		s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_1][(uint8)WVR_InputId::WVR_InputId_Alias1_X] = true;

		event->trackerInput.tracker.common.type = WVR_EventType::WVR_EventType_TrackerButtonPressed;
		event->trackerInput.tracker.common.timestamp = GFrameCounter;
		event->trackerInput.tracker.trackerId = WVR_TrackerId::WVR_TrackerId_1;
		event->trackerInput.inputId = WVR_InputId::WVR_InputId_Alias1_X;
		return true;
	}

	// No button pressed
	for (int i = 0; i < kButtonCount; i++)
	{
		if (s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left][i])
		{
			s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Left][i] = false;

			event->input.device.common.type = WVR_EventType::WVR_EventType_ButtonUnpressed;
			event->input.device.common.timestamp = GFrameCounter;
			event->input.device.deviceType = WVR_DeviceType::WVR_DeviceType_Controller_Left;
			event->input.inputId = (WVR_InputId)i;
			return true;
		}
		if (s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right][i])
		{
			s_ButtonPressed[WVR_DeviceType::WVR_DeviceType_Controller_Right][i] = false;

			event->input.device.common.type = WVR_EventType::WVR_EventType_ButtonUnpressed;
			event->input.device.common.timestamp = GFrameCounter;
			event->input.device.deviceType = WVR_DeviceType::WVR_DeviceType_Controller_Right;
			event->input.inputId = (WVR_InputId)i;
			return true;
		}
		if (s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_0][i])
		{
			s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_0][i] = false;

			event->trackerInput.tracker.common.type = WVR_EventType::WVR_EventType_TrackerButtonUnpressed;
			event->trackerInput.tracker.common.timestamp = GFrameCounter;
			event->trackerInput.tracker.trackerId = WVR_TrackerId::WVR_TrackerId_0;
			event->trackerInput.inputId = WVR_InputId::WVR_InputId_Alias1_A;
			return true;
		}
		if (s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_1][i])
		{
			s_TrackerPressed[WVR_TrackerId::WVR_TrackerId_1][i] = false;

			event->trackerInput.tracker.common.type = WVR_EventType::WVR_EventType_TrackerButtonUnpressed;
			event->trackerInput.tracker.common.timestamp = GFrameCounter;
			event->trackerInput.tracker.trackerId = WVR_TrackerId::WVR_TrackerId_1;
			event->trackerInput.inputId = WVR_InputId::WVR_InputId_Alias1_X;
			return true;
		}
	}

	return false;
}
