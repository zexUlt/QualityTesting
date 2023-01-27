// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "EyeExpression/FWaveVREyeExpThread.h"
#include "HAL/RunnableThread.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogFWaveVREyeExpThread, Log, All);

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FWaveVREyeExpThread* FWaveVREyeExpThread::Runnable = NULL;
//***********************************************************

FWaveVREyeExpThread::FWaveVREyeExpThread()
	: shutDownThread(false)
	, getSupportedFeature(false)
	, eyeExpStatus(EWaveVREyeExpStatus::NOT_START)
{
	Thread = FRunnableThread::Create(this, TEXT("FWaveVREyeExpThread"));
}

FWaveVREyeExpThread::~FWaveVREyeExpThread()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FWaveVREyeExpThread::Init()
{
	LOGD(LogFWaveVREyeExpThread, "Init()");
	return true;
}

//Run
uint32 FWaveVREyeExpThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!shutDownThread)
	{
		if (!getSupportedFeature)
		{
			uint64_t supportedFeatures = FWaveVRAPIWrapper::GetInstance()->GetSupportedFeatures();
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_EyeExp) == 0)
			{
				eyeExpStatus = EWaveVREyeExpStatus::NO_SUPPORT;
			}
			else
			{
				eyeExpStatus = EWaveVREyeExpStatus::NOT_START;
			}
			getSupportedFeature = true;

			LOGD(LogFWaveVREyeExpThread, "Run() supportedFeatures %d, eyeExpStatus %d", (int)supportedFeatures, (int)eyeExpStatus);
		}

		bool has_action = qActions.Dequeue(action);
		if (has_action)
		{
			m_mutex.Lock();
			switch (action)
			{
			case Actions::Activate:
				if (eyeExpStatus == EWaveVREyeExpStatus::NOT_START || eyeExpStatus == EWaveVREyeExpStatus::START_FAILURE)
				{
					eyeExpStatus = EWaveVREyeExpStatus::STARTING;

					LOGD(LogFWaveVREyeExpThread, "Run() Start EyeExp.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartEyeExp();
					switch (result)
					{
					case WVR_Result::WVR_Success:
						eyeExpStatus = EWaveVREyeExpStatus::AVAILABLE;
						break;
					case WVR_Result::WVR_Error_FeatureNotSupport:
						eyeExpStatus = EWaveVREyeExpStatus::NO_SUPPORT;
						break;
					default:
						eyeExpStatus = EWaveVREyeExpStatus::START_FAILURE;
						break;
					}
					LOGD(LogFWaveVREyeExpThread, "Run() Start EyeExp result: %d", (uint8)result);
				}
				break;
			case Actions::Deactivate:
				if (eyeExpStatus == EWaveVREyeExpStatus::AVAILABLE)
				{
					eyeExpStatus = EWaveVREyeExpStatus::STOPING;
					LOGD(LogFWaveVREyeExpThread, "Run() Stop EyeExp.");
					FWaveVRAPIWrapper::GetInstance()->StopEyeExp();
					eyeExpStatus = EWaveVREyeExpStatus::NOT_START;
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
void FWaveVREyeExpThread::Stop()
{
}

FWaveVREyeExpThread* FWaveVREyeExpThread::JoyInit()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FWaveVREyeExpThread();
		LOGD(LogFWaveVREyeExpThread, "JoyInit() Create new thread.");
	}
	return Runnable;
}

void FWaveVREyeExpThread::EnsureCompletion()
{
	shutDownThread = true;
	Stop();
	Thread->WaitForCompletion();
}

void FWaveVREyeExpThread::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}
