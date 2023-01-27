// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "LipExpression/WaveVRLipExpImpl.h"

#include "GameFramework/WorldSettings.h"
#include "HeadMountedDisplayTypes.h"
#include "XRTrackingSystemBase.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRLipExpImpl, Log, All);

WaveVRLipExpImpl * WaveVRLipExpImpl::Instance = nullptr;

WaveVRLipExpImpl::WaveVRLipExpImpl()
{
	Instance = this;
}

WaveVRLipExpImpl::~WaveVRLipExpImpl()
{
	Instance = nullptr;
}

void WaveVRLipExpImpl::InitLipExpData()
{
	s_LipExpData.Init(0, (uint8)EWaveVRLipExp::Max);

	m_LipExpThread = FWaveVRLipExpThread::JoyInit();

	LOGD(LogWaveVRLipExpImpl, "InitLipExpData()");
}
bool WaveVRLipExpImpl::LogInterval()
{
	if (logFrame != GFrameCounter)
	{
		logFrame = GFrameCounter;
		logCount++;
		logCount %= 1000;
	}
	return (logCount == 0);
}
void WaveVRLipExpImpl::TickLipExpData()
{
	LogInterval();

	if (GWorld && GWorld->GetWorld()->WorldType == EWorldType::Type::Editor)
		return;

	UpdateData();
	if (hasLipExpData)
	{
		for (uint8 i = 0; i < (uint8)EWaveVRLipExp::Max; i++)
		{
			s_LipExpData[i] = s_LipExpValues[i];
		}
		if (LogInterval())
		{
			for (uint8 i = 0; i < (uint8)EWaveVRLipExp::Max; i++)
				LOGD(LogWaveVRLipExpImpl, "TickLipExpData() Lip Expression %d = %f", i, s_LipExpData[i]);
		}
	}
}

#pragma region
bool WaveVRLipExpImpl::CanStartLipExp()
{
	if (m_LipExpThread == NULL) { return false; }

	EWaveVRLipExpStatus status = m_LipExpThread->GetLipExpStatus();
	if (status == EWaveVRLipExpStatus::NOT_START ||
		status == EWaveVRLipExpStatus::START_FAILURE)
	{
		return true;
	}
	return false;
}
bool WaveVRLipExpImpl::CanStopLipExp()
{
	if (m_LipExpThread == NULL) { return false; }

	EWaveVRLipExpStatus status = m_LipExpThread->GetLipExpStatus();
	if (status == EWaveVRLipExpStatus::AVAILABLE)
	{
		return true;
	}
	return false;
}
#pragma endregion Life cycle

void WaveVRLipExpImpl::UpdateData()
{
	EWaveVRLipExpStatus status = GetLipExpStatus();
	if (status == EWaveVRLipExpStatus::AVAILABLE)
	{
		WVR_Result result = FWaveVRAPIWrapper::GetInstance()->GetLipExpData(s_LipExpValues);
		hasLipExpData = (result == WVR_Result::WVR_Success);
	}
	else
	{
		hasLipExpData = false;
	}
}

#pragma region
void WaveVRLipExpImpl::StartLipExp()
{
	if (CanStartLipExp())
	{
		LOGD(LogWaveVRLipExpImpl, "StartLipExp()");
		m_LipExpThread->StartLipExp();
	}
}
void WaveVRLipExpImpl::StopLipExp()
{
	if (CanStopLipExp())
	{
		LOGD(LogWaveVRLipExpImpl, "StopLipExp()");
		m_LipExpThread->StopLipExp();
	}
}
EWaveVRLipExpStatus WaveVRLipExpImpl::GetLipExpStatus()
{
	if (m_LipExpThread)
	{
		return m_LipExpThread->GetLipExpStatus();
	}
	return EWaveVRLipExpStatus::NO_SUPPORT;
}
bool WaveVRLipExpImpl::IsLipExpAvailable()
{
	if (m_LipExpThread)
	{
		EWaveVRLipExpStatus status = m_LipExpThread->GetLipExpStatus();
		return (status == EWaveVRLipExpStatus::AVAILABLE);
	}

	return false;
}
#pragma endregion Public Interface

#pragma region
float WaveVRLipExpImpl::GetWorldToMetersScale()
{
	if (IsInGameThread() && GWorld != nullptr)
	{
		// For example, One world unit need multiply 100 to become 1 meter.
		float wtm = GWorld->GetWorldSettings()->WorldToMeters;
		//LOGI(LogWaveVREyeExpImpl, "GWorld->GetWorldSettings()->WorldToMeters = %f", wtm);
		return wtm;
	}
	return 100.0f;
}
#pragma endregion Major Standalone Function
