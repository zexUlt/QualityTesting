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
#include "CoreMinimal.h"  // Always be first included

#include "WaveVRRenderDoc.generated.h"  // Always be last included

UCLASS()
class WAVEVR_API UWaveVRRenderDoc : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "WaveVR|RenderDoc"
		, meta = (ToolTip = "Let RenderDoc capture frameCount frames after delayInMs.  Range of frameCount is [0, 10].  Range of delayInMs is [0, 60000]."))
	static void CaptureFrame(int frameCount, int delayInMs);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|RenderDoc"
		, meta = (ToolTip = 
		"RenderDoc start to capture frameCount frames every periodInMs microsecond after delayInMs ms.  When frameCount is zero, stop auto capture.  Range of frameCount is [0, 10].  Range of periodInMs is [3000, 600000].  Range of delayInMs is [0, 60000]."))
	static void SetAutoCapture(int frameCount, int periodInMs, int delayInMs);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|RenderDoc"
		, meta = (ToolTip = "Check if RenderDoc is available."))
	static bool Available();
};

namespace Wave {
namespace RenderDoc {

class FRenderDoc
{
public:
	~FRenderDoc();

	void OnFrameBegin();

	void CaptureFrame(int frameCount, int delayInMs);
	void SetAutoCapture(int frameCount, int periodInMs, int delayInMs);

	bool IsLibraryLoaded() const;
	inline static FRenderDoc& GetInstance() { return *instance; }

private:
	void LoadLibrary();
	void UnloadLibrary();

	void StartFrameCapture();
	void EndFrameCapture();

private:
	static FRenderDoc * instance;

	void * libraryHandle;
	void * apiPtr;

private:
	FRenderDoc();

private:
	bool IsCapturing;
	int FrameCaptured;

	bool NeedCapture;
	int CaptureFrameCount;

	bool NeedAutoCapture;
	int AutoCaptureFrameCount;
	int AutoCapturePeriod;

	int TargetFrameCount;

	FDateTime NextTime;
	FDateTime NextTimeAuto;
};

}  // namespace RenderDoc
}  // namespace Wave
