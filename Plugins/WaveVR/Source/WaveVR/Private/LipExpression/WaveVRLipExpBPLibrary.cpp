// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "LipExpression/WaveVRLipExpBPLibrary.h"

#include "Platforms/WaveVRLogWrapper.h"

void UWaveVRLipExpBPLibrary::StartLipExp()
{
	WaveVRLipExpImpl* pLipExp = WaveVRLipExpImpl::GetInstance();
	if (pLipExp == nullptr) { return; }

	LOGD(LogWaveVRLipExpBPLibrary, "StartLipExp()");
	pLipExp->StartLipExp();
}

void UWaveVRLipExpBPLibrary::StopLipExp()
{
	WaveVRLipExpImpl* pLipExp = WaveVRLipExpImpl::GetInstance();
	if (pLipExp == nullptr) { return; }

	LOGD(LogWaveVRLipExpBPLibrary, "StopLipExp()");
	pLipExp->StopLipExp();
}

EWaveVRLipExpStatus UWaveVRLipExpBPLibrary::GetLipExpStatus()
{
	WaveVRLipExpImpl* pLipExp = WaveVRLipExpImpl::GetInstance();
	if (pLipExp == nullptr) { return EWaveVRLipExpStatus::NO_SUPPORT; }

	return pLipExp->GetLipExpStatus();
}

bool UWaveVRLipExpBPLibrary::IsLipExpAvailable()
{
	WaveVRLipExpImpl* pLipExp = WaveVRLipExpImpl::GetInstance();
	if (pLipExp == nullptr) { return false; }

	return pLipExp->IsLipExpAvailable();
}

float UWaveVRLipExpBPLibrary::GetLipExpValue(EWaveVRLipExp lipExp)
{
	WaveVRLipExpImpl* pLipExp = WaveVRLipExpImpl::GetInstance();
	if (pLipExp == nullptr) { return false; }

	return pLipExp->GetLipExpValue(lipExp);
}


bool UWaveVRLipExpBPLibrary::GetLipExpData(TArray<float>& OutValue)
{
	WaveVRLipExpImpl* pLipExp = WaveVRLipExpImpl::GetInstance();
	if (pLipExp == nullptr) { return false; }

	return pLipExp->GetLipExpData(OutValue);
}
