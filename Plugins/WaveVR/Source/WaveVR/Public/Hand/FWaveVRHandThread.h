// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"

#include "WaveVRHandEnums.h"

class WAVEVR_API FWaveVRHandThread : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FWaveVRHandThread* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	enum Actions
	{
		None,
		StartGesture,
		StopGesture,
		StartNaturalTracker,
		StopNaturalTracker,
		StartElectronicTracker,
		StopElectronicTracker,
	};

public:
	FWaveVRHandThread();
	virtual ~FWaveVRHandThread();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();


	//~~~ Starting and Stopping Thread ~~~


	/*
		Start the thread and the worker from static (easy access)!
		This code ensures only 1 Prime Number thread will be able to run at a time.
		This function returns a handle to the newly started instance.
	*/
	static FWaveVRHandThread* JoyInit();

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();


	// ~~~ WaveVR related components ~~~
	void StartHandGesture();
	void StopHandGesture();
	void RestartHandGesture();
	bool IsHandGestureAvailable() { return handGestureStatus == EWaveVRHandGestureStatus::AVAILABLE ? true : false; }
	EWaveVRHandGestureStatus GetHandGestureStatus() { return handGestureStatus; }

	void StartHandTracking(EWaveVRTrackerType tracker);
	void StopHandTracking(EWaveVRTrackerType tracker);
	void RestartHandTracking(EWaveVRTrackerType tracker);
	bool IsHandTrackingAvailable(EWaveVRTrackerType tracker) {
		if (tracker == EWaveVRTrackerType::Natural)
			return m_NaturalTrackerStatus == EWaveVRHandTrackingStatus::AVAILABLE ? true : false;
		if (tracker == EWaveVRTrackerType::Electronic)
			return m_ElectronicTrackerStatus == EWaveVRHandTrackingStatus::AVAILABLE ? true : false;
		return false;
	}
	EWaveVRHandTrackingStatus GetHandTrackingStatus(EWaveVRTrackerType tracker) {
		if (tracker == EWaveVRTrackerType::Natural)
			return m_NaturalTrackerStatus;
		if (tracker == EWaveVRTrackerType::Electronic)
			return m_ElectronicTrackerStatus;
		return EWaveVRHandTrackingStatus::UNSUPPORT;
	}

private:
	FCriticalSection m_mutex;


	// ~~~ WaveVR related components ~~~
	bool shutDownThread;
	bool getSupportedFeature;
	volatile EWaveVRHandGestureStatus handGestureStatus;
	volatile EWaveVRHandTrackingStatus m_NaturalTrackerStatus, m_ElectronicTrackerStatus;
	volatile uint64_t handGestureDemands;

	Actions action = Actions::None;
	TQueue<Actions> qActions;
};
