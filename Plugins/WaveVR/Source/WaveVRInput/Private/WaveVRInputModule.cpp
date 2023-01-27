// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRInputModule.h"
#include "WaveVRInput.h"
#include "IWaveVRPlugin.h"

#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRInputSimulator.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRInputModule, Log, All);

//-------------------------------------------------------------------------------------------------
// FWaveVRInputModule
//-------------------------------------------------------------------------------------------------


void FWaveVRInputModule::StartupModule()
{
	IInputDeviceModule::StartupModule();
	LOGD(LogWaveVRInputModule, "StartupModule");
	FWaveVRInput::AddKeys();
}


TSharedPtr< class IInputDevice > FWaveVRInputModule::CreateInputDevice( const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler )
{
	WaveVRInputSimulator *pSimulator = new WaveVRInputSimulator();
	if (IWaveVRPlugin::IsAvailable())
	{
		return TSharedPtr< class IInputDevice >(new FWaveVRInput(pSimulator, InMessageHandler));
	}
	else
	{
		LOGD(LogWaveVRInputModule, "WaveVRInput module is enabled, but WaveVRHMD module is not available.");
	}
	return nullptr;
}
IMPLEMENT_MODULE( FWaveVRInputModule, WaveVRInput )
