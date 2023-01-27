// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Tracker/FWaveVRTrackerThread.h"
#include "HAL/RunnableThread.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogFWaveVRTrackerThread, Log, All);

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FWaveVRTrackerThread* FWaveVRTrackerThread::Runnable = NULL;
//***********************************************************

FWaveVRTrackerThread::FWaveVRTrackerThread()
	: shutDownThread(false)
	, getSupportedFeature(false)
	, trackerStatus(EWaveVRTrackerStatus::NOT_START)
{
	Thread = FRunnableThread::Create(this, TEXT("FWaveVRTrackerThread"));
}

FWaveVRTrackerThread::~FWaveVRTrackerThread()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FWaveVRTrackerThread::Init()
{
	LOGD(LogFWaveVRTrackerThread, "Init()");
	return true;
}

//Run
uint32 FWaveVRTrackerThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!shutDownThread)
	{
		if (!getSupportedFeature)
		{
			uint64_t supportedFeatures = FWaveVRAPIWrapper::GetInstance()->GetSupportedFeatures();
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_Tracker) == 0)
			{
				trackerStatus = EWaveVRTrackerStatus::UNSUPPORT;
			}
			else
			{
				trackerStatus = EWaveVRTrackerStatus::NOT_START;
			}
			getSupportedFeature = true;

			LOGD(LogFWaveVRTrackerThread, "Run() supportedFeatures %d, trackerStatus %d", (int)supportedFeatures, (int)trackerStatus);
		}

		bool has_action = qActions.Dequeue(action);
		if (has_action)
		{
			m_mutex.Lock();
			switch (action)
			{
			case Actions::Activate:
				if (trackerStatus == EWaveVRTrackerStatus::NOT_START || trackerStatus == EWaveVRTrackerStatus::START_FAILURE)
				{
					trackerStatus = EWaveVRTrackerStatus::STARTING;

					LOGD(LogFWaveVRTrackerThread, "Run() Start Tracker.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartTracker();
					switch (result)
					{
					case WVR_Result::WVR_Success:
						trackerStatus = EWaveVRTrackerStatus::AVAILABLE;
						break;
					case WVR_Result::WVR_Error_FeatureNotSupport:
						trackerStatus = EWaveVRTrackerStatus::UNSUPPORT;
						break;
					default:
						trackerStatus = EWaveVRTrackerStatus::START_FAILURE;
						break;
					}
					LOGD(LogFWaveVRTrackerThread, "Run() Start Tracker result: %d", (uint8)result);
				}
				break;
			case Actions::Deactivate:
				if (trackerStatus == EWaveVRTrackerStatus::AVAILABLE)
				{
					trackerStatus = EWaveVRTrackerStatus::STOPING;
					LOGD(LogFWaveVRTrackerThread, "Run() Stop Tracker.");
					FWaveVRAPIWrapper::GetInstance()->StopTracker();
					trackerStatus = EWaveVRTrackerStatus::NOT_START;
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
void FWaveVRTrackerThread::Stop()
{
}

FWaveVRTrackerThread* FWaveVRTrackerThread::JoyInit()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FWaveVRTrackerThread();
		LOGD(LogFWaveVRTrackerThread, "JoyInit() Create new thread.");
	}
	return Runnable;
}

void FWaveVRTrackerThread::EnsureCompletion()
{
	shutDownThread = true;
	Stop();
	Thread->WaitForCompletion();
}

void FWaveVRTrackerThread::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}
