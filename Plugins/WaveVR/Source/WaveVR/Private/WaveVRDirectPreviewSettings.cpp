// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRDirectPreviewSettings.h"
#include "HAL/IConsoleManager.h"

#include "Platforms/WaveVRLogWrapper.h"


DEFINE_LOG_CATEGORY(LogWaveVRDirectPreviewSettings);

void WaveVRDirectPreviewSettings::SettingsInfo() {

	auto VarConnectType = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.DirectPreview.ConnectType"));
	int ConnectType = VarConnectType->GetInt();
	LOGD(LogWaveVRDirectPreviewSettings, "ConnectType: %d", ConnectType);

	auto VarDeviceWiFiIP = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.DirectPreview.DeviceWiFiIP"));
	FString DeviceWiFiIP = VarDeviceWiFiIP->GetString();
	LOGD(LogWaveVRDirectPreviewSettings, "DeviceWiFiIP: %s", PLATFORM_CHAR(*DeviceWiFiIP));

	auto VarEnablePreviewImage = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.DirectPreview.EnablePreviewImage"));
	bool EnablePreviewImage = VarEnablePreviewImage->GetBool();
	LOGD(LogWaveVRDirectPreviewSettings, "EnablePreviewImage: %s", EnablePreviewImage ? PLATFORM_CHAR(*FString("true")) : PLATFORM_CHAR(*FString("false")));

	auto VarUpdateFrequency = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.DirectPreview.UpdateFrequency"));
	int UpdateFrequency = VarUpdateFrequency->GetInt();
	LOGD(LogWaveVRDirectPreviewSettings, "UpdateFrequency: %d", UpdateFrequency);

	auto VarRegularlySaveImages = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.DirectPreview.RegularlySaveImages"));
	bool RegularlySaveImages = VarRegularlySaveImages->GetBool();
	LOGD(LogWaveVRDirectPreviewSettings, "RegularlySaveImages: %s", RegularlySaveImages ? PLATFORM_CHAR(*FString("true")) : PLATFORM_CHAR(*FString("false")));
}
