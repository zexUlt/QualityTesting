// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "EyeExpression/WaveVREyeExpImpl.h"

#include "GameFramework/WorldSettings.h"
#include "HeadMountedDisplayTypes.h"
#include "XRTrackingSystemBase.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREyeExpImpl, Log, All);

WaveVREyeExpImpl* WaveVREyeExpImpl::Instance = nullptr;

WaveVREyeExpImpl::WaveVREyeExpImpl()
{
	Instance = this;
}

WaveVREyeExpImpl::~WaveVREyeExpImpl()
{
	Instance = nullptr;
}

void WaveVREyeExpImpl::InitEyeExpData()
{
	s_EyeExpData.Init(0, (uint8)EWaveVREyeExp::MAX);

	m_EyeExpThread = FWaveVREyeExpThread::JoyInit();

	LOGD(LogWaveVREyeExpImpl, "InitEyeExpData()");
}
bool WaveVREyeExpImpl::LogInterval()
{
	if (logFrame != GFrameCounter)
	{
		logFrame = GFrameCounter;
		logCount++;
		logCount %= 1000;
	}
	return (logCount == 0);
}
void WaveVREyeExpImpl::TickEyeExpData()
{
	LogInterval();

	if (GWorld && GWorld->GetWorld()->WorldType == EWorldType::Type::Editor)
		return;

	UpdateData();

	if (LogInterval())
	{
		LOGD(LogWaveVREyeExpImpl, "TickEyeExpData() hasEyeExpData %d", (uint8)hasEyeExpData);
		for (uint8 i = 0; i < (uint8)EWaveVREyeExp::MAX; i++)
			LOGD(LogWaveVREyeExpImpl, "TickEyeExpData() Eye Expression %d = %f", i, s_EyeExpData[i]);
	}
}

#pragma region
bool WaveVREyeExpImpl::CanStartEyeExp()
{
	EWaveVREyeExpStatus status = GetEyeExpStatus();
	if (status == EWaveVREyeExpStatus::NOT_START || status == EWaveVREyeExpStatus::START_FAILURE)
	{
		return true;
	}
	return false;
}
bool WaveVREyeExpImpl::CanStopEyeExp()
{
	EWaveVREyeExpStatus status = GetEyeExpStatus();
	if (status == EWaveVREyeExpStatus::AVAILABLE)
	{
		return true;
	}
	return false;
}
#pragma endregion Life cycle

void WaveVREyeExpImpl::UpdateData()
{
	EWaveVREyeExpStatus status = GetEyeExpStatus();
	if (status == EWaveVREyeExpStatus::AVAILABLE)
	{
		WVR_EyeExp_t eyeexp;
		WVR_Result result = FWaveVRAPIWrapper::GetInstance()->GetEyeExpData(&eyeexp);
		hasEyeExpData = (result == WVR_Result::WVR_Success);
		if (hasEyeExpData)
		{
			for (uint8 i = 0; i < (uint8)EWaveVREyeExp::MAX; i++)
				s_EyeExpData[i] = eyeexp.weights[i];
		}
	}
	else
	{
		hasEyeExpData = false;
	}
}

#pragma region
void WaveVREyeExpImpl::StartEyeExp()
{
	if (CanStartEyeExp())
	{
		LOGD(LogWaveVREyeExpImpl, "StartEyeExp()");
		m_EyeExpThread->StartEyeExp();
	}
}
void WaveVREyeExpImpl::StopEyeExp()
{
	if (CanStopEyeExp())
	{
		LOGD(LogWaveVREyeExpImpl, "StopEyeExp()");
		m_EyeExpThread->StopEyeExp();
	}
}
EWaveVREyeExpStatus WaveVREyeExpImpl::GetEyeExpStatus()
{
	if (m_EyeExpThread)
	{
		return m_EyeExpThread->GetEyeExpStatus();
	}
	return EWaveVREyeExpStatus::NO_SUPPORT;
}
bool WaveVREyeExpImpl::IsEyeExpAvailable()
{
	EWaveVREyeExpStatus status = GetEyeExpStatus();
	return (status == EWaveVREyeExpStatus::AVAILABLE);
}
#pragma endregion Public Interface

#pragma region
float WaveVREyeExpImpl::GetWorldToMetersScale()
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
