// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveVRControllerModel.h"

#include "WaveVREventCommon.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRHMD.h"

#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogControllerModel, Log, All);

// Sets default values for this component's properties
UWaveVRControllerModel::UWaveVRControllerModel() :
	preConnection(false),
	currConnection(false),
	SpawnController(nullptr),
	realDeviceType(EWVR_DeviceType::DeviceType_Invalid),
	isLeftHand(false),
	deviceTypeChanged(false),
	ddelay(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	LOGD(LogControllerModel, "Device(%d), UWaveVRControllerModel constructor", (uint8)deviceType);
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UWaveVRControllerModel::BeginPlay()
{
	Super::BeginPlay();

	// ...
	checkIfLeftHand();
}

void UWaveVRControllerModel::DestroyController()
{
	LOGD(LogControllerModel, "Device(%d), Should destory controller()", (uint8)deviceType);
	if (SpawnController != nullptr)
	{
		SpawnController->Destroy();
		SpawnController = nullptr;
	}
}

void UWaveVRControllerModel::SetDeviceType(EWVR_DeviceType devType)
{
	if (deviceType != devType) {
		LOGD(LogControllerModel, "Device(%d), deviceType changed", (uint8)deviceType);
		deviceType = devType;
		deviceTypeChanged = true;
		checkIfLeftHand();
	}
}

void UWaveVRControllerModel::checkIfLeftHand()
{
	realDeviceType = deviceType;
}

// Called every frame
void UWaveVRControllerModel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (deviceTypeChanged) {
		LOGD(LogControllerModel, "DeviceType changed to %d, reset", (uint8)deviceType);
		DestroyController();
		checkIfLeftHand();
		deviceTypeChanged = false;
	}

	if (ddelay < (75 * 1)) {
		if ((ddelay % 25) == 0) {
			LOGD(LogControllerModel, "delay %d", (uint8)deviceType);
		}

		ddelay += 1;
		return;
	}

	currConnection = UWaveVRBlueprintFunctionLibrary::IsDevicePoseValid(realDeviceType);

	if (currConnection != preConnection) {
		preConnection = currConnection;
		LOGD(LogControllerModel, "Device(%d), is left hand mode = %d", (uint8)deviceType, isLeftHand);
		LOGD(LogControllerModel, "Device(%d), connect status changed, new connect status = %d", (uint8)deviceType, currConnection);

		if (currConnection) {
			SpawnControllerModel();

			TArray<USceneComponent*> abc = this->GetAttachChildren();

			LOGD(LogControllerModel, "Device(%d), attach child count = %d", (uint8)deviceType, abc.Num());

			for (int i = 0; i < abc.Num(); i++) {
				USceneComponent*a = abc[i];
				LOGD(LogControllerModel, "Device(%d), attach child = %s", (uint8)deviceType, PLATFORM_CHAR(*a->GetName()));
			}
		}
		else {
			DestroyController();
		}
	}
}

//bool UWaveVRControllerModel::IsShowControllerModel()
//{
//	if (FWaveVRHMD::GetInstance() == nullptr)
//		return false;
//
//	bool isFocusCapturedBySystem = FWaveVRHMD::GetInstance()->IsFocusCapturedBySystem();
//	bool isControllerMode = (FWaveVRHMD::GetInstance()->GetInteractionMode() == WVR_InteractionMode::WVR_InteractionMode_Controller);
//
//	return ((!isFocusCapturedBySystem) && isControllerMode);
//}

void UWaveVRControllerModel::SpawnControllerModel()
{
	LOGD(LogControllerModel, "Device(%d), SpawnControllerModel real device type %d ++", (uint8)deviceType, (uint8)realDeviceType);
	if (SpawnController == nullptr)
	{
		NativeModelParameter::SetSpawnDeviceType((WVR_DeviceType)realDeviceType);
		NativeModelParameter::SetAlwaysShow(alwaysShow);
		NativeModelParameter::SetHideWhenIdle(HideWhenIdle);
		NativeModelParameter::SetSpawnFlag(true);

		//FActorSpawnParameters ActorSpawnParameters;
		//ActorSpawnParameters.Name = ((realDeviceType == EWVR_DeviceType::DeviceType_Controller_Right) ? FName("RightControllerActor") : FName("LeftControllerActor"));

		SpawnController = GetWorld()->SpawnActor<AWaveVRNativeModel>(AWaveVRNativeModel::StaticClass(), FTransform::Identity);
		LOGD(LogControllerModel, "Device(%d),  controller %d is spawned.", (uint8)deviceType, (uint8)realDeviceType);

		SpawnController->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

		NativeModelParameter::SetSpawnDeviceType((WVR_DeviceType)WVR_DeviceType::WVR_DeviceType_Invalid);
		NativeModelParameter::SetAlwaysShow(false);
		NativeModelParameter::SetSpawnFlag(false);
	}
}

