// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRSettings.h"

#include "HAL/IConsoleManager.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "UObject/PropertyPortFlags.h"

DEFINE_LOG_CATEGORY_STATIC(WVRSettings, Display, All);

// No, you just can't find its declaration in header.
UWaveVRSettings::UWaveVRSettings(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	//bTripleDepthBuffer(false),
	AMCMode(EAMCModeS::Off),
	bEyeTrackingEnable(false),
	bRenderMaskEnable(true),
	FoveatedRenderingMode(EFoveatedRenderingMode::Default),
	FoveatedRenderingPeripheralFOV(38),
	FoveatedRenderingPeripheralQuality(EFoveatedRenderingPeripheralQuality::High),
	TimeWarpStabilizedMode(ETimeWarpStabilizedMode::Auto),
	//SupportedFPS(ESupportedFPS::HMD_Default),
	bFadeOutEnable(false),
	bFrameSharpnessEnhancement(false),
	//colorGamutPreference0(EColorGamut::Native),
	//colorGamutPreference1(EColorGamut::sRGB),
	//colorGamutPreference2(EColorGamut::DisplayP3),
	//colorGamutPreference3(EColorGamut::Native),

	bRenderDocEnable(false),
	DirectPreviewConnectType(EDirectPreviewConnectType::WiFi),
	sDirectPreviewDeviceWiFiIP(""),
	DirectPreviewTargetDevice(EDirectPreviewdDevice::FOCUS_3),
	bDirectPreviewEnablePreviewImage(true),
	DirectPreviewUpdateFrequency(EDirectPreviewUpdateFrequency::FPS_60),
	bDirectPreviewEnableRegularlySaveImages(false),
	bAdaptiveQuality(true),
	AQMode(EAQMode::Quality_Oriented),
	bAdaptiveQualitySendQualityEvent(true),
	bAdaptiveQualityAutoFoveation(true),
	bHandTrackingEnable(false),
	bTrackerEnable(false),
	bSimultaneousInteractionEnable(false),
	bLipExpressionEnabled(false)
	//bPMCSupported(false)
{
}

void UWaveVRSettings::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR

static FName DeveloperSettingsConsoleVariableMetaFName(TEXT("ConsoleVariable"));

static void ImportConsoleVariableValues(UWaveVRSettings * settings)
{
	for (FProperty* Property = settings->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (!Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		const FString& CVarName = Property->GetMetaData(DeveloperSettingsConsoleVariableMetaFName);
		if (!CVarName.IsEmpty())
		{
			IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
			if (CVar)
			{
				if (Property->ImportText(*CVar->GetString(), Property->ContainerPtrToValuePtr<uint8>(settings, 0), PPF_ConsoleVariable, settings) == NULL)
				{
					UE_LOG(WVRSettings, Error, TEXT("%s import failed for %s on console variable %s (=%s)"), *settings->GetClass()->GetName(), *Property->GetName(), *CVarName, *CVar->GetString());
				}
			}
			else
			{
				UE_LOG(WVRSettings, Fatal, TEXT("%s failed to find console variable %s for %s"), *settings->GetClass()->GetName(), *CVarName, *Property->GetName());
			}
		}
	}
}

static void ExportValuesToConsoleVariables(UWaveVRSettings * settings, FProperty* PropertyThatChanged)
{
	if (PropertyThatChanged)
	{
		const FString& CVarName = PropertyThatChanged->GetMetaData(DeveloperSettingsConsoleVariableMetaFName);
		if (!CVarName.IsEmpty())
		{
			IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
			if (CVar && (CVar->GetFlags() & ECVF_ReadOnly) == 0)
			{
				FByteProperty* ByteProperty = CastField<FByteProperty>(PropertyThatChanged);
				if (ByteProperty != NULL && ByteProperty->Enum != NULL)
				{
					CVar->Set(ByteProperty->GetPropertyValue_InContainer(settings), ECVF_SetByProjectSetting);
				}
				else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(PropertyThatChanged))
				{
					FNumericProperty* UnderlyingProp = EnumProperty->GetUnderlyingProperty();
					void* PropertyAddress = EnumProperty->ContainerPtrToValuePtr<void>(settings);
					CVar->Set((int32)UnderlyingProp->GetSignedIntPropertyValue(PropertyAddress), ECVF_SetByProjectSetting);
				}
				else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(PropertyThatChanged))
				{
					CVar->Set((int32)BoolProperty->GetPropertyValue_InContainer(settings), ECVF_SetByProjectSetting);
				}
				else if (FIntProperty* IntProperty = CastField<FIntProperty>(PropertyThatChanged))
				{
					CVar->Set(IntProperty->GetPropertyValue_InContainer(settings), ECVF_SetByProjectSetting);
				}
				else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(PropertyThatChanged))
				{
					CVar->Set(FloatProperty->GetPropertyValue_InContainer(settings), ECVF_SetByProjectSetting);
				}
				else if (FStrProperty* StringProperty = CastField<FStrProperty>(PropertyThatChanged))
				{
					CVar->Set(*StringProperty->GetPropertyValue_InContainer(settings), ECVF_SetByProjectSetting);
				}
				else if (FNameProperty* NameProperty = CastField<FNameProperty>(PropertyThatChanged))
				{
					CVar->Set(*NameProperty->GetPropertyValue_InContainer(settings).ToString(), ECVF_SetByProjectSetting);
				}

			}
			else
			{
				UE_LOG(WVRSettings, Warning, TEXT("CVar named '%s' marked up in %s was not found or is set to read-only"), *CVarName, *settings->GetClass()->GetName());
			}
		}
	}
}

void UWaveVRSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Keep bPMCSupported to disable the TripleDepthBuffer
	//bPMCSupported = false;
	//if (!bTripleDepthBuffer) {
		// TODO Force_PMC is unavailable in enum class now.  Waiting for PMC complete
		//if (AMCMode == EAMCModeS::Force_PMC) {
		//	AMCMode = EAMCModeS::Off;
		//}
	//}

	// Gray out and auto select/deselect effect.
	if (bAdaptiveQuality) {
		if (AQMode == EAQMode::Quality_Oriented || AQMode == EAQMode::Performance_Oriented) {
			bAdaptiveQualitySendQualityEvent = true;
			bAdaptiveQualityAutoFoveation = true;
		}
	}
	// Should update config to CVar system manually. Or the CVar value will mismatch with DefaultEngine.ini.
	auto* CVarAQSendQualityEvent = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.AdaptiveQuality.sendQualityEvent"));
	if (CVarAQSendQualityEvent) {
		CVarAQSendQualityEvent->Set(bAdaptiveQualitySendQualityEvent, ECVF_SetByProjectSetting);
	}
	auto* CVarAQAutoFoveation = IConsoleManager::Get().FindConsoleVariable(TEXT("wvr.AdaptiveQuality.autoFoveation"));
	if (CVarAQAutoFoveation) {
		CVarAQAutoFoveation->Set(bAdaptiveQualityAutoFoveation, ECVF_SetByProjectSetting);
	}

	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(this, PropertyChangedEvent.Property);
	}

	//Update and write back to DefaultEngine.ini immediately.
	UpdateDefaultConfigFile();
}
#endif
