// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "LipExpression/FWaveVRLipExpThread.h"
#include "HAL/RunnableThread.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogFWaveVRLipExpThread, Log, All);

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FWaveVRLipExpThread* FWaveVRLipExpThread::Runnable = NULL;
//***********************************************************

FWaveVRLipExpThread::FWaveVRLipExpThread()
	: shutDownThread(false)
	, getSupportedFeature(false)
	, lipExpStatus(EWaveVRLipExpStatus::NOT_START)
{
	Thread = FRunnableThread::Create(this, TEXT("FWaveVRLipExpThread"));
}

FWaveVRLipExpThread::~FWaveVRLipExpThread()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FWaveVRLipExpThread::Init()
{
	LOGD(LogFWaveVRLipExpThread, "Init()");
	return true;
}

//Run
uint32 FWaveVRLipExpThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!shutDownThread)
	{
		if (!getSupportedFeature)
		{
			uint64_t supportedFeatures = FWaveVRAPIWrapper::GetInstance()->GetSupportedFeatures();
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_LipExp) == 0)
			{
				lipExpStatus = EWaveVRLipExpStatus::NO_SUPPORT;
			}
			else
			{
				lipExpStatus = EWaveVRLipExpStatus::NOT_START;
			}
			getSupportedFeature = true;

			LOGD(LogFWaveVRLipExpThread, "Run() supportedFeatures %d, lipExpStatus %d", (int)supportedFeatures, (int)lipExpStatus);
		}

		bool has_action = qActions.Dequeue(action);
		if (has_action)
		{
			m_mutex.Lock();
			switch (action)
			{
			case Actions::Activate:
				if (lipExpStatus == EWaveVRLipExpStatus::NOT_START || lipExpStatus == EWaveVRLipExpStatus::START_FAILURE)
				{
					lipExpStatus = EWaveVRLipExpStatus::STARTING;

					LOGD(LogFWaveVRLipExpThread, "Run() Start LipExp.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartLipExp();
					switch (result)
					{
					case WVR_Result::WVR_Success:
						lipExpStatus = EWaveVRLipExpStatus::AVAILABLE;
						break;
					case WVR_Result::WVR_Error_FeatureNotSupport:
						lipExpStatus = EWaveVRLipExpStatus::NO_SUPPORT;
						break;
					default:
						lipExpStatus = EWaveVRLipExpStatus::START_FAILURE;
						break;
					}
					LOGD(LogFWaveVRLipExpThread, "Run() Start LipExp result: %d", (uint8)result);
				}
				break;
			case Actions::Deactivate:
				if (lipExpStatus == EWaveVRLipExpStatus::AVAILABLE)
				{
					lipExpStatus = EWaveVRLipExpStatus::STOPING;
					LOGD(LogFWaveVRLipExpThread, "Run() Stop LipExp.");
					FWaveVRAPIWrapper::GetInstance()->StopLipExp();
					lipExpStatus = EWaveVRLipExpStatus::NOT_START;
				}
				break;
			default:
				break;
			}
			m_mutex.Unlock();
		}

		FPlatformProcess::Sleep(0.1);
	}

	return 0;
}

//stop
void FWaveVRLipExpThread::Stop()
{
}

FWaveVRLipExpThread* FWaveVRLipExpThread::JoyInit()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FWaveVRLipExpThread();
		LOGD(LogFWaveVRLipExpThread, "JoyInit() Create new thread.");
	}
	return Runnable;
}

void FWaveVRLipExpThread::EnsureCompletion()
{
	shutDownThread = true;
	Stop();
	Thread->WaitForCompletion();
}

void FWaveVRLipExpThread::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}
