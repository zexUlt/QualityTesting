// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Eye/FWaveVREyeRunnable.h"
#include "HAL/RunnableThread.h"

#include "wvr_eyetracking.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVREyeRunnable, Log, All);

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FWaveVREyeRunnable* FWaveVREyeRunnable::Runnable = NULL;
//***********************************************************

FWaveVREyeRunnable::FWaveVREyeRunnable()
	: shutDownThread(false)
	, getSupportedFeature(false)
	, eyeTrackingStatus(EWaveVREyeTrackingStatus::NOT_START)
{
	Thread = FRunnableThread::Create(this, TEXT("FWaveVREyeRunnable"));
}

FWaveVREyeRunnable::~FWaveVREyeRunnable()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FWaveVREyeRunnable::Init()
{
	return true;
}

//Run
uint32 FWaveVREyeRunnable::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!shutDownThread)
	{
		if (!getSupportedFeature)
		{
			uint64_t supportedFeatures = FWaveVRAPIWrapper::GetInstance()->GetSupportedFeatures();
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_EyeTracking) == 0)
			{
				eyeTrackingStatus = EWaveVREyeTrackingStatus::UNSUPPORT;
			}
			else
			{
				eyeTrackingStatus = EWaveVREyeTrackingStatus::NOT_START;
			}

			LOGD(LogWaveVREyeRunnable, "Run() supportedFeatures %d, eyeTrackingStatus %d", (int)supportedFeatures, (int)eyeTrackingStatus);
			getSupportedFeature = true;
		}

		bool has_action = qActions.Dequeue(action);
		if (has_action)
		{
			m_mutex.Lock();
			switch (action)
			{
			case Actions::StartTracking:
				if (eyeTrackingStatus == EWaveVREyeTrackingStatus::NOT_START ||
					eyeTrackingStatus == EWaveVREyeTrackingStatus::START_FAILURE)
				{
					eyeTrackingStatus = EWaveVREyeTrackingStatus::STARTING;
					LOGD(LogWaveVREyeRunnable, "Run() Start eye tracking.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartEyeTracking();
					switch (result)
					{
					case WVR_Result::WVR_Error_FeatureNotSupport:
						eyeTrackingStatus = EWaveVREyeTrackingStatus::UNSUPPORT;
						break;
					case WVR_Result::WVR_Success:
						eyeTrackingStatus = EWaveVREyeTrackingStatus::AVAILABLE;
						break;
					default:
						eyeTrackingStatus = EWaveVREyeTrackingStatus::START_FAILURE;
						break;
					}
					LOGD(LogWaveVREyeRunnable, "Run() Start eye tracking result: %d", (uint8)result);
				}
				break;
			case Actions::StopTracking:
				if (eyeTrackingStatus == EWaveVREyeTrackingStatus::AVAILABLE)
				{
					eyeTrackingStatus = EWaveVREyeTrackingStatus::STOPPING;
					LOGD(LogWaveVREyeRunnable, "Run() Stop eye tracking.");
					FWaveVRAPIWrapper::GetInstance()->StopEyeTracking();
					eyeTrackingStatus = EWaveVREyeTrackingStatus::NOT_START;
					LOGD(LogWaveVREyeRunnable, "Run() Eye tracking stopped.");
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
void FWaveVREyeRunnable::Stop()
{
}

FWaveVREyeRunnable* FWaveVREyeRunnable::JoyInit()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FWaveVREyeRunnable();
		LOGD(LogWaveVREyeRunnable, "JoyInit() Create new thread.");
	}
	return Runnable;
}

void FWaveVREyeRunnable::EnsureCompletion()
{
	shutDownThread = true;
	Stop();
	Thread->WaitForCompletion();
}

void FWaveVREyeRunnable::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

void FWaveVREyeRunnable::StartEyeTracking()
{
	qActions.Enqueue(Actions::StartTracking);
}

void FWaveVREyeRunnable::StopEyeTracking()
{
	qActions.Enqueue(Actions::StopTracking);
}

void FWaveVREyeRunnable::RestartEyeTracking()
{
	qActions.Enqueue(Actions::StopTracking);
	qActions.Enqueue(Actions::StartTracking);
}
