// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRBlueprintFunctionLibrary.h"

// Engine
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

// Wave
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRHMD.h"
#include "PoseManagerImp.h"
#include "WaveVRSplash.h"
#include "Platforms/Editor/WaveVRDirectPreview.h"
#include "WaveVRScreenshot.h"
#include "WaveVRUtils.h"

using namespace wvr::utils;


#if PLATFORM_ANDROID
extern FString GExternalFilePath;
#endif

static bool IsPlayInEditor()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World()->IsPlayInEditor())
		{
			return true;
		}
	}
	return false;
}

bool IsRegularDevice(EWVR_DeviceType type)
{
	if (type == EWVR_DeviceType::DeviceType_HMD ||
		type == EWVR_DeviceType::DeviceType_Controller_Left ||
		type == EWVR_DeviceType::DeviceType_Controller_Right)
	{
		return true;
	}
	return false;
}

EWVR_DeviceType GetDeviceType(WVR_DeviceType type)
{
	EWVR_DeviceType ResType = EWVR_DeviceType::DeviceType_Invalid;

	switch (type)
	{
	case WVR_DeviceType::WVR_DeviceType_HMD:
		ResType = EWVR_DeviceType::DeviceType_HMD;
		break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Right:
		ResType = EWVR_DeviceType::DeviceType_Controller_Right;
		break;
	case WVR_DeviceType::WVR_DeviceType_Controller_Left:
		ResType = EWVR_DeviceType::DeviceType_Controller_Left;
		break;
	case WVR_DeviceType::WVR_DeviceType_Camera:
		ResType = EWVR_DeviceType::DeviceType_Camera;
		break;
	case WVR_DeviceType::WVR_DeviceType_EyeTracking:
		ResType = EWVR_DeviceType::DeviceType_EyeTracking;
		break;
	case WVR_DeviceType::WVR_DeviceType_HandGesture_Right:
		ResType = EWVR_DeviceType::DeviceType_HandGesture_Right;
		break;
	case WVR_DeviceType::WVR_DeviceType_HandGesture_Left:
		ResType = EWVR_DeviceType::DeviceType_HandGesture_Left;
		break;
	case WVR_DeviceType::WVR_DeviceType_NaturalHand_Right:
		ResType = EWVR_DeviceType::DeviceType_NaturalHand_Right;
		break;
	case WVR_DeviceType::WVR_DeviceType_NaturalHand_Left:
		ResType = EWVR_DeviceType::DeviceType_NaturalHand_Left;
		break;
	case WVR_DeviceType::WVR_DeviceType_ElectronicHand_Right:
		ResType = EWVR_DeviceType::DeviceType_ElectronicHand_Right;
		break;
	case WVR_DeviceType::WVR_DeviceType_ElectronicHand_Left:
		ResType = EWVR_DeviceType::DeviceType_ElectronicHand_Left;
		break;
	default:
		break;
	}

	return ResType;
}

EWVR_InputId GetInputId(WVR_InputId id)
{
	EWVR_InputId ResId = EWVR_InputId::NoUse;

	switch (id)
	{
	case WVR_InputId::WVR_InputId_Alias1_Menu:
		ResId = EWVR_InputId::Menu;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Grip:
		ResId = EWVR_InputId::Grip;
		break;
	case WVR_InputId::WVR_InputId_Alias1_DPad_Left:
		ResId = EWVR_InputId::DPad_Left;
		break;
	case WVR_InputId::WVR_InputId_Alias1_DPad_Up:
		ResId = EWVR_InputId::DPad_Up;
		break;
	case WVR_InputId::WVR_InputId_Alias1_DPad_Right:
		ResId = EWVR_InputId::DPad_Right;
		break;
	case WVR_InputId::WVR_InputId_Alias1_DPad_Down:
		ResId = EWVR_InputId::DPad_Down;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Volume_Up:
		ResId = EWVR_InputId::Volume_Up;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Volume_Down:
		ResId = EWVR_InputId::Volume_Down;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Bumper:
		ResId = EWVR_InputId::Bumper;
		break;
	case WVR_InputId::WVR_InputId_Alias1_A:
		ResId = EWVR_InputId::A;
		break;
	case WVR_InputId::WVR_InputId_Alias1_B:
		ResId = EWVR_InputId::B;
		break;
	case WVR_InputId::WVR_InputId_Alias1_X:
		ResId = EWVR_InputId::X;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Y:
		ResId = EWVR_InputId::Y;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Back:
		ResId = EWVR_InputId::Back;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Enter:
		ResId = EWVR_InputId::Enter;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Touchpad:
		ResId = EWVR_InputId::Touchpad;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Trigger:
		ResId = EWVR_InputId::Trigger;
		break;
	case WVR_InputId::WVR_InputId_Alias1_Thumbstick:
		ResId = EWVR_InputId::Thumbstick;
		break;
	default:
		break;
	}

	return ResId;
}

UWaveVRBlueprintFunctionLibrary::UWaveVRBlueprintFunctionLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

bool UWaveVRBlueprintFunctionLibrary::GetDevicePose(FVector& OutPosition, FRotator& OutOrientation, EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return false;

#if WITH_EDITOR
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	if (IsPlayInEditor() && !HMD->IsDirectPreview())
	{
		//LOGD(LogWaveVRBPFunLib, "GetDevicePose() in PIE.");
		return HMD->GetSimulationPose(OutPosition, OutOrientation, static_cast<WVR_DeviceType>(type));
	}
#endif // WITH_EDITOR

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	PoseManagerImp::Device* device = PoseMngr->GetDevice(static_cast<WVR_DeviceType>(type));
	OutOrientation = device->rotation;
	OutPosition = device->position;
	return device->pose.pose.isValidPose;
}

bool UWaveVRBlueprintFunctionLibrary::GetDevicePose2(FVector& OutPosition, FQuat& OutOrientation, EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return false;

#if WITH_EDITOR
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	if (IsPlayInEditor() && !HMD->IsDirectPreview())
	{
		//LOGD(LogWaveVRBPFunLib, "GetDevicePose() in PIE.");
		return HMD->GetSimulationPose(OutPosition, OutOrientation, static_cast<WVR_DeviceType>(type));
	}
#endif // WITH_EDITOR

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	PoseManagerImp::Device* device = PoseMngr->GetDevice(static_cast<WVR_DeviceType>(type));
	OutOrientation = device->orientation;
	OutPosition = device->position;
	return device->pose.pose.isValidPose;
}

void UWaveVRBlueprintFunctionLibrary::SetPosePredictEnabled(EWVR_DeviceType type, bool enabled_position_predict, bool enabled_rotation_predict)
{
	if (!IsRegularDevice(type))
		return;

	FWaveVRAPIWrapper::GetInstance()->SetPosePredictEnabled(static_cast<WVR_DeviceType>(type), enabled_position_predict, enabled_rotation_predict);
}

FMatrix UWaveVRBlueprintFunctionLibrary::GetDeviceMatrix(EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return FMatrix::Identity;

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	PoseManagerImp::Device* device = PoseMngr->GetDevice(static_cast<WVR_DeviceType>(type));
	auto pose = device->pose.pose.poseMatrix;
	FMatrix m {};
	float * dest = &(m.M[0][0]);
	float * src = &(pose.m[0][0]);
	for (int i = 0; i < 16; i++)
		*dest++ = *src++;
	return m;
}

FVector UWaveVRBlueprintFunctionLibrary::GetDeviceVelocity(EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return FVector::ZeroVector;

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	PoseManagerImp::Device* device = PoseMngr->GetDevice(static_cast<WVR_DeviceType>(type));
	WVR_Vector3f_t velocity = device->pose.pose.velocity;
	return FVector(velocity.v[0], velocity.v[1], velocity.v[2]);
}

FVector UWaveVRBlueprintFunctionLibrary::GetDeviceAngularVelocity(EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return FVector::ZeroVector;

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	PoseManagerImp::Device* device = PoseMngr->GetDevice(static_cast<WVR_DeviceType>(type));
	WVR_Vector3f_t angularv = device->pose.pose.angularVelocity;
	return FVector(angularv.v[0], angularv.v[1], angularv.v[2]);
}

bool UWaveVRBlueprintFunctionLibrary::IsDevicePoseValid(EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return false;

#if WITH_EDITOR
	if (GIsEditor && !WaveVRDirectPreview::IsDirectPreview())
		return true;
#endif // WITH_EDITOR

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	return PoseMngr->IsDevicePoseValid(static_cast<WVR_DeviceType>(type));
}

EWVR_DOF UWaveVRBlueprintFunctionLibrary::GetSupportedNumOfDoF(EWVR_DeviceType type)
{
	if (!IsRegularDevice(type))
		return EWVR_DOF::DOF_SYSTEM;

	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	return PoseMngr->GetSupportedNumOfDoF(static_cast<WVR_DeviceType>(type));
}

void UWaveVRBlueprintFunctionLibrary::SetTrackingHMDPosition(bool IsTrackingPosition)
{
	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	return PoseMngr->SetTrackingHMDPosition(IsTrackingPosition);
}

bool UWaveVRBlueprintFunctionLibrary::IsTrackingHMDPosition()
{
	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	return PoseMngr->IsTrackingHMDPosition();
}

void UWaveVRBlueprintFunctionLibrary::SetTrackingHMDRotation(bool IsTrackingRotation)
{
	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	return PoseMngr->SetTrackingHMDRotation(IsTrackingRotation);
}

void UWaveVRBlueprintFunctionLibrary::SetTrackingOrigin3Dof()
{
	PoseManagerImp* PoseMngr = PoseManagerImp::GetInstance();
	return PoseMngr->SetTrackingOrigin3Dof();
}

bool UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType type)
{
#if WITH_EDITOR
	if (GIsEditor && !WaveVRDirectPreview::IsDirectPreview())
		return true;
	if (GIsEditor && WaveVRDirectPreview::IsDirectPreview())
		return FWaveVRAPIWrapper::GetInstance()->IsDeviceConnected((WVR_DeviceType)type);
#endif // WITH_EDITOR

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	return HMD->IsDeviceConnected(static_cast<WVR_DeviceType>(type));
}

bool UWaveVRBlueprintFunctionLibrary::IsInputFocusCapturedBySystem()
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	return HMD->IsFocusCapturedBySystem();
}

void UWaveVRBlueprintFunctionLibrary::InAppRecenter(ERecenterType type)
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		switch (type)
		{
		case ERecenterType::Disabled:
			LOGI(LogWaveVRBPFunLib, "RecenterSwitchType = %s", "WVR_RecenterType_Disabled");
			WVR()->InAppRecenter(WVR_RecenterType_Disabled);
			break;
		case ERecenterType::YawOnly:
			LOGI(LogWaveVRBPFunLib, "RecenterSwitchType = %s", "WVR_RecenterType_YawOnly");
			WVR()->InAppRecenter(WVR_RecenterType_YawOnly);
			break;
		case ERecenterType::YawAndPosition:
			LOGI(LogWaveVRBPFunLib, "RecenterSwitchType = %s", "WVR_RecenterType_YawAndPosition");
			WVR()->InAppRecenter(WVR_RecenterType_YawAndPosition);
			break;
		case ERecenterType::RotationAndPosition:
			LOGI(LogWaveVRBPFunLib, "RecenterSwitchType = %s", "WVR_RecenterType_RotationAndPosition");
			WVR()->InAppRecenter(WVR_RecenterType_RotationAndPosition);
			break;
		case ERecenterType::Position:
			LOGI(LogWaveVRBPFunLib, "RecenterSwitchType = %s", "WVR_RecenterType_Position");
			WVR()->InAppRecenter(WVR_RecenterType_Position);
			break;
		default:
			break;
		}
	}
}

void UWaveVRBlueprintFunctionLibrary::SetPoseSimulationOption(SimulatePosition Option)
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		LOGD(LogWaveVRBPFunLib, "SetPoseSimulationOption() %d", (uint8)Option);
		if (Option == SimulatePosition::WhenNoPosition)
		{
			FWaveVRAPIWrapper::GetInstance()->SetArmModel(WVR_SimulationType::WVR_SimulationType_Auto);
		}
		else if (Option == SimulatePosition::ForceSimulation)
		{
			FWaveVRAPIWrapper::GetInstance()->SetArmModel(WVR_SimulationType::WVR_SimulationType_ForceOn);
		}
		else
		{
			FWaveVRAPIWrapper::GetInstance()->SetArmModel(WVR_SimulationType::WVR_SimulationType_ForceOff);
		}
	}
}

void UWaveVRBlueprintFunctionLibrary::SetFollowHead(bool follow)
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		LOGD(LogWaveVRBPFunLib, "SetFollowHead() %d", (uint8)follow);
		FWaveVRAPIWrapper::GetInstance()->SetArmSticky(follow);
	}
}

float UWaveVRBlueprintFunctionLibrary::getDeviceBatteryPercentage(EWVR_DeviceType type) {
	if (!IsRegularDevice(type))
		return 0;

	float ret = FWaveVRAPIWrapper::GetInstance()->GetDeviceBatteryPercentage((WVR_DeviceType) type);
	LOGD(LogWaveVRBPFunLib, "getDeviceBatteryPercentage() %d -> %f", (uint8)type, ret);
	return ret;
}

bool UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode()
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr)
		return false;

	return HMD->IsLeftHandedMode();
}

int UWaveVRBlueprintFunctionLibrary::GetHoveredWidgetSeqId(UUserWidget* ParentWidget, TArray<UWidget*>& ChildWidgets)
{

	int index = 0;
	ChildWidgets.Empty();

	if (ParentWidget)
	{
		UWidgetTree::GetChildWidgets(ParentWidget->GetRootWidget(), ChildWidgets);
		for (UWidget* Widget : ChildWidgets)
		{
			index += 1;
			if (Widget->IsHovered())
			{
				return index;
			}
		}
	}
	return index;
}

void UWaveVRBlueprintFunctionLibrary::EnableNeckModel(bool enable)
{
	LOGD(LogWaveVRBPFunLib, "EnableNeckModel() %d", (uint8)enable);
	FWaveVRAPIWrapper::GetInstance()->SetNeckModelEnabled(enable);
}

EWVR_InputId UWaveVRBlueprintFunctionLibrary::GetInputMappingPair(EWVR_DeviceType type, EWVR_InputId button)
{
#if WITH_EDITOR
	return button;
#endif
	if (!IsRegularDevice(type))
		return EWVR_InputId::NoUse;

	EWVR_InputId _id = EWVR_InputId::NoUse;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr)
	{
		WVR_InputId ref_btn = static_cast<WVR_InputId>(button);
		bool result = HMD->GetInputMappingPair(static_cast<WVR_DeviceType>(type), ref_btn);
		if (result)
			_id = GetInputId(ref_btn);
	}

	return _id;
}

bool UWaveVRBlueprintFunctionLibrary::IsButtonAvailable(EWVR_DeviceType type, EWVR_InputId button)
{
	if (!IsRegularDevice(type))
		return false;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;

	return HMD->IsButtonAvailable((WVR_DeviceType)type, static_cast<WVR_InputId>(button));
}

bool UWaveVRBlueprintFunctionLibrary::GetRenderTargetSize(FIntPoint & OutSize)
{
	LOG_FUNC();

	uint32 width = 1600, height = 900;
	FWaveVRAPIWrapper::GetInstance()->GetRenderTargetSize(&width, &height);
	OutSize.X = width;
	OutSize.Y = height;
	return true;
}

FString UWaveVRBlueprintFunctionLibrary::GetRenderModelName(EWVR_Hand hand) {
	FString retString = FString(TEXT("Generic"));

#if PLATFORM_ANDROID
	uint32_t num2 = 0;
	bool bIsLeftHanded = UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode();
	char str[128] = {0};
	FString str_name = FString(TEXT("GetRenderModelName")); //Controller getParameter
	if (bIsLeftHanded) {
		if (hand == EWVR_Hand::Hand_Controller_Right) {
			num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Left, TCHAR_TO_ANSI(*str_name), str, 128);
		}
		else if (hand == EWVR_Hand::Hand_Controller_Left) {
			num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Right, TCHAR_TO_ANSI(*str_name), str, 128);
		}
	}
	else {
		if (hand == EWVR_Hand::Hand_Controller_Right) {
			num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Right, TCHAR_TO_ANSI(*str_name), str, 128);
		}
		else if (hand == EWVR_Hand::Hand_Controller_Left) {
			num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Left, TCHAR_TO_ANSI(*str_name), str, 128);
		}
	}

	retString = FString(ANSI_TO_TCHAR(str));
#endif

	return retString;
}

AActor * UWaveVRBlueprintFunctionLibrary::LoadCustomControllerModel(EWVR_DeviceType type, EWVR_DOF dof, FTransform transform) {
	if (!IsRegularDevice(type))
		return nullptr;

	// auto cls = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("/WaveVR/Blueprints/Controller_BP_3DoF_Finch"));
	// asyc load asset (materials, texture ...)
	//auto cls = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("/WaveVR/Materials/Touch_Icon_Sprite"));
	FString GenericControllerModel = FString(TEXT("WVR_CONTROLLER_GENERIC"));
	FString ControllerName = FString(TEXT(""));
	char *str = new char[30];
	FString str_name = FString(TEXT("GetRenderModelName")); //Controller getParameter
	FString BPClassName = FString(TEXT("/WaveVR/Blueprints/"));

#if PLATFORM_ANDROID
	bool bIsLeftHanded = UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode();

	if (bIsLeftHanded) {
		LOGI(LogWaveVRBPFunLib, "is left hand mode");
		if (type == EWVR_DeviceType::DeviceType_Controller_Right) {
			uint32_t num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Left, TCHAR_TO_ANSI(*str_name), str, 30);
		}
		else if (type == EWVR_DeviceType::DeviceType_Controller_Left) {
			uint32_t num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Right, TCHAR_TO_ANSI(*str_name), str, 30);
		}
	} else {
		LOGI(LogWaveVRBPFunLib, "is not  left hand mode");
		if (type == EWVR_DeviceType::DeviceType_Controller_Right) {
			uint32_t num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Right, TCHAR_TO_ANSI(*str_name), str, 30);
		}
		else if (type == EWVR_DeviceType::DeviceType_Controller_Left) {
			uint32_t num2 = FWaveVRAPIWrapper::GetInstance()->GetParameters(WVR_DeviceType_Controller_Left, TCHAR_TO_ANSI(*str_name), str, 30);
		}
	}

	ControllerName.Append(FString(ANSI_TO_TCHAR(str)));

	if (dof == EWVR_DOF::DOF_3) {
		ControllerName.Append(FString(TEXT("_3DOF_MC_")));
	}
	else if (dof == EWVR_DOF::DOF_6) {
		ControllerName.Append(FString(TEXT("_6DOF_MC_")));
	}
	else {
		WVR_NumDoF tmp = WVR_NumDoF::WVR_NumDoF_3DoF;
		if (type == EWVR_DeviceType::DeviceType_Controller_Right) {
			tmp = FWaveVRAPIWrapper::GetInstance()->GetDegreeOfFreedom(WVR_DeviceType_Controller_Right);
		}
		else if (type == EWVR_DeviceType::DeviceType_Controller_Left) {
			tmp = FWaveVRAPIWrapper::GetInstance()->GetDegreeOfFreedom(WVR_DeviceType_Controller_Left);
		}
		if (tmp == WVR_NumDoF::WVR_NumDoF_3DoF) {
			ControllerName.Append(FString(TEXT("_3DOF_MC_")));
		}
		else {
			ControllerName.Append(FString(TEXT("_6DOF_MC_")));
		}
	}

	if (type == EWVR_DeviceType::DeviceType_Controller_Right) {
		ControllerName.Append(FString(TEXT("R")));
	}
	else if (type == EWVR_DeviceType::DeviceType_Controller_Left) {
		ControllerName.Append(FString(TEXT("L")));
	}

	FString tmpName = ControllerName;

	ControllerName.Append(FString(TEXT(".")));
	ControllerName.Append(tmpName);
	ControllerName.Append(FString(TEXT("_C")));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *ControllerName);
	LOGI(LogWaveVRBPFunLib, "ControllerName = %s", TCHAR_TO_ANSI(*ControllerName));

	BPClassName.Append(ControllerName);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *BPClassName);
	LOGI(LogWaveVRBPFunLib, "BPClassName = %s", TCHAR_TO_ANSI(*BPClassName));
#else
	GenericControllerModel.Append(FString(TEXT("_3DOF_MC_R")));
	FString tmpName = GenericControllerModel;
	GenericControllerModel.Append(FString(TEXT(".")));
	GenericControllerModel.Append(tmpName);
	GenericControllerModel.Append(FString(TEXT("_C")));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *GenericControllerModel);
	LOGI(LogWaveVRBPFunLib, "ControllerName = %s", TCHAR_TO_ANSI(*GenericControllerModel));

	BPClassName.Append(GenericControllerModel);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *BPClassName);
	LOGI(LogWaveVRBPFunLib, "BPClassName = %s", TCHAR_TO_ANSI(*BPClassName));
#endif
	//load blueprints
	//UClass* cls = LoadObject< UClass >(nullptr, TEXT("/WaveVR/Blueprints/Controller_BP_3DoF_Finch.Controller_BP_3DoF_Finch_C"));
	//BPClassName = FString(TEXT("/WaveVR/Blueprints/WVR_CONTROLLER_GENERIC_3DOF_MC_R.WVR_CONTROLLER_GENERIC_3DOF_MC_R_C"));
	UClass* cls1 = LoadObject< UClass >(nullptr, *BPClassName);

	if (cls1 != nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Controller model loaded "));
		LOGI(LogWaveVRBPFunLib, "UWaveVRBlueprintFunctionLibrary::BeginPlay load model BP");
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Controller model is null, try load generic controller model "));
		LOGI(LogWaveVRBPFunLib, "Controller model is null, try load generic controller model");

		BPClassName = FString(TEXT("/WaveVR/Blueprints/"));
		GenericControllerModel = FString(TEXT("WVR_CONTROLLER_GENERIC_3DOF_MC_"));
		if (type == EWVR_DeviceType::DeviceType_Controller_Right) {
			GenericControllerModel.Append(FString(TEXT("R")));
		}
		else if (type == EWVR_DeviceType::DeviceType_Controller_Left) {
			GenericControllerModel.Append(FString(TEXT("L")));
		}
		FString tmpName2 = GenericControllerModel;
		GenericControllerModel.Append(FString(TEXT(".")));
		GenericControllerModel.Append(tmpName2);
		GenericControllerModel.Append(FString(TEXT("_C")));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *GenericControllerModel);
		LOGI(LogWaveVRBPFunLib, "ControllerName = %s", TCHAR_TO_ANSI(*GenericControllerModel));

		BPClassName.Append(GenericControllerModel);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *BPClassName);
		LOGI(LogWaveVRBPFunLib, "BPClassName = %s", TCHAR_TO_ANSI(*BPClassName));

		cls1 = LoadObject< UClass >(nullptr, *BPClassName);
		if (cls1 == nullptr) {
			LOGI(LogWaveVRBPFunLib, "Generic controller model is null");
			return nullptr;
		}
		else {
			LOGI(LogWaveVRBPFunLib, "Generic controller model is loaded");
		}
	}

	//UBlueprint * bp = Cast<UBlueprint>(cls);
	/*
	if (!bp) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Failed to load UClass 2  "));
		return nullptr;
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UClass LOADED!!!! 2 " + bp->GetName()));
	}*/

	//TSubclassOf<class UObject> MyItemBlueprint;

	//MyItemBlueprint = (UClass*)bp->GeneratedClass;
	UWorld* const World = GWorld->GetWorld();
	UObject* SpawnObject = nullptr;
	AActor* SpawnActor = nullptr;
	if (World) {
		FActorSpawnParameters SpawnParams;
		//SpawnParams.Instigator = this;
		SpawnActor = World->SpawnActor<AActor>(cls1, transform.GetLocation(), transform.Rotator(), SpawnParams);
		//SpawnObject = World->SpawnActor<UObject>(cls, { 0,0,0 }, { 0,0,0 }, SpawnParams);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("NO WORLD!!!!"));
	}
	delete[] str;
	return SpawnActor;
}

UTexture2D* UWaveVRBlueprintFunctionLibrary::GetTexture2DFromImageFile(
	FString imageFileName,
	FString imagePath,
	EUTex2DFmtType type)
{
	TArray<uint8> RawFileData;
	UTexture2D* UImageTexture2D = nullptr;
	FString FFName;

	LOGI(LogWaveVRBPFunLib, "Oz,(in) image file name: %s, image file path: %s", PLATFORM_CHAR(*imageFileName), PLATFORM_CHAR(*imagePath));

	if (type == EUTex2DFmtType::BMP) {
		FFName = imageFileName + "." + "bmp";
	}
	else if (type == EUTex2DFmtType::PNG) {
		FFName = imageFileName + "." + "png";
	}
	else if (type == EUTex2DFmtType::JPEG) {
		FFName = imageFileName + "." + "jpeg";
	}
	else
	{
		;;
	}

	FString androidRootDirPath = "";

#if PLATFORM_ANDROID
	TArray<FString> Folders;
	//Get external root dir path:
	GExternalFilePath.ParseIntoArray(Folders, TEXT("/"));
	for (FString Folder : Folders) {
		androidRootDirPath += FString("/..");
	}
#endif

	const FString FilePath = androidRootDirPath + imagePath + FFName;

	LOGI(LogWaveVRBPFunLib, "Oz, (out) image androidRootDirPath: %s, image file name: %s, image file path: %s", PLATFORM_CHAR(*androidRootDirPath), PLATFORM_CHAR(*imageFileName), PLATFORM_CHAR(*imagePath));

	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath /*"<path to file>"*/))
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

		TSharedPtr<IImageWrapper> ImageWrapper;

		if ( type == EUTex2DFmtType::BMP ) {
			ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP);
		}
		else if ( type == EUTex2DFmtType::PNG ){
			ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		}
		else if ( type == EUTex2DFmtType::JPEG ){
			ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		}
		else {
			;;
		}

		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			TArray<uint8> UncompressedBGRA;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				UImageTexture2D = UTexture2D::CreateTransient(  ImageWrapper->GetWidth(),
										ImageWrapper->GetHeight(),
										PF_B8G8R8A8);

				void* TextureData = UImageTexture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
				UImageTexture2D->PlatformData->Mips[0].BulkData.Unlock();

				UImageTexture2D->UpdateResource();
			}
		}
	}
	return UImageTexture2D;
}

#pragma region
bool UWaveVRBlueprintFunctionLibrary::GetInputButtonState(EWVR_DeviceType type, EWVR_InputId id)
{
	if (!IsRegularDevice(type))
		return false;

	return FWaveVRAPIWrapper::GetInstance()->GetInputButtonState(static_cast<WVR_DeviceType>(type), static_cast<WVR_InputId>(id));
}
bool UWaveVRBlueprintFunctionLibrary::GetInputTouchState(EWVR_DeviceType type, EWVR_InputId id)
{
	if (!IsRegularDevice(type))
		return false;

	return FWaveVRAPIWrapper::GetInstance()->GetInputTouchState(static_cast<WVR_DeviceType>(type), static_cast<WVR_InputId>(id));
}
bool UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType type, EWVR_InputId id)
{
	if (!IsRegularDevice(type))
		return false;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr)
		return false;

/*#if WITH_EDITOR
	if (IsPlayInEditor() && !HMD->IsDirectPreview())
	{
		return HMD->GetSimulationPressState(static_cast<WVR_DeviceType>(type), static_cast<WVR_InputId>(id));
	}
#endif // WITH_EDITOR*/

	return HMD->GetButtonPressState(static_cast<WVR_DeviceType>(type), static_cast<WVR_InputId>(id));
}
bool UWaveVRBlueprintFunctionLibrary::IsButtonTouched(EWVR_DeviceType type, EWVR_TouchId id)
{
	if (!IsRegularDevice(type))
		return false;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	return HMD->GetButtonTouchState(static_cast<WVR_DeviceType>(type), static_cast<WVR_InputId>(id));
}
#pragma endregion Button

bool UWaveVRBlueprintFunctionLibrary::IsRenderFoveationSupport() {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	return HMD->IsRenderFoveationSupport();
}

bool UWaveVRBlueprintFunctionLibrary::IsRenderFoveationEnabled() {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;
	return HMD->IsRenderFoveationEnabled();
}

void UWaveVRBlueprintFunctionLibrary::SetFoveationMode(EWVR_FoveationMode Mode) {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return;

	WVR_FoveationMode mode = WVR_FoveationMode::WVR_FoveationMode_Default;

	switch (Mode) {
	case EWVR_FoveationMode::Disable:
		mode = WVR_FoveationMode::WVR_FoveationMode_Disable;
		break;
	case EWVR_FoveationMode::Enable:
		mode = WVR_FoveationMode::WVR_FoveationMode_Enable;
		break;
	case EWVR_FoveationMode::Default:
		mode = WVR_FoveationMode::WVR_FoveationMode_Default;
		break;
	}
	HMD->SetFoveationMode(mode);
}

void UWaveVRBlueprintFunctionLibrary::SetFoveationParams(EEye Eye, float Focal_X, float Focal_Y, float FOV, EWVR_PeripheralQuality PeripheralQuality) {

	WVR_RenderFoveationParams_t FoveatParam;

	FoveatParam.focalX = Focal_X;
	FoveatParam.focalY = Focal_Y;
	switch (PeripheralQuality) {
	case EWVR_PeripheralQuality::Low:
		FoveatParam.periQuality = WVR_PeripheralQuality::WVR_PeripheralQuality_Low;
		break;
	case EWVR_PeripheralQuality::Medium:
		FoveatParam.periQuality = WVR_PeripheralQuality::WVR_PeripheralQuality_Medium;
		break;
	case EWVR_PeripheralQuality::High:
		FoveatParam.periQuality = WVR_PeripheralQuality::WVR_PeripheralQuality_High;
		break;
	}
	FoveatParam.fovealFov = FOV;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return;
	EStereoscopicPass eSSPEye = (Eye == EEye::LEFT) ? EStereoscopicPass::eSSP_LEFT_EYE : EStereoscopicPass::eSSP_RIGHT_EYE;
	HMD->SetFoveationParams(eSSPEye, FoveatParam);
}

void UWaveVRBlueprintFunctionLibrary::GetFoveationParams(EEye Eye, float& Focal_X, float& Focal_Y, float& FOV, EWVR_PeripheralQuality& PeripheralQuality) {

	WVR_RenderFoveationParams_t FoveatParam;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return;
	EStereoscopicPass eSSPEye = (Eye == EEye::LEFT) ? EStereoscopicPass::eSSP_LEFT_EYE : EStereoscopicPass::eSSP_RIGHT_EYE;
	HMD->GetFoveationParams(eSSPEye, FoveatParam);

	Focal_X = FoveatParam.focalX;
	Focal_Y = FoveatParam.focalY;
	FOV = FoveatParam.fovealFov;
	PeripheralQuality = (EWVR_PeripheralQuality)FoveatParam.periQuality;
}

bool UWaveVRBlueprintFunctionLibrary::IsAdaptiveQualityEnabled() {
	return FWaveVRAPIWrapper::GetInstance()->IsAdaptiveQualityEnabled();
}

bool UWaveVRBlueprintFunctionLibrary::EnableAdaptiveQuality_K2(bool Enable, EAdaptiveQualityMode Mode, bool SendQualityEvent, bool AutoFoveation) {

	LOGD(LogWaveVRBPFunLib, "Enable AdaptiveQuality : Enable(%u), Mode(%u), SendQualityEvent(%u), AutoFoveation(%u)", Enable, (uint8)Mode, SendQualityEvent, AutoFoveation);

	if (FWaveVRAPIWrapper::GetInstance()->GetWaveRuntimeVersion() < 4) {
		LOGD(LogWaveVRBPFunLib, "AdaptiveQuality have no effect becuase the service api level is less than 4");
		return false;
	}

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return false;

	uint32_t strategyFlags = WVR_QualityStrategy_Default;
	if (Mode == EAdaptiveQualityMode::Quality_Oriented) {
		SendQualityEvent = true;
		AutoFoveation = true;
		strategyFlags |= WVR_QualityStrategy_Reserved_2;
	} else if (Mode == EAdaptiveQualityMode::Performance_Oriented) {
		SendQualityEvent = true;
		AutoFoveation = true;
		strategyFlags |= WVR_QualityStrategy_Reserved_1;
		strategyFlags |= WVR_QualityStrategy_AutoAMC;
	}

	if (SendQualityEvent) {
		strategyFlags |= WVR_QualityStrategy_SendQualityEvent;
	}
	if (AutoFoveation) {
		strategyFlags |= WVR_QualityStrategy_AutoFoveation;
	}

	// Cache AdaptiveQualityState which will execute after render init.
	if (Enable) {
		HMD->SetAdaptiveQualityState(true, strategyFlags);
	}
	else {
		HMD->SetAdaptiveQualityState(false, 0);
	}

	if (HMD->IsRenderInitialized()) {
		return FWaveVRAPIWrapper::GetInstance()->EnableAdaptiveQuality(Enable, strategyFlags);
	}
	else {
		LOGD(LogWaveVRBPFunLib, "Enable AdaptiveQuality before render init, the effect will active after render init!");
	}

	return true;
}

// Set AMC mode.
void UWaveVRBlueprintFunctionLibrary::SetAMCMode(EAMCMode mode) {
	WVR_AMCMode amcMode = (WVR_AMCMode)mode;

	// TODO If EAMCMode enum has Force_PMC, these comment lines should be uncommented.
	//auto CVarTripleDepthBuffer = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("wvr.tripleDepthBuffer"));
	//bool hasTripleDepthBuffer = (bool)CVarTripleDepthBuffer->GetValueOnAnyThread();
	//if (!hasTripleDepthBuffer) {
	//	if (amcMode == WVR_AMCMode_Force_PMC) {
	//		amcMode = WVR_AMCMode_Off;
	//	}
	//}
	WVR()->SetAMCMode(amcMode);
}

// Get AMC mode.
EAMCMode UWaveVRBlueprintFunctionLibrary::GetAMCMode() {
	return (EAMCMode)WVR()->GetAMCMode();
}

// Get AMC status.
EAMCStatus UWaveVRBlueprintFunctionLibrary::GetAMCStatus() {
	return (EAMCStatus)WVR()->GetAMCStatus();
}


void UWaveVRBlueprintFunctionLibrary::SetSplashParam(UTexture2D* InSplashTexture, FLinearColor BackGroundColor, float ScaleFactor, FVector2D Shift, bool EnableAutoLoading) {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return;
	if (InSplashTexture == nullptr) {
		LOGD(LogWaveVRBPFunLib, "SetSplashParam() Splash Texture is nullptr!");
	} else if (!InSplashTexture->Resource) {
		LOGD(LogWaveVRBPFunLib, "SetSplashParam() Resource is not valid!");
	}
	if (HMD->GetSplashScreen().IsValid()) {
		HMD->GetSplashScreen()->SetSplashParam(InSplashTexture, BackGroundColor, ScaleFactor, Shift, EnableAutoLoading);
	}
}

void UWaveVRBlueprintFunctionLibrary::ShowSplashScreen() {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return;
	if (HMD->GetSplashScreen().IsValid()) {
		HMD->GetSplashScreen()->Show();
	}
}

void UWaveVRBlueprintFunctionLibrary::HideSplashScreen() {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD == nullptr) return;
	if (HMD->GetSplashScreen().IsValid()) {
		HMD->GetSplashScreen()->Hide();
	}
}

int UWaveVRBlueprintFunctionLibrary::GetWaveVRRuntimeVersion() {
	return FWaveVRAPIWrapper::GetInstance()->GetWaveRuntimeVersion();
}

bool UWaveVRBlueprintFunctionLibrary::IsDirectPreview() {
#if WITH_EDITOR
	return WaveVRDirectPreview::IsDirectPreview();
#endif
	return false;
}

void UWaveVRBlueprintFunctionLibrary::SetParameters(EWVR_DeviceType type, FString pchValue) {
	if (!IsRegularDevice(type))
		return;

	char* chValue = TCHAR_TO_ANSI(*pchValue);
	FWaveVRAPIWrapper::GetInstance()->SetParameters(static_cast<WVR_DeviceType>(type), chValue);
}

#pragma region ScreenshotMode
bool UWaveVRBlueprintFunctionLibrary::ScreenshotMode(EScreenshotMode ScreenshotMode) {

	ScreenshotImpl* screenshotMode = ScreenshotImpl::GetInstance();

	bool ret = false;

	switch (ScreenshotMode) {
	case EScreenshotMode::DEFAULT:
		LOGI(LogWaveVRBPFunLib, "Oz Screenshot Mode: DEFAULT");
		break;
	case EScreenshotMode::RAW:
		LOGI(LogWaveVRBPFunLib, "Oz Screenshot Mode: RAW");
		break;
	case EScreenshotMode::DISTORTED:
		LOGI(LogWaveVRBPFunLib, "Oz Screenshot Mode: DISTORTED");
		break;
	default:
		break;
	}

	ret = screenshotMode->Screenshot(ScreenshotMode);
	return ret;
}

void UWaveVRBlueprintFunctionLibrary::GetScreenshotFileInfo(FString &ImageFileName, FString &ImagePath) {

	ScreenshotImpl* screenshotMode = ScreenshotImpl::GetInstance();

	ImageFileName = screenshotMode->ScreenshotFileName;
	ImagePath = screenshotMode->ScreenshotImagePath;

	LOGI(LogWaveVRBPFunLib	, "Oz (GetScreenshotFileInfo) imageFileName: %s, imagePath: %s", PLATFORM_CHAR(*ImageFileName), PLATFORM_CHAR(*ImagePath));
}
#pragma endregion

void UWaveVRBlueprintFunctionLibrary::SimulateCPULoading(int gameThreadLoading, int renderThreadLoading) {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr)
		HMD->SimulateCPULoading((unsigned int)gameThreadLoading, (unsigned int)renderThreadLoading);
}

void UWaveVRBlueprintFunctionLibrary::EnableLateUpdate(bool enable, bool doUpdateInGameThread, float predictTimeInGameThread) {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr)
		HMD->EnableLateUpdate(enable, doUpdateInGameThread, predictTimeInGameThread);
}

bool UWaveVRBlueprintFunctionLibrary::IsLateUpdateEnabled() {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	return HMD != nullptr && HMD->DoesSupportLateUpdate();
}

bool UWaveVRBlueprintFunctionLibrary::SetControllerPoseMode(EWVR_Hand Type, EWVR_ControllerPoseMode Mode) {
	LOGD(LogWaveVRBPFunLib, "SetControllerPoseMode with Type(%d) Mode (%d)", (uint8_t)Type, (uint8_t)Mode);
	bool ret = false;
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		WVR()->SetControllerPoseMode(static_cast<WVR_DeviceType>(Type), static_cast<WVR_ControllerPoseMode>(Mode));
		HMD->UpdatePoseModeAndBroadcast(static_cast<WVR_DeviceType>(Type));
		ret = true;
	}
	return ret;
}

bool UWaveVRBlueprintFunctionLibrary::GetControllerPoseMode(EWVR_Hand Type, EWVR_ControllerPoseMode& OutMode) {
	//LOGD(LogWaveVRBPFunLib, "GetControllerPoseMode with Type(%d)", (uint8_t)Type);
	bool ret = false;
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		int index = static_cast<uint8>(Type)-2;
		OutMode = static_cast<EWVR_ControllerPoseMode>(HMD->GetCachedPoseModes()[index]);
		ret = true;
	}
	return ret;
}

bool UWaveVRBlueprintFunctionLibrary::GetControllerPoseModeOffset(EWVR_Hand Type, EWVR_ControllerPoseMode Mode, FTransform& OutTransform) {
	//LOGD(LogWaveVRBPFunLib, "GetControllerPoseModeOffset with Type(%d) Mode (%d)", (uint8_t)Type, (uint8_t)Mode);
	bool ret = false;
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		int index = (static_cast<uint8>(Type)-2)*4 + static_cast<uint8>(Mode);
		OutTransform = HMD->GetPoseModeOffsets()[index];
		ret = true;
	}
	return ret;
}

float UWaveVRBlueprintFunctionLibrary::GetWorldToMetersScale() {
	float ret = 0;
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		ret = HMD->GetWorldToMetersScale();
	}
	return ret;
}

#pragma region
bool UWaveVRBlueprintFunctionLibrary::IsDeviceTableStatic(EWVR_DeviceType type) {
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		return HMD->IsDeviceTableStatic(static_cast<WVR_DeviceType>(type));
	}
	return false;
}
#pragma endregion Simultaneous Interaction

#pragma region
void UWaveVRBlueprintFunctionLibrary::SetFocusController(EWVR_DeviceType type)
{
	LOGD(LogWaveVRBPFunLib, "SetFocusController() %d", (uint8)type);
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD)
		HMD->SetFocusedController(static_cast<WVR_DeviceType>(type));
}
EWVR_DeviceType UWaveVRBlueprintFunctionLibrary::GetFocusController()
{
	WVR_DeviceType ret = WVR_DeviceType_Invalid;
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD)
		ret = HMD->GetFocusedController();

	return GetDeviceType(ret);
}
EWVRInteractionMode UWaveVRBlueprintFunctionLibrary::GetInteractionMode()
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		WVR_InteractionMode mode = HMD->GetInteractionMode();
		switch (mode)
		{
		case WVR_InteractionMode::WVR_InteractionMode_Gaze:
			return EWVRInteractionMode::Gaze;
		case WVR_InteractionMode::WVR_InteractionMode_Controller:
			return EWVRInteractionMode::Controller;
		case WVR_InteractionMode::WVR_InteractionMode_Hand:
			return EWVRInteractionMode::Hand;
		default:
			break;
		}
	}

	return EWVRInteractionMode::Invalid;
}

bool UWaveVRBlueprintFunctionLibrary::SetGazeTriggerType(EWVRGazeTriggerType type)
{
	if (type == EWVRGazeTriggerType::Invalid)
		return false;

	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (!HMD)
		return false;

	return HMD->SetGazeTriggerType(static_cast<WVR_GazeTriggerType>(type));
}

EWVRGazeTriggerType UWaveVRBlueprintFunctionLibrary::GetGazeTriggerType()
{
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (!HMD)
		return EWVRGazeTriggerType::Invalid;

	WVR_GazeTriggerType type = HMD->GetGazeTriggerType();
	switch (type) {
	case WVR_GazeTriggerType::WVR_GazeTriggerType_Timeout:
		return EWVRGazeTriggerType::Timeout;
	case WVR_GazeTriggerType::WVR_GazeTriggerType_Button:
		return EWVRGazeTriggerType::Button;
	case WVR_GazeTriggerType::WVR_GazeTriggerType_TimeoutButton:
		return EWVRGazeTriggerType::TimeoutButton;
	default:
		break;
	}
	return EWVRGazeTriggerType::Invalid;
}
#pragma endregion Interaction Mode Manager

void UWaveVRBlueprintFunctionLibrary::TriggerVibration(
	EWVR_DeviceType controller, int32 durationMilliSec, EWVR_Intensity intensity)
{
	if (intensity == EWVR_Intensity::Invalid || controller == EWVR_DeviceType::DeviceType_Invalid) { return; }

	LOGD(LogWaveVRBPFunLib, "TriggerVibration controller %d, duration %d milliseconds, intensity %d",
		(uint8_t)controller, durationMilliSec, (uint8_t)intensity);

	durationMilliSec = durationMilliSec > kVibrationDurationMax ? kVibrationDurationMax : durationMilliSec;
	durationMilliSec = durationMilliSec < 0 ? kVibrationDurationDefault : durationMilliSec;
	uint32_t durationMicroSec = durationMilliSec * 1000;

	FWaveVRAPIWrapper::GetInstance()->TriggerVibration(
		static_cast<WVR_DeviceType>(controller),
		WVR_InputId::WVR_InputId_Alias1_Touchpad,
		durationMicroSec,
		1,	// frequency = 1 time
		static_cast<WVR_Intensity>(intensity)
	);
}
void UWaveVRBlueprintFunctionLibrary::TriggerVibrationScale(
	EWVR_DeviceType controller, int32 durationMilliSec, float amplitude)
{
	if (controller == EWVR_DeviceType::DeviceType_Invalid) { return; }

	LOGD(LogWaveVRBPFunLib, "TriggerVibrationScale controller %d, duration %d milliseconds, amplitude %f",
		(uint8_t)controller, durationMilliSec, amplitude);

	if (amplitude <= 0 || amplitude > 1) { amplitude = kVibrationAmplitudeDefault; }

	durationMilliSec = durationMilliSec > kVibrationDurationMax ? kVibrationDurationMax : durationMilliSec;
	durationMilliSec = durationMilliSec < 0 ? kVibrationDurationDefault : durationMilliSec;
	uint32_t durationMicroSec = durationMilliSec * 1000;

	FWaveVRAPIWrapper::GetInstance()->TriggerVibrationScale(
		static_cast<WVR_DeviceType>(controller),
		WVR_InputId::WVR_InputId_Alias1_Touchpad,
		durationMicroSec,
		1,	// frequency = 1 time
		amplitude
	);
}

void UWaveVRBlueprintFunctionLibrary::SetFrameSharpnessEnhancementLevel(float level)
{
	LOGD(LogWaveVRBPFunLib, "SetFrameSharpnessEnhancementLevel as %f", level);
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	if (HMD != nullptr) {
		HMD->SetFrameSharpnessEnhancementLevel(level);
	}
}

bool UWaveVRBlueprintFunctionLibrary::ShowPassthroughOverlay(bool show, bool delaySubmit, bool showIndicator) {
	LOGD(LogWaveVRBPFunLib, "ShowPassthroughOverlay(show, delaySubmit, showIndicator) is (%u, %u, %u)", show, delaySubmit, showIndicator);
	return WVR()->ShowPassthroughOverlay(show, delaySubmit, showIndicator);
}

bool UWaveVRBlueprintFunctionLibrary::IsPassthroughOverlayVisible() {
	return WVR()->IsPassthroughOverlayVisible();
}

bool UWaveVRBlueprintFunctionLibrary::SetPassthroughOverlayAlpha(const float alpha) {

	WVR_Result ResType = WVR_Result::WVR_Error_SystemInvalid;
	ResType = WVR()->SetPassthroughOverlayAlpha(alpha);
	bool result = false;

	switch (ResType)
	{
		case WVR_Result::WVR_Success:
			result = true;
			break;
		case WVR_Result::WVR_Error_RuntimeVersionNotSupport:
			LOGW(LogWaveVRBPFunLib, "SetPassthroughOverlayAlpha failed, WVR_Error_RuntimeVersionNotSupport. This feature is supported from Runtime version 7 or higher.");
			break;
		case WVR_Result::WVR_Error_FeatureNotSupport:
			LOGW(LogWaveVRBPFunLib, "SetPassthroughOverlayAlpha failed, WVR_Error_FeatureNotSupport. This feature is not supported on this device.");
			break;
		default:
			LOGW(LogWaveVRBPFunLib, "SetPassthroughOverlayAlpha failed, ResType %d is not expected.", (uint8)ResType);
			break;
	}
	return result;
}

bool UWaveVRBlueprintFunctionLibrary::ShowPassthroughUnderlay(bool show) {

	WVR_Result ResType = WVR_Result::WVR_Error_SystemInvalid;
	ResType = WVR()->ShowPassthroughUnderlay(show);
	bool result = false;

	switch (ResType)
	{
		case WVR_Result::WVR_Success:
			result = true;
			break;
		case WVR_Result::WVR_Error_RuntimeVersionNotSupport:
			LOGW(LogWaveVRBPFunLib, "ShowPassthroughUnderlay failed, WVR_Error_RuntimeVersionNotSupport. This feature is supported from Runtime version 7 or higher.");
			break;
		case WVR_Result::WVR_Error_FeatureNotSupport:
			LOGW(LogWaveVRBPFunLib, "ShowPassthroughUnderlay failed, WVR_Error_FeatureNotSupport. This feature is not supported on this device.");
			break;
		default:
			LOGW(LogWaveVRBPFunLib, "ShowPassthroughUnderlay failed, ResType %d is not expected.", (uint8)ResType);
			break;
	}
	return result;
}
