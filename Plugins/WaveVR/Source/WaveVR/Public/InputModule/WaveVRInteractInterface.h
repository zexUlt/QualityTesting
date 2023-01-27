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
#include "UObject/Interface.h"
#include "WaveVRInteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWaveVRInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class WAVEVR_API IWaveVRInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WaveVR|InputModule")
	void OnEnter(AActor* caller);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="WaveVR|InputModule")
	void OnHover(AActor* caller);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WaveVR|InputModule")
	void OnExit(AActor* caller);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WaveVR|InputModule")
	void OnClick(AActor* caller);

/**
 * Usage:
 * For the "inherited" UClass object (ex: AActor):
 *
 * IWaveVRInteractInterface* interface = Cast<IWaveVRInteractInterface>(<ActorInstance>);
 * if (interface)
 * {
 *     interface->Execute_OnHover(<ActorInstance>, GetOwner());
 * }
 *
 * For the "implemented" Blueprint object (ex: Actor):
 *
 * if (<ActorInstance>->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass()))
 * {
 *     IWaveVRInteractInterface::Execute_OnHover(<ActorInstance>, GetOwner());
 * }
 **/
};
