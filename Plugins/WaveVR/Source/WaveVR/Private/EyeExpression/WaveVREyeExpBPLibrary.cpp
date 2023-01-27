// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "EyeExpression/WaveVREyeExpBPLibrary.h"

#include "Platforms/WaveVRLogWrapper.h"

void UWaveVREyeExpBPLibrary::StartEyeExp()
{
	WaveVREyeExpImpl* pEyeExp = WaveVREyeExpImpl::GetInstance();
	if (pEyeExp == nullptr) { return; }

	LOGD(LogWaveVREyeExpBPLibrary, "StartEyeExp()");
	pEyeExp->StartEyeExp();
}

void UWaveVREyeExpBPLibrary::StopEyeExp()
{
	WaveVREyeExpImpl* pEyeExp = WaveVREyeExpImpl::GetInstance();
	if (pEyeExp == nullptr) { return; }

	LOGD(LogWaveVREyeExpBPLibrary, "StopEyeExp()");
	pEyeExp->StopEyeExp();
}

EWaveVREyeExpStatus UWaveVREyeExpBPLibrary::GetEyeExpStatus()
{
	WaveVREyeExpImpl* pEyeExp = WaveVREyeExpImpl::GetInstance();
	if (pEyeExp == nullptr) { return EWaveVREyeExpStatus::NO_SUPPORT; }

	return pEyeExp->GetEyeExpStatus();
}

bool UWaveVREyeExpBPLibrary::IsEyeExpAvailable()
{
	WaveVREyeExpImpl* pEyeExp = WaveVREyeExpImpl::GetInstance();
	if (pEyeExp == nullptr) { return false; }

	return pEyeExp->IsEyeExpAvailable();
}

float UWaveVREyeExpBPLibrary::GetEyeExpValue(EWaveVREyeExp lipExp)
{
	WaveVREyeExpImpl* pEyeExp = WaveVREyeExpImpl::GetInstance();
	if (pEyeExp == nullptr) { return false; }

	return pEyeExp->GetEyeExpValue(lipExp);
}

bool UWaveVREyeExpBPLibrary::GetEyeExpData(TArray<float>& OutValue)
{
	WaveVREyeExpImpl* pEyeExp = WaveVREyeExpImpl::GetInstance();
	if (pEyeExp == nullptr) { return false; }

	return pEyeExp->GetEyeExpData(OutValue);
}
