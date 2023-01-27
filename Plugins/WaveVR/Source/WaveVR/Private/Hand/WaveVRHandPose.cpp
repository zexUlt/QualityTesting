// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/WaveVRHandPose.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "HeadMountedDisplayTypes.h"
#include "XRTrackingSystemBase.h"

#include "WaveVRBlueprintFunctionLibrary.h"
#include "Hand/WaveVRHandGestureComponent.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandPose, Log, All);

WaveVRHandPose * WaveVRHandPose::Instance = nullptr;
WVR_HandGestureData handGestureData;
WVR_HandTrackerInfo_t m_NaturalTrackerInfo;
WVR_HandTrackingData_t m_NaturalHandTrackerData;
WVR_HandPoseData_t m_NaturalHandPoseData;
WVR_HandTrackerInfo_t m_ElectronicTrackerInfo;
WVR_HandTrackingData_t m_ElectronicHandTrackerData;
WVR_HandPoseData_t m_ElectronicHandPoseData;

WaveVRHandPose::WaveVRHandPose()
{
	Instance = this;
}
WaveVRHandPose::~WaveVRHandPose()
{
	Instance = nullptr;
}

#pragma region
void WaveVRHandPose::InitHandData()
{
	m_EnableHandGesture = false;
	bHandGestureDataUpdated = false;
	prevStaticGestureRight = EWaveVRGestureType::Invalid;
	prevStaticGestureLeft = EWaveVRGestureType::Invalid;
	currStaticGestureRight = EWaveVRGestureType::Invalid;
	currStaticGestureLeft = EWaveVRGestureType::Invalid;

	m_EnableNaturalTracker = false;
	hasNaturalTrackerInfo = false;
	hasNaturalTrackerData = false;
	m_NaturalTrackerStopTick = 0;
	m_NaturalTrackerStartTick = 0;

	m_EnableElectronicTracker = false;
	hasElectronicTrackerInfo = false;
	hasElectronicTrackerData = false;
	m_ElectronicTrackerStopTick = 0;
	m_ElectronicTrackerStartTick = 0;

	m_NaturalTrackerInfo.jointMappingArray = nullptr;
	m_NaturalTrackerInfo.jointValidFlagArray = nullptr;
	m_NaturalTrackerInfo.pinchTHR = 0;
	m_NaturalHandTrackerData.left.joints = nullptr;
	m_NaturalHandTrackerData.right.joints = nullptr;

	s_NaturalJointPositionLeft.Init(FVector::ZeroVector, EWaveVRHandJointCount); // count of WVR_HandJoint
	s_NaturalJointRotationLeft.Init(FQuat::Identity, EWaveVRHandJointCount); // count of WVR_HandJoint
	s_NaturalJointPositionRight.Init(FVector::ZeroVector, EWaveVRHandJointCount); // count of WVR_HandJoint
	s_NaturalJointRotationRight.Init(FQuat::Identity, EWaveVRHandJointCount); // count of WVR_HandJoint

	m_ElectronicTrackerInfo.jointMappingArray = nullptr;
	m_ElectronicTrackerInfo.jointValidFlagArray = nullptr;
	m_ElectronicTrackerInfo.pinchTHR = 0;
	m_ElectronicHandTrackerData.left.joints = nullptr;
	m_ElectronicHandTrackerData.right.joints = nullptr;

	s_ElectronicJointPositionLeft.Init(FVector::ZeroVector, 26); // count of WVR_HandJoint
	s_ElectronicJointRotationLeft.Init(FQuat::Identity, 26); // count of WVR_HandJoint
	s_ElectronicJointPositionRight.Init(FVector::ZeroVector, 26); // count of WVR_HandJoint
	s_ElectronicJointRotationRight.Init(FQuat::Identity, 26); // count of WVR_HandJoint

	m_HandThread = FWaveVRHandThread::JoyInit();
	BONE_OFFSET_L = FVector(0, 0, 0);
	BONE_OFFSET_R = FVector(0, 0, 0);

	LOGD(LogWaveVRHandPose, "InitHandData()");
}
void WaveVRHandPose::UpdateLeftHandGestureData()
{
	// Update the left gesture.
	prevStaticGestureLeft = currStaticGestureLeft;
	currStaticGestureLeft = EWaveVRGestureType::Invalid;

	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_Unknown)
		currStaticGestureLeft = EWaveVRGestureType::Unknown;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_Fist)
		currStaticGestureLeft = EWaveVRGestureType::Fist;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_Five)
		currStaticGestureLeft = EWaveVRGestureType::Five;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_OK)
		currStaticGestureLeft = EWaveVRGestureType::OK;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_ThumbUp)
		currStaticGestureLeft = EWaveVRGestureType::Like;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_IndexUp)
		currStaticGestureLeft = EWaveVRGestureType::Point;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_Palm_Pinch)
		currStaticGestureLeft = EWaveVRGestureType::Palm_Pinch;
	if (handGestureData.left == WVR_HandGestureType::WVR_HandGestureType_Yeah)
		currStaticGestureLeft = EWaveVRGestureType::Yeah;

	if (currStaticGestureLeft != prevStaticGestureLeft)
	{
		LOGD(LogWaveVRHandPose, "UpdateLeftHandGestureData() broadcast left static gesture %d", (uint8)currStaticGestureLeft);
		UWaveVRHandGestureComponent::OnStaticGestureNative_Left.Broadcast(currStaticGestureLeft);
	}
}
void WaveVRHandPose::UpdateRightHandGestureData()
{
	// Update the right gesture.
	prevStaticGestureRight = currStaticGestureRight;
	currStaticGestureRight = EWaveVRGestureType::Invalid;

	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_Unknown)
		currStaticGestureRight = EWaveVRGestureType::Unknown;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_Fist)
		currStaticGestureRight = EWaveVRGestureType::Fist;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_Five)
		currStaticGestureRight = EWaveVRGestureType::Five;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_OK)
		currStaticGestureRight = EWaveVRGestureType::OK;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_ThumbUp)
		currStaticGestureRight = EWaveVRGestureType::Like;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_IndexUp)
		currStaticGestureRight = EWaveVRGestureType::Point;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_Palm_Pinch)
		currStaticGestureRight = EWaveVRGestureType::Palm_Pinch;
	if (handGestureData.right == WVR_HandGestureType::WVR_HandGestureType_Yeah)
		currStaticGestureRight = EWaveVRGestureType::Yeah;

	if (currStaticGestureRight != prevStaticGestureRight)
	{
		LOGD(LogWaveVRHandPose, "UpdateRightHandGestureData() broadcast right static gesture %d", (uint8)currStaticGestureRight);
		UWaveVRHandGestureComponent::OnStaticGestureNative_Right.Broadcast(currStaticGestureRight);
	}
}
void WaveVRHandPose::TickHandData()
{
	logCount++;
	logCount %= 500;
	printable = (logCount == 0);

	if (m_HandThread == NULL)
	{
		bHandGestureDataUpdated = false;

		hasNaturalTrackerInfo = false;
		hasNaturalTrackerData = false;

		hasElectronicTrackerInfo = false;
		hasElectronicTrackerData = false;
		return;
	}

	if (GWorld && GWorld->GetWorld()->WorldType == EWorldType::Type::Editor)
		return;

	/* ----------------------- Hand Gesture -----------------------*/
	EWaveVRHandGestureStatus hand_gesture_status = m_HandThread->GetHandGestureStatus();
	if (hand_gesture_status == EWaveVRHandGestureStatus::AVAILABLE)
	{
		if (!m_EnableHandGesture
			//|| (!UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_HandGesture_Right) &&
				//!UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_HandGesture_Left))
			)
		{
			LOGD(LogWaveVRHandPose, "TickHandData() Stops Hand Gesture.");
			m_HandThread->StopHandGesture();
		}
		else
		{
			bHandGestureDataUpdated = FWaveVRAPIWrapper::GetInstance()->GetHandGestureData(&handGestureData) == WVR_Result::WVR_Success ? true : false;
			if (bHandGestureDataUpdated)
			{
				UpdateLeftHandGestureData();
				UpdateRightHandGestureData();
			}
		}
	}
	else if (hand_gesture_status == EWaveVRHandGestureStatus::NOT_START || hand_gesture_status == EWaveVRHandGestureStatus::START_FAILURE)
	{
		bHandGestureDataUpdated = false;
		if (m_EnableHandGesture
			//&& (UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_HandGesture_Right) ||
				//UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_HandGesture_Left))
			)
		{
			LOGD(LogWaveVRHandPose, "TickHandData() Starts Hand Gesture.");
			m_HandThread->StartHandGesture();
		}
	}



	/* ----------------------- Hand Tracking -----------------------*/
	EWaveVRHandTrackingStatus natural_tracker_status = m_HandThread->GetHandTrackingStatus(EWaveVRTrackerType::Natural);
	if (natural_tracker_status == EWaveVRHandTrackingStatus::AVAILABLE)
	{
		m_NaturalTrackerStartTick = 0;

		if (!m_EnableNaturalTracker
			//|| (!UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_NaturalHand_Right) &&
				//!UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_NaturalHand_Left))
			)
		{
			hasNaturalTrackerInfo = false;
			hasNaturalTrackerData = false;

			if (m_NaturalTrackerStopTick == 0)
			{
				LOGD(LogWaveVRHandPose, "TickHandData() Stop Natural Hand Tracker.");
				m_HandThread->StopHandTracking(EWaveVRTrackerType::Natural);
			}
			m_NaturalTrackerStopTick++;
			m_NaturalTrackerStopTick %= 60;
		}
		else
		{
			// Calls GetHandJointCount one time after starting tracker.
			if (!hasNaturalTrackerInfo)
			{
				WVR_Result result = WVR()->GetHandJointCount(WVR_HandTrackerType::WVR_HandTrackerType_Natural, &m_NaturalTrackerInfo.jointCount);
				if (result == WVR_Result::WVR_Success)
				{
					LOGD(LogWaveVRHandPose, "TickHandData() Natural tracker, get joint count %d", m_NaturalTrackerInfo.jointCount);

					/// Initialize m_NaturalTrackerInfo
					m_NaturalTrackerInfo.handModelTypeBitMask = 0;
					if (m_NaturalTrackerInfo.jointMappingArray != nullptr)
					{
						memset(m_NaturalTrackerInfo.jointMappingArray, 0, sizeof(WVR_HandJoint)* m_NaturalTrackerInfo.jointCount);
						delete[] m_NaturalTrackerInfo.jointMappingArray;
						m_NaturalTrackerInfo.jointMappingArray = nullptr;

						LOGD(LogWaveVRHandPose, "TickHandData() Release m_NaturalTrackerInfo.jointMappingArray.");
					}
					m_NaturalTrackerInfo.jointMappingArray = new WVR_HandJoint[m_NaturalTrackerInfo.jointCount];
					memset(m_NaturalTrackerInfo.jointMappingArray, 0, sizeof(WVR_HandJoint)* m_NaturalTrackerInfo.jointCount);
					LOGD(LogWaveVRHandPose, "TickHandData() m_NaturalTrackerInfo.jointMappingArray %p", m_NaturalTrackerInfo.jointMappingArray);

					if (m_NaturalTrackerInfo.jointValidFlagArray != nullptr)
					{
						memset(m_NaturalTrackerInfo.jointValidFlagArray, 0, sizeof(uint64_t)* m_NaturalTrackerInfo.jointCount);
						delete[] m_NaturalTrackerInfo.jointValidFlagArray;
						m_NaturalTrackerInfo.jointValidFlagArray = nullptr;

						LOGD(LogWaveVRHandPose, "TickHandData() Release m_NaturalTrackerInfo.jointValidFlagArray.");
					}
					m_NaturalTrackerInfo.jointValidFlagArray = new uint64_t[m_NaturalTrackerInfo.jointCount];
					memset(m_NaturalTrackerInfo.jointValidFlagArray, 0, sizeof(uint64_t)* m_NaturalTrackerInfo.jointCount);
					LOGD(LogWaveVRHandPose, "TickHandData() m_NaturalTrackerInfo.jointValidFlagArray %p", m_NaturalTrackerInfo.jointValidFlagArray);

					m_NaturalTrackerInfo.pinchTHR = 0;

					hasNaturalTrackerInfo =
						(WVR()->GetHandTrackerInfo(WVR_HandTrackerType::WVR_HandTrackerType_Natural, &m_NaturalTrackerInfo) == WVR_Result::WVR_Success ? true : false);

					if (hasNaturalTrackerInfo)
					{
						LOGD(LogWaveVRHandPose, "TickHandData() Natural tracker, joint %d, pinchTHR %f",
							m_NaturalTrackerInfo.jointCount,
							m_NaturalTrackerInfo.pinchTHR);

						for (int32_t i = 0; i < (int32_t)m_NaturalTrackerInfo.jointCount; i++)
						{
							LOGD(LogWaveVRHandPose, "TickHandData() Natural tracker, jointMappingArray[%d] = %d, jointValidFlagArray[%d] = %d",
								i, (uint8)m_NaturalTrackerInfo.jointMappingArray[i],
								i, (uint8)m_NaturalTrackerInfo.jointValidFlagArray[i]);
						}

						/// Initialize m_NaturalHandTrackerData
						m_NaturalHandTrackerData.timestamp = 0;
						m_NaturalHandTrackerData.left.confidence = 0;
						m_NaturalHandTrackerData.left.isValidPose = false;
						m_NaturalHandTrackerData.left.jointCount = m_NaturalTrackerInfo.jointCount;
						if (m_NaturalHandTrackerData.left.joints != nullptr)
						{
							memset(m_NaturalHandTrackerData.left.joints, 0, sizeof(WVR_Pose_t)* m_NaturalTrackerInfo.jointCount);
							delete[]m_NaturalHandTrackerData.left.joints;
							m_NaturalHandTrackerData.left.joints = nullptr;

							LOGD(LogWaveVRHandPose, "TickHandData() Release m_NaturalHandTrackerData.left.joints.");
						}
						m_NaturalHandTrackerData.left.joints = new WVR_Pose_t[m_NaturalTrackerInfo.jointCount];
						memset(m_NaturalHandTrackerData.left.joints, 0, sizeof(WVR_Pose_t)* m_NaturalTrackerInfo.jointCount);
						LOGD(LogWaveVRHandPose, "TickHandData() m_NaturalHandTrackerData.left.joints %p", m_NaturalHandTrackerData.left.joints);
						m_NaturalHandTrackerData.left.scale.v[0] = 0;
						m_NaturalHandTrackerData.left.scale.v[1] = 0;
						m_NaturalHandTrackerData.left.scale.v[2] = 0;

						m_NaturalHandTrackerData.right.confidence = 0;
						m_NaturalHandTrackerData.right.isValidPose = false;
						m_NaturalHandTrackerData.right.jointCount = m_NaturalTrackerInfo.jointCount;
						if (m_NaturalHandTrackerData.right.joints != nullptr)
						{
							memset(m_NaturalHandTrackerData.right.joints, 0, sizeof(WVR_Pose_t)* m_NaturalTrackerInfo.jointCount);
							delete[] m_NaturalHandTrackerData.right.joints;
							m_NaturalHandTrackerData.right.joints = nullptr;

							LOGD(LogWaveVRHandPose, "TickHandData() Release m_NaturalHandTrackerData.right.joints.");
						}
						m_NaturalHandTrackerData.right.joints = new WVR_Pose_t[m_NaturalTrackerInfo.jointCount];
						memset(m_NaturalHandTrackerData.right.joints, 0, sizeof(WVR_Pose_t)* m_NaturalTrackerInfo.jointCount);
						LOGD(LogWaveVRHandPose, "TickHandData() m_NaturalHandTrackerData.right.joints %p", m_NaturalHandTrackerData.right.joints);
						m_NaturalHandTrackerData.right.scale.v[0] = 0;
						m_NaturalHandTrackerData.right.scale.v[1] = 0;
						m_NaturalHandTrackerData.right.scale.v[2] = 0;


						/// Initialize m_NaturalHandPoseData
						m_NaturalHandPoseData.timestamp = 0;
						m_NaturalHandPoseData.left.base.type = WVR_HandPoseType::WVR_HandPoseType_Invalid;
						m_NaturalHandPoseData.right.base.type = WVR_HandPoseType::WVR_HandPoseType_Invalid;
					}
				}
			} // if (!hasNaturalTrackerInfo)

			// Calls GetHandTrackingData on each frame.
			if (hasNaturalTrackerInfo &&
				((m_NaturalTrackerInfo.handModelTypeBitMask & (uint64_t)WVR_HandModelType::WVR_HandModelType_WithoutController) != 0))
			{
				IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
				EHMDTrackingOrigin::Type dofType = XRSystem->GetTrackingOrigin();
				WVR_PoseOriginModel originModel =
					(dofType == EHMDTrackingOrigin::Type::Floor ?
						WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnGround : WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnHead);

				hasNaturalTrackerData = (
					WVR()->GetHandTrackingData(
						WVR_HandTrackerType::WVR_HandTrackerType_Natural,
						WVR_HandModelType::WVR_HandModelType_WithoutController,
						originModel,
						&m_NaturalHandTrackerData,
						&m_NaturalHandPoseData
					) == WVR_Result::WVR_Success ? true : false);

				if (hasNaturalTrackerData)
				{
					{
						auto wvrScaleL = m_NaturalHandTrackerData.left.scale;
						auto wvrScaleR = m_NaturalHandTrackerData.right.scale;
						NaturalHandScaleL = FVector(wvrScaleL.v[2], wvrScaleL.v[0], wvrScaleL.v[1]);
						NaturalHandScaleR = FVector(wvrScaleR.v[2], wvrScaleR.v[0], wvrScaleR.v[1]);

						NaturalWristLinearVelocityL = CoordinateUtil::GetVector3(m_NaturalHandTrackerData.left.wristLinearVelocity, GetWorldToMetersScale());
						NaturalWristLinearVelocityR = CoordinateUtil::GetVector3(m_NaturalHandTrackerData.right.wristLinearVelocity, GetWorldToMetersScale());
						NaturalWristAngularVelocityL = CoordinateUtil::GetVector3(m_NaturalHandTrackerData.left.wristAngularVelocity, GetWorldToMetersScale());
						NaturalWristAngularVelocityR = CoordinateUtil::GetVector3(m_NaturalHandTrackerData.right.wristAngularVelocity, GetWorldToMetersScale());
					}
					for (uint32_t i = 0; i < m_NaturalTrackerInfo.jointCount; i++)
					{
						if ((m_NaturalTrackerInfo.jointValidFlagArray[i] & (uint64_t)WVR_HandJointValidFlag::WVR_HandJointValidFlag_PositionValid) != 0)
						{
							s_NaturalJointPositionLeft[(uint8)m_NaturalTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetVector3(m_NaturalHandTrackerData.left.joints[i].position, GetWorldToMetersScale());
							s_NaturalJointPositionRight[(uint8)m_NaturalTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetVector3(m_NaturalHandTrackerData.right.joints[i].position, GetWorldToMetersScale());
						}
					}
					for (uint32_t i = 0; i < m_NaturalTrackerInfo.jointCount; i++)
					{
						if ((m_NaturalTrackerInfo.jointValidFlagArray[i] & (uint64_t)WVR_HandJointValidFlag::WVR_HandJointValidFlag_RotationValid) != 0)
						{
							s_NaturalJointRotationLeft[(uint8)m_NaturalTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetQuaternion(m_NaturalHandTrackerData.left.joints[i].rotation);
							s_NaturalJointRotationRight[(uint8)m_NaturalTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetQuaternion(m_NaturalHandTrackerData.right.joints[i].rotation);
						}
					}

					if (printable)
					{
						LOGD(LogWaveVRHandPose, "TickComponent() left scale (%f, %f, %f), right scale (%f, %f, %f)",
							NaturalHandScaleL.X, NaturalHandScaleL.Y, NaturalHandScaleL.Z,
							NaturalHandScaleR.X, NaturalHandScaleR.Y, NaturalHandScaleR.Z);

						for (uint32_t i = 0; i < m_NaturalTrackerInfo.jointCount; i++)
						{
							uint8 joint = (uint8)m_NaturalTrackerInfo.jointMappingArray[i];
							LOGD(LogWaveVRHandPose, "TickHandData() Natural tracker joint %d, left (%f, %f, %f), right (%f, %f, %f)"
								, joint
								, s_NaturalJointPositionLeft[joint].X, s_NaturalJointPositionLeft[joint].Y, s_NaturalJointPositionLeft[joint].Z
								, s_NaturalJointPositionRight[joint].X, s_NaturalJointPositionRight[joint].Y, s_NaturalJointPositionRight[joint].Z);
						}
					}
				}
			}
			else
			{
				hasNaturalTrackerData = false;
			}
		}
	}
	else if (natural_tracker_status == EWaveVRHandTrackingStatus::NOT_START || natural_tracker_status == EWaveVRHandTrackingStatus::START_FAILURE)
	{
		m_NaturalTrackerStopTick = 0;

		if (m_EnableNaturalTracker
			//&& (UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_NaturalHand_Right) ||
				//UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_NaturalHand_Left))
			)
		{
			if (m_NaturalTrackerStartTick == 0)
			{
				LOGD(LogWaveVRHandPose, "TickHandData() Start Natural Hand Tracker.");
				m_HandThread->StartHandTracking(EWaveVRTrackerType::Natural);
			}
			m_NaturalTrackerStartTick++;
			m_NaturalTrackerStartTick %= 60;
		}
	}

	EWaveVRHandTrackingStatus electronic_tracker_status = m_HandThread->GetHandTrackingStatus(EWaveVRTrackerType::Electronic);
	if (electronic_tracker_status == EWaveVRHandTrackingStatus::AVAILABLE)
	{
		m_ElectronicTrackerStartTick = 0;

		if (!m_EnableElectronicTracker
			//|| (!UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_ElectronicHand_Right) &&
				//!UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_ElectronicHand_Left))
			)
		{
			hasElectronicTrackerInfo = false;
			hasElectronicTrackerData = false;

			if (m_ElectronicTrackerStopTick == 0)
			{
				LOGD(LogWaveVRHandPose, "TickHandData() Stop Electronic Hand Tracker.");
				m_HandThread->StopHandTracking(EWaveVRTrackerType::Electronic);
			}
			m_ElectronicTrackerStopTick++;
			m_ElectronicTrackerStopTick %= 60;
		}
		else
		{
			// Calls GetHandJointCount one time after starting tracker.
			if (!hasElectronicTrackerInfo)
			{
				WVR_Result result = WVR()->GetHandJointCount(WVR_HandTrackerType::WVR_HandTrackerType_Electronic, &m_ElectronicTrackerInfo.jointCount);
				if (result == WVR_Result::WVR_Success)
				{
					LOGD(LogWaveVRHandPose, "TickHandData() Electronic tracker, get joint count %d", m_ElectronicTrackerInfo.jointCount);

					// Calls GetHandTrackerInfo one time after starting tracker.
					m_ElectronicTrackerInfo.handModelTypeBitMask = 0;
					if (m_ElectronicTrackerInfo.jointMappingArray != nullptr)
					{
						memset(m_ElectronicTrackerInfo.jointMappingArray, 0, sizeof(WVR_HandJoint)* m_ElectronicTrackerInfo.jointCount);
						delete[] m_ElectronicTrackerInfo.jointMappingArray;
						m_ElectronicTrackerInfo.jointMappingArray = nullptr;

						LOGD(LogWaveVRHandPose, "TickHandData() Release m_ElectronicTrackerInfo.jointMappingArray.");
					}
					m_ElectronicTrackerInfo.jointMappingArray = new WVR_HandJoint[m_ElectronicTrackerInfo.jointCount];
					memset(m_ElectronicTrackerInfo.jointMappingArray, 0, sizeof(WVR_HandJoint)* m_ElectronicTrackerInfo.jointCount);

					if (m_ElectronicTrackerInfo.jointValidFlagArray != nullptr)
					{
						memset(m_ElectronicTrackerInfo.jointValidFlagArray, 0, sizeof(uint64_t)* m_ElectronicTrackerInfo.jointCount);
						delete[] m_ElectronicTrackerInfo.jointValidFlagArray;
						m_ElectronicTrackerInfo.jointValidFlagArray = nullptr;

						LOGD(LogWaveVRHandPose, "TickHandData() Release m_ElectronicTrackerInfo.jointValidFlagArray.");
					}
					m_ElectronicTrackerInfo.jointValidFlagArray = new uint64_t[m_ElectronicTrackerInfo.jointCount];
					memset(m_ElectronicTrackerInfo.jointValidFlagArray, 0, sizeof(uint64_t)* m_ElectronicTrackerInfo.jointCount);

					m_ElectronicTrackerInfo.pinchTHR = 0;

					hasElectronicTrackerInfo =
						(WVR()->GetHandTrackerInfo(WVR_HandTrackerType::WVR_HandTrackerType_Electronic, &m_ElectronicTrackerInfo) == WVR_Result::WVR_Success ? true : false);

					if (hasElectronicTrackerInfo)
					{
						LOGD(LogWaveVRHandPose, "TickHandData() Electronic tracker, joint %d, pinchTHR %f",
							m_ElectronicTrackerInfo.jointCount,
							m_ElectronicTrackerInfo.pinchTHR);

						for (int32_t i = 0; i < (int32_t)m_ElectronicTrackerInfo.jointCount; i++)
						{
							LOGD(LogWaveVRHandPose, "TickHandData() Electronic tracker, jointMappingArray[%d] = %d, jointValidFlagArray[%d] = %d",
								i, (uint8)m_ElectronicTrackerInfo.jointMappingArray[i],
								i, (uint8)m_ElectronicTrackerInfo.jointValidFlagArray[i]);
						}

						/// Initialize m_ElectronicHandTrackerData
						m_ElectronicHandTrackerData.timestamp = 0;
						m_ElectronicHandTrackerData.left.confidence = 0;
						m_ElectronicHandTrackerData.left.isValidPose = false;
						m_ElectronicHandTrackerData.left.jointCount = m_ElectronicTrackerInfo.jointCount;
						if (m_ElectronicHandTrackerData.left.joints != nullptr)
						{
							memset(m_ElectronicHandTrackerData.left.joints, 0, sizeof(WVR_Pose_t)* m_ElectronicTrackerInfo.jointCount);
							delete[] m_ElectronicHandTrackerData.left.joints;
							m_ElectronicHandTrackerData.left.joints = nullptr;

							LOGD(LogWaveVRHandPose, "TickHandData() Release m_ElectronicHandTrackerData.left.joints.");
						}
						m_ElectronicHandTrackerData.left.joints = new WVR_Pose_t[m_ElectronicTrackerInfo.jointCount];
						memset(m_ElectronicHandTrackerData.left.joints, 0, sizeof(WVR_Pose_t)* m_ElectronicTrackerInfo.jointCount);
						m_ElectronicHandTrackerData.left.scale.v[0] = 0;
						m_ElectronicHandTrackerData.left.scale.v[1] = 0;
						m_ElectronicHandTrackerData.left.scale.v[2] = 0;

						m_ElectronicHandTrackerData.right.confidence = 0;
						m_ElectronicHandTrackerData.right.isValidPose = false;
						m_ElectronicHandTrackerData.right.jointCount = m_ElectronicTrackerInfo.jointCount;
						if (m_ElectronicHandTrackerData.right.joints != nullptr)
						{
							memset(m_ElectronicHandTrackerData.right.joints, 0, sizeof(WVR_Pose_t)* m_ElectronicTrackerInfo.jointCount);
							delete[] m_ElectronicHandTrackerData.right.joints;
							m_ElectronicHandTrackerData.right.joints = nullptr;

							LOGD(LogWaveVRHandPose, "TickHandData() Release m_ElectronicHandTrackerData.right.joints.");
						}
						m_ElectronicHandTrackerData.right.joints = new WVR_Pose_t[m_ElectronicTrackerInfo.jointCount];
						memset(m_ElectronicHandTrackerData.right.joints, 0, sizeof(WVR_Pose_t)* m_ElectronicTrackerInfo.jointCount);
						m_ElectronicHandTrackerData.right.scale.v[0] = 0;
						m_ElectronicHandTrackerData.right.scale.v[1] = 0;
						m_ElectronicHandTrackerData.right.scale.v[2] = 0;

						/// Initialize m_ElectronicHandPoseData
						m_ElectronicHandPoseData.timestamp = 0;
						m_ElectronicHandPoseData.left.base.type = WVR_HandPoseType::WVR_HandPoseType_Invalid;
						m_ElectronicHandPoseData.right.base.type = WVR_HandPoseType::WVR_HandPoseType_Invalid;
					}
				}
			} // if (!hasElectronicTrackerInfo)

			// Calls GetHandTrackingData on each frame.
			if (hasElectronicTrackerInfo)
			{
				IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
				EHMDTrackingOrigin::Type dofType = XRSystem->GetTrackingOrigin();
				WVR_PoseOriginModel originModel =
					(dofType == EHMDTrackingOrigin::Type::Floor ?
						WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnGround : WVR_PoseOriginModel::WVR_PoseOriginModel_OriginOnHead);

				WVR_HandModelType handModel = WVR_HandModelType::WVR_HandModelType_WithoutController;
				if ((m_ElectronicTrackerInfo.handModelTypeBitMask & (uint64_t)handModel) == 0)
					handModel = WVR_HandModelType::WVR_HandModelType_WithController;

				hasElectronicTrackerData = (
					WVR()->GetHandTrackingData(
						WVR_HandTrackerType::WVR_HandTrackerType_Electronic,
						handModel,
						originModel,
						&m_ElectronicHandTrackerData,
						&m_ElectronicHandPoseData
					) == WVR_Result::WVR_Success ? true : false);

				if (hasElectronicTrackerData)
				{
					{
						auto wvrScaleL = m_ElectronicHandTrackerData.left.scale;
						auto wvrScaleR = m_ElectronicHandTrackerData.right.scale;
						ElectronicHandScaleL = FVector(wvrScaleL.v[2], wvrScaleL.v[0], wvrScaleL.v[1]);
						ElectronicHandScaleR = FVector(wvrScaleR.v[2], wvrScaleR.v[0], wvrScaleR.v[1]);

						ElectronicWristLinearVelocityL = CoordinateUtil::GetVector3(m_ElectronicHandTrackerData.left.wristLinearVelocity, GetWorldToMetersScale());
						ElectronicWristLinearVelocityR = CoordinateUtil::GetVector3(m_ElectronicHandTrackerData.right.wristLinearVelocity, GetWorldToMetersScale());
						ElectronicWristAngularVelocityL = CoordinateUtil::GetVector3(m_ElectronicHandTrackerData.left.wristAngularVelocity, GetWorldToMetersScale());
						ElectronicWristAngularVelocityR = CoordinateUtil::GetVector3(m_ElectronicHandTrackerData.right.wristAngularVelocity, GetWorldToMetersScale());
					}

					for (uint32_t i = 0; i < m_ElectronicTrackerInfo.jointCount; i++)
					{
						if ((m_ElectronicTrackerInfo.jointValidFlagArray[i] & (uint64_t)WVR_HandJointValidFlag::WVR_HandJointValidFlag_PositionValid) != 0)
						{
							s_ElectronicJointPositionLeft[(uint8)m_ElectronicTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetVector3(m_ElectronicHandTrackerData.left.joints[i].position, GetWorldToMetersScale());
							s_ElectronicJointPositionRight[(uint8)m_ElectronicTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetVector3(m_ElectronicHandTrackerData.right.joints[i].position, GetWorldToMetersScale());
						}
					}
					for (uint32_t i = 0; i < m_ElectronicTrackerInfo.jointCount; i++)
					{
						if ((m_ElectronicTrackerInfo.jointValidFlagArray[i] & (uint64_t)WVR_HandJointValidFlag::WVR_HandJointValidFlag_RotationValid) != 0)
						{
							s_ElectronicJointRotationLeft[(uint8)m_ElectronicTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetQuaternion(m_ElectronicHandTrackerData.left.joints[i].rotation);
							s_ElectronicJointRotationRight[(uint8)m_ElectronicTrackerInfo.jointMappingArray[i]] =
								CoordinateUtil::GetQuaternion(m_ElectronicHandTrackerData.right.joints[i].rotation);
						}
					}
				}
			}
			else
			{
				hasElectronicTrackerData = false;
			}
		}
	}
	else if (electronic_tracker_status == EWaveVRHandTrackingStatus::NOT_START || electronic_tracker_status == EWaveVRHandTrackingStatus::START_FAILURE)
	{
		m_ElectronicTrackerStopTick = 0;

		if (m_EnableElectronicTracker
			//&& (UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_ElectronicHand_Right) ||
				//UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_ElectronicHand_Left))
			)
		{
			if (m_ElectronicTrackerStartTick == 0)
			{
				LOGD(LogWaveVRHandPose, "TickHandData() Start Electronic Hand Tracker.");
				m_HandThread->StartHandTracking(EWaveVRTrackerType::Electronic);
			}
			m_ElectronicTrackerStartTick++;
			m_ElectronicTrackerStartTick %= 60;
		}
	}
}
void WaveVRHandPose::CheckPoseFusion()
{
	m_WearingTracker = WVR()->IsWearingTracker();
	LOGD(LogWaveVRHandPose, "CheckPoseFusion() m_WearingTracker %d", (uint8)m_WearingTracker);
}
#pragma endregion Hand Data Update
#pragma region
void WaveVRHandPose::StartHandGesture()
{
	LOGD(LogWaveVRHandPose, "StartHandGesture()");
	m_EnableHandGesture = true;
}
void WaveVRHandPose::StopHandGesture()
{
	LOGD(LogWaveVRHandPose, "StopHandGesture()");
	m_EnableHandGesture = false;
}
bool WaveVRHandPose::IsHandGestureAvailable()
{
	if (m_HandThread != NULL)
		return m_HandThread->IsHandGestureAvailable();

	return false;
}
EWaveVRGestureType WaveVRHandPose::GetStaticGestureType(EWaveVRHandType DevType)
{
	if (!bHandGestureDataUpdated)
		return EWaveVRGestureType::Invalid;

	if (DevType == EWaveVRHandType::Left)
		return currStaticGestureLeft;
	if (DevType == EWaveVRHandType::Right)
		return currStaticGestureRight;

	return EWaveVRGestureType::Invalid;
}
EWaveVRHandGestureStatus WaveVRHandPose::GetHandGestureStatus()
{
	if (m_HandThread != NULL)
		return m_HandThread->GetHandGestureStatus();

	return EWaveVRHandGestureStatus::UNSUPPORT;
}
#pragma endregion Hand Gesture
#pragma region
void WaveVRHandPose::StartHandTracking(EWaveVRTrackerType tracker)
{
	LOGD(LogWaveVRHandPose, "StartHandTracking() %d", (uint8)tracker);
	if (tracker == EWaveVRTrackerType::Natural)
		m_EnableNaturalTracker = true;
	if (tracker == EWaveVRTrackerType::Electronic)
		m_EnableElectronicTracker = true;
}
void WaveVRHandPose::StopHandTracking(EWaveVRTrackerType tracker)
{
	LOGD(LogWaveVRHandPose, "StopHandTracking() %d", (uint8)tracker);
	if (tracker == EWaveVRTrackerType::Natural)
		m_EnableNaturalTracker = false;
	if (tracker == EWaveVRTrackerType::Electronic)
		m_EnableElectronicTracker = false;
}
bool WaveVRHandPose::IsHandTrackingAvailable(EWaveVRTrackerType tracker)
{
	if (m_HandThread != NULL)
		return m_HandThread->IsHandTrackingAvailable(tracker);

	return false;
}
EWaveVRHandTrackingStatus WaveVRHandPose::GetHandTrackingStatus(EWaveVRTrackerType tracker)
{
	if (m_HandThread != NULL)
		return m_HandThread->GetHandTrackingStatus(tracker);

	return EWaveVRHandTrackingStatus::UNSUPPORT;
}
float WaveVRHandPose::GetHandConfidence(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hasNaturalTrackerData && hasNaturalTrackerInfo)
		{
			if (hand == EWaveVRHandType::Left)
				return m_NaturalHandTrackerData.left.confidence;
			else
				return m_NaturalHandTrackerData.right.confidence;
		}
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hasElectronicTrackerInfo && hasElectronicTrackerData)
		{
			if (hand == EWaveVRHandType::Left)
				return m_ElectronicHandTrackerData.left.confidence;
			else
				return m_ElectronicHandTrackerData.right.confidence;
		}
	}

	return 0;
}
bool WaveVRHandPose::IsHandPoseValid(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	bool valid = false;
	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hasNaturalTrackerInfo && hasNaturalTrackerData)
		{
			if (hand == EWaveVRHandType::Left)
				valid = m_NaturalHandTrackerData.left.isValidPose;
			else
				valid = m_NaturalHandTrackerData.right.isValidPose;
		}
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hasElectronicTrackerInfo && hasElectronicTrackerData)
		{
			if (hand == EWaveVRHandType::Left)
				valid = m_ElectronicHandTrackerData.left.isValidPose;
			else
				valid = m_ElectronicHandTrackerData.right.isValidPose;
		}
	}
	return valid;
}
bool WaveVRHandPose::GetHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FRotator& OutRotation)
{
	if (!IsHandPoseValid(tracker, hand))
		return false;

	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hand == EWaveVRHandType::Left)
		{
			OutPosition = s_NaturalJointPositionLeft[(uint8)joint] + BONE_OFFSET_L;
			OutRotation = s_NaturalJointRotationLeft[(uint8)joint].Rotator();
		}
		if (hand == EWaveVRHandType::Right)
		{
			OutPosition = s_NaturalJointPositionRight[(uint8)joint] + BONE_OFFSET_R;
			OutRotation = s_NaturalJointRotationRight[(uint8)joint].Rotator();
		}
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hand == EWaveVRHandType::Left)
		{
			OutPosition = s_ElectronicJointPositionLeft[(uint8)joint] + BONE_OFFSET_L;
			OutRotation = s_ElectronicJointRotationLeft[(uint8)joint].Rotator();
		}
		if (hand == EWaveVRHandType::Right)
		{
			OutPosition = s_ElectronicJointPositionRight[(uint8)joint] + BONE_OFFSET_R;
			OutRotation = s_ElectronicJointRotationRight[(uint8)joint].Rotator();
		}
	}

	return true;
}
bool WaveVRHandPose::GetHandJointPose(EWaveVRTrackerType tracker, EWaveVRHandType hand, EWaveVRHandJoint joint, FVector& OutPosition, FQuat& OutRotation)
{
	if (!IsHandPoseValid(tracker, hand))
		return false;

	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hand == EWaveVRHandType::Left)
		{
			OutPosition = s_NaturalJointPositionLeft[(uint8)joint] + BONE_OFFSET_L;
			OutRotation = s_NaturalJointRotationLeft[(uint8)joint];
		}
		if (hand == EWaveVRHandType::Right)
		{
			OutPosition = s_NaturalJointPositionRight[(uint8)joint] + BONE_OFFSET_R;
			OutRotation = s_NaturalJointRotationRight[(uint8)joint];
		}
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hand == EWaveVRHandType::Left)
		{
			OutPosition = s_ElectronicJointPositionLeft[(uint8)joint] + BONE_OFFSET_L;
			OutRotation = s_ElectronicJointRotationLeft[(uint8)joint];
		}
		if (hand == EWaveVRHandType::Right)
		{
			OutPosition = s_ElectronicJointPositionRight[(uint8)joint] + BONE_OFFSET_R;
			OutRotation = s_ElectronicJointRotationRight[(uint8)joint];
		}
	}

	return true;
}
bool WaveVRHandPose::GetAllHandJointPoses(EWaveVRTrackerType tracker, EWaveVRHandType hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations)
{
	if (!IsHandPoseValid(tracker, hand))
		return false;

	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hand == EWaveVRHandType::Left)
		{
			OutPositions = s_NaturalJointPositionLeft;
			OutRotations = s_NaturalJointRotationLeft;
		}
		if (hand == EWaveVRHandType::Right)
		{
			OutPositions = s_NaturalJointPositionRight;
			OutRotations = s_NaturalJointRotationRight;
		}
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hand == EWaveVRHandType::Left)
		{
			OutPositions = s_ElectronicJointPositionLeft;
			OutRotations = s_ElectronicJointRotationLeft;
		}
		if (hand == EWaveVRHandType::Right)
		{
			OutPositions = s_ElectronicJointPositionRight;
			OutRotations = s_ElectronicJointRotationRight;
		}
	}

	return true;
}
EWaveVRHandMotion WaveVRHandPose::GetHandMotion(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hasNaturalTrackerInfo && hasNaturalTrackerData)
		{
			// Note: The counts of WVR_HandPoseType and EWaveVRHandMotion are equivalent so casting directly.
			return (hand == EWaveVRHandType::Left ?
				static_cast<EWaveVRHandMotion>(m_NaturalHandPoseData.left.base.type) :
				static_cast<EWaveVRHandMotion>(m_NaturalHandPoseData.right.base.type));
		}
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hasElectronicTrackerInfo && hasElectronicTrackerData)
		{
			// Note: The counts of WVR_HandPoseType and EWaveVRHandMotion are equivalent so casting directly.
			return (hand == EWaveVRHandType::Left ?
				static_cast<EWaveVRHandMotion>(m_ElectronicHandPoseData.left.base.type) :
				static_cast<EWaveVRHandMotion>(m_ElectronicHandPoseData.right.base.type));
		}
	}

	return EWaveVRHandMotion::Invalid;
}
float WaveVRHandPose::GetHandPinchThreshold(EWaveVRTrackerType tracker)
{
	if (tracker == EWaveVRTrackerType::Natural)
		return m_NaturalTrackerInfo.pinchTHR;
	if (tracker == EWaveVRTrackerType::Electronic)
		return m_ElectronicTrackerInfo.pinchTHR;

	return 0;
}
float WaveVRHandPose::GetHandPinchStrength(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (GetHandMotion(tracker, hand) == EWaveVRHandMotion::Pinch)
	{
		if (tracker == EWaveVRTrackerType::Natural)
		{
			if (hand == EWaveVRHandType::Left)
				return m_NaturalHandPoseData.left.pinch.strength;
			else
				return m_NaturalHandPoseData.right.pinch.strength;
		}
		if (tracker == EWaveVRTrackerType::Electronic)
		{
			if (hand == EWaveVRHandType::Left)
				return m_ElectronicHandPoseData.left.pinch.strength;
			else
				return m_ElectronicHandPoseData.right.pinch.strength;
		}
	}
	return 0;
}
FVector WaveVRHandPose::GetHandPinchOrigin(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (GetHandMotion(tracker, hand) == EWaveVRHandMotion::Pinch)
	{
		if (tracker == EWaveVRTrackerType::Natural)
		{
			if (hand == EWaveVRHandType::Left)
				return (CoordinateUtil::GetVector3(m_NaturalHandPoseData.left.pinch.origin, GetWorldToMetersScale()) + BONE_OFFSET_L);
			else
				return (CoordinateUtil::GetVector3(m_NaturalHandPoseData.right.pinch.origin, GetWorldToMetersScale()) + BONE_OFFSET_R);
		}
		if (tracker == EWaveVRTrackerType::Electronic)
		{
			if (hand == EWaveVRHandType::Left)
				return (CoordinateUtil::GetVector3(m_ElectronicHandPoseData.left.pinch.origin, GetWorldToMetersScale()) + BONE_OFFSET_L);
			else
				return (CoordinateUtil::GetVector3(m_ElectronicHandPoseData.right.pinch.origin, GetWorldToMetersScale()) + BONE_OFFSET_R);
		}
	}

	return FVector::ZeroVector;
}
FVector WaveVRHandPose::GetHandPinchDirection(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (GetHandMotion(tracker, hand) == EWaveVRHandMotion::Pinch)
	{
		if (tracker == EWaveVRTrackerType::Natural)
		{
			if (hand == EWaveVRHandType::Left)
				return CoordinateUtil::GetVector3(m_NaturalHandPoseData.left.pinch.direction, GetWorldToMetersScale());
			else
				return CoordinateUtil::GetVector3(m_NaturalHandPoseData.right.pinch.direction, GetWorldToMetersScale());
		}
		if (tracker == EWaveVRTrackerType::Electronic)
		{
			if (hand == EWaveVRHandType::Left)
				return CoordinateUtil::GetVector3(m_ElectronicHandPoseData.left.pinch.direction, GetWorldToMetersScale());
			else
				return CoordinateUtil::GetVector3(m_ElectronicHandPoseData.right.pinch.direction, GetWorldToMetersScale());
		}
	}

	return FVector::ZeroVector;
}
EWaveVRHandHoldRoleType WaveVRHandPose::GetHandHoldRole(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (GetHandMotion(tracker, hand) == EWaveVRHandMotion::Hold)
	{
		if (tracker == EWaveVRTrackerType::Natural)
		{
			return (hand == EWaveVRHandType::Left ?
				static_cast<EWaveVRHandHoldRoleType>(m_NaturalHandPoseData.left.hold.role) :
				static_cast<EWaveVRHandHoldRoleType>(m_NaturalHandPoseData.right.hold.role));
		}
		if (tracker == EWaveVRTrackerType::Electronic)
		{
			return (hand == EWaveVRHandType::Left ?
				static_cast<EWaveVRHandHoldRoleType>(m_ElectronicHandPoseData.left.hold.role) :
				static_cast<EWaveVRHandHoldRoleType>(m_ElectronicHandPoseData.right.hold.role));
		}
	}
	return EWaveVRHandHoldRoleType::None;
}
EWaveVRHandHoldObjectType WaveVRHandPose::GetHandHoldType(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (GetHandMotion(tracker, hand) == EWaveVRHandMotion::Hold)
	{
		if (tracker == EWaveVRTrackerType::Natural)
		{
			return (hand == EWaveVRHandType::Left ?
				static_cast<EWaveVRHandHoldObjectType>(m_NaturalHandPoseData.left.hold.object) :
				static_cast<EWaveVRHandHoldObjectType>(m_NaturalHandPoseData.right.hold.object));
		}
		if (tracker == EWaveVRTrackerType::Electronic)
		{
			return (hand == EWaveVRHandType::Left ?
				static_cast<EWaveVRHandHoldObjectType>(m_ElectronicHandPoseData.left.hold.object) :
				static_cast<EWaveVRHandHoldObjectType>(m_ElectronicHandPoseData.right.hold.object));
		}
	}
	return EWaveVRHandHoldObjectType::None;
}
FVector WaveVRHandPose::GetHandScale(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hand == EWaveVRHandType::Left)
			return NaturalHandScaleL;
		if (hand == EWaveVRHandType::Right)
			return NaturalHandScaleR;
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hand == EWaveVRHandType::Left)
			return ElectronicHandScaleL;
		if (hand == EWaveVRHandType::Right)
			return ElectronicHandScaleR;
	}

	return FVector::OneVector;
}
FVector WaveVRHandPose::GetWristLinearVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hand == EWaveVRHandType::Left)
			return NaturalWristLinearVelocityL;
		if (hand == EWaveVRHandType::Right)
			return NaturalWristLinearVelocityR;
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hand == EWaveVRHandType::Left)
			return ElectronicWristLinearVelocityL;
		if (hand == EWaveVRHandType::Right)
			return ElectronicWristLinearVelocityR;
	}

	return FVector::OneVector;
}
FVector WaveVRHandPose::GetWristAngularVelocity(EWaveVRTrackerType tracker, EWaveVRHandType hand)
{
	if (tracker == EWaveVRTrackerType::Natural)
	{
		if (hand == EWaveVRHandType::Left)
			return NaturalWristAngularVelocityL;
		if (hand == EWaveVRHandType::Right)
			return NaturalWristAngularVelocityR;
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		if (hand == EWaveVRHandType::Left)
			return ElectronicWristAngularVelocityL;
		if (hand == EWaveVRHandType::Right)
			return ElectronicWristAngularVelocityR;
	}

	return FVector::OneVector;
}
void WaveVRHandPose::FuseWristPositionWithTracker(bool fuse)
{
	LOGD(LogWaveVRHandPose, "FuseWristPositionWithTracker() %d", (uint8)fuse);
	WVR()->WearWristTracker(fuse);
}
void WaveVRHandPose::ActivateHoldMotion(bool active)
{
	LOGD(LogWaveVRHandPose, "ActivateHoldMotion() %d", (uint8)active);
	WVR()->SetParameters(WVR_DeviceType::WVR_DeviceType_HMD, (active ? kHoldGunOn : kHoldGunOff));
}
void WaveVRHandPose::ActivateGunMode(bool active)
{
	LOGD(LogWaveVRHandPose, "ActivateGunMode() %d", (uint8)active);
	WVR()->SetParameters(WVR_DeviceType::WVR_DeviceType_HMD, (active ? kGunModeOn : kGunModeOff));
}
#pragma endregion Hand Tracking

#pragma region
float WaveVRHandPose::GetWorldToMetersScale()
{
	if (IsInGameThread() && GWorld != nullptr)
	{
		// For example, One world unit need multiply 100 to become 1 meter.
		float wtm = GWorld->GetWorldSettings()->WorldToMeters;
		//LOGI(LogWaveVRHandPose, "GWorld->GetWorldSettings()->WorldToMeters = %f", wtm);
		return wtm;
	}
	return 100.0f;
}
#pragma endregion Major Standalone Function
