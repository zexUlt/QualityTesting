// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "Hand/FWaveVRHandThread.h"
#include "HAL/RunnableThread.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandThread, Log, All);

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FWaveVRHandThread* FWaveVRHandThread::Runnable = NULL;
//***********************************************************

FWaveVRHandThread::FWaveVRHandThread()
	: shutDownThread(false)
	, getSupportedFeature(false)
	, handGestureStatus(EWaveVRHandGestureStatus::UNSUPPORT)
	, m_NaturalTrackerStatus(EWaveVRHandTrackingStatus::UNSUPPORT)
	, m_ElectronicTrackerStatus(EWaveVRHandTrackingStatus::UNSUPPORT)
{
	handGestureDemands = (1 << WVR_HandGestureType::WVR_HandGestureType_Invalid)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_Unknown)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_Fist)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_Five)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_OK)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_ThumbUp)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_IndexUp)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_Palm_Pinch)
		| (1 << WVR_HandGestureType::WVR_HandGestureType_Yeah);

	Thread = FRunnableThread::Create(this, TEXT("FWaveVRHandThread"));
}

FWaveVRHandThread::~FWaveVRHandThread()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FWaveVRHandThread::Init()
{
	LOGD(LogWaveVRHandThread, "Init() handGestureDemands %d", (uint8)handGestureDemands);
	return true;
}

//Run
uint32 FWaveVRHandThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!shutDownThread)
	{
		if (!getSupportedFeature)
		{
			uint64_t supportedFeatures = FWaveVRAPIWrapper::GetInstance()->GetSupportedFeatures();
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_HandGesture) != 0)
				handGestureStatus = EWaveVRHandGestureStatus::NOT_START;
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_HandTracking) != 0)
				m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::NOT_START;
			if ((supportedFeatures & (uint64_t)WVR_SupportedFeature::WVR_SupportedFeature_ElectronicHand) != 0)
				m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::NOT_START;

			LOGD(LogWaveVRHandThread,
				"Run() supportedFeatures %d, handGestureStatus %d, m_NaturalTrackerStatus %d, m_ElectronicTrackerStatus %d",
				(int)supportedFeatures, (int)handGestureStatus, (int)m_NaturalTrackerStatus, (int)m_ElectronicTrackerStatus);
			getSupportedFeature = true;
		}

		bool has_action = qActions.Dequeue(action);
		if (has_action)
		{
			m_mutex.Lock();
			switch (action)
			{
			case Actions::StartGesture:
				if (handGestureStatus == EWaveVRHandGestureStatus::NOT_START || handGestureStatus == EWaveVRHandGestureStatus::START_FAILURE)
				{
					handGestureStatus = EWaveVRHandGestureStatus::STARTING;
					LOGD(LogWaveVRHandThread, "Run() Start hand gesture.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartHandGesture(handGestureDemands);
					switch (result)
					{
					case WVR_Result::WVR_Success:
						handGestureStatus = EWaveVRHandGestureStatus::AVAILABLE;
						break;
					case WVR_Result::WVR_Error_FeatureNotSupport:
						handGestureStatus = EWaveVRHandGestureStatus::UNSUPPORT;
						break;
					default:
						handGestureStatus = EWaveVRHandGestureStatus::START_FAILURE;
						break;
					}
					LOGD(LogWaveVRHandThread, "Run() Start hand gesture result: %d", (uint8)result);
				}
				break;
			case Actions::StopGesture:
				if (handGestureStatus == EWaveVRHandGestureStatus::AVAILABLE)
				{
					handGestureStatus = EWaveVRHandGestureStatus::STOPING;
					LOGD(LogWaveVRHandThread, "Run() Stop hand gesture.");
					FWaveVRAPIWrapper::GetInstance()->StopHandGesture();
					handGestureStatus = EWaveVRHandGestureStatus::NOT_START;
					LOGD(LogWaveVRHandThread, "Run() Hand gesture stopped.");
				}
				break;
			case Actions::StartNaturalTracker:
				if (m_NaturalTrackerStatus == EWaveVRHandTrackingStatus::NOT_START ||
					m_NaturalTrackerStatus == EWaveVRHandTrackingStatus::START_FAILURE)
				{
					m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::STARTING;

					LOGD(LogWaveVRHandThread, "Run() Start natural hand tracker.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartHandTracking(WVR_HandTrackerType::WVR_HandTrackerType_Natural);
					if (result == WVR_Result::WVR_Success)
						m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::AVAILABLE;
					else if (result == WVR_Result::WVR_Error_FeatureNotSupport)
						m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::UNSUPPORT;
					else
						m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::START_FAILURE;

					LOGD(LogWaveVRHandThread, "Run() Start natural hand tracker result: %d", (uint8)result);
				}
				break;
			case Actions::StopNaturalTracker:
				if (m_NaturalTrackerStatus == EWaveVRHandTrackingStatus::AVAILABLE)
				{
					m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::STOPING;
					LOGD(LogWaveVRHandThread, "Run() Stop natural hand tracker.");
					FWaveVRAPIWrapper::GetInstance()->StopHandTracking(WVR_HandTrackerType::WVR_HandTrackerType_Natural);
					m_NaturalTrackerStatus = EWaveVRHandTrackingStatus::NOT_START;
					LOGD(LogWaveVRHandThread, "Run() Natural hand tracker stopped.");
				}
				break;
			case Actions::StartElectronicTracker:
				if (m_ElectronicTrackerStatus == EWaveVRHandTrackingStatus::NOT_START ||
					m_ElectronicTrackerStatus == EWaveVRHandTrackingStatus::START_FAILURE)
				{
					m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::STARTING;

					LOGD(LogWaveVRHandThread, "Run() Start electronic hand tracker.");
					WVR_Result result = FWaveVRAPIWrapper::GetInstance()->StartHandTracking(WVR_HandTrackerType::WVR_HandTrackerType_Electronic);
					if (result == WVR_Result::WVR_Success)
						m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::AVAILABLE;
					else if (result == WVR_Result::WVR_Error_FeatureNotSupport)
						m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::UNSUPPORT;
					else
						m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::START_FAILURE;

					LOGD(LogWaveVRHandThread, "Run() Start electronic hand tracker result: %d", (uint8)result);
				}
				break;
			case Actions::StopElectronicTracker:
				if (m_ElectronicTrackerStatus == EWaveVRHandTrackingStatus::AVAILABLE)
				{
					m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::STOPING;
					LOGD(LogWaveVRHandThread, "Run() Stop electronic hand tracker.");
					FWaveVRAPIWrapper::GetInstance()->StopHandTracking(WVR_HandTrackerType::WVR_HandTrackerType_Electronic);
					m_ElectronicTrackerStatus = EWaveVRHandTrackingStatus::NOT_START;
					LOGD(LogWaveVRHandThread, "Run() Electronic hand tracking stopped.");
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
void FWaveVRHandThread::Stop()
{
}

FWaveVRHandThread* FWaveVRHandThread::JoyInit()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FWaveVRHandThread();
		LOGD(LogWaveVRHandThread, "JoyInit() Create new thread.");
	}
	return Runnable;
}

void FWaveVRHandThread::EnsureCompletion()
{
	shutDownThread = true;
	Stop();
	Thread->WaitForCompletion();
}

void FWaveVRHandThread::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

void FWaveVRHandThread::StartHandGesture()
{
	qActions.Enqueue(Actions::StartGesture);
}

void FWaveVRHandThread::StopHandGesture()
{
	qActions.Enqueue(Actions::StopGesture);
}

void FWaveVRHandThread::RestartHandGesture()
{
	qActions.Enqueue(Actions::StopGesture);
	qActions.Enqueue(Actions::StartGesture);
}

void FWaveVRHandThread::StartHandTracking(EWaveVRTrackerType tracker)
{
	if (tracker == EWaveVRTrackerType::Natural)
		qActions.Enqueue(Actions::StartNaturalTracker);
	if (tracker == EWaveVRTrackerType::Electronic)
		qActions.Enqueue(Actions::StartElectronicTracker);
}

void FWaveVRHandThread::StopHandTracking(EWaveVRTrackerType tracker)
{
	if (tracker == EWaveVRTrackerType::Natural)
		qActions.Enqueue(Actions::StopNaturalTracker);
	if (tracker == EWaveVRTrackerType::Electronic)
		qActions.Enqueue(Actions::StopElectronicTracker);
}

void FWaveVRHandThread::RestartHandTracking(EWaveVRTrackerType tracker)
{
	if (tracker == EWaveVRTrackerType::Natural)
	{
		qActions.Enqueue(Actions::StopNaturalTracker);
		qActions.Enqueue(Actions::StartNaturalTracker);
	}
	if (tracker == EWaveVRTrackerType::Electronic)
	{
		qActions.Enqueue(Actions::StopElectronicTracker);
		qActions.Enqueue(Actions::StartElectronicTracker);
	}
}
