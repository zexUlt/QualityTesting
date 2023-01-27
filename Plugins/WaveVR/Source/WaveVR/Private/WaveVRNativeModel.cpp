// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveVRNativeModel.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "WaveVRControllerModel.h"
#include "WaveVROEMConfig.h"
#include "WaveVRController.h"
#include "WaveVREventCommon.h"
#include "WaveVRHMD.h"

#include "WaveVRUtils.h"
using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(NativeCtrlModel, Log, All);

static void DebugMatrixNM(const FMatrix& m) {
	LOGD(NativeCtrlModel,
		"/ %6f, %6f, %6f, %6f \\\n"
		"| %6f, %6f, %6f, %6f |\n"
		"| %6f, %6f, %6f, %6f |\n"
		"\\ %6f, %6f, %6f, %6f /\n",
		m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3],
		m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3],
		m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
		m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]
	);
}

// Sets default values
AWaveVRNativeModel::AWaveVRNativeModel() :
	alwaysShow(false),
	isButtonAnimation(false),
	ctrl(nullptr),
	controllerInterface(nullptr),
	controllerOutlineInterface(nullptr),
	batteryInterface(nullptr),
	batteryDynamic(nullptr),
	touchEffectDynamic(nullptr),
	blueEffectTex(nullptr),
	inputMappingPairUpdatedFrame(0),
	hasBatteryMesh(false),
	showBattery(false),
	intervalCount(300),
	printCount(0),
	isTouchPress(false),
	printable(false),
	preBatteryLevel(-1),
	PoseMode(0),
	assembleControllerDone(false),
	componentIsCompleted(false)
{
	// ControllerMat
	FString pControllerMat = TEXT("Material'/WaveVR/Materials/NControllerMat.NControllerMat'");
	controllerInterface = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *pControllerMat));

	// ControllerOutlineMat
	FString pControllerOutlineMat = TEXT("Material'/WaveVR/Materials/NControllerOutlineMat.NControllerOutlineMat'");
	controllerOutlineInterface = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *pControllerOutlineMat));

	// BatteryMat
	FString pBatteryMat = TEXT("Material'/WaveVR/Materials/NBattery.NBattery'");
	batteryInterface = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *pBatteryMat));

	// BlueEffectTex
	FString pBlueEffectTex = TEXT("Texture2D'/WaveVR/Textures/BlueDot.BlueDot'");
	blueEffectTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *pBlueEffectTex));

	if (NativeModelParameter::GetSpawnFlag()) {
		deviceTypeWVR = NativeModelParameter::GetSpawnDeviceType();
		deviceType = (EWVR_DeviceType)deviceTypeWVR;
		deviceTypeInt = (int)deviceType;
		alwaysShow = NativeModelParameter::GetAlwaysShow();
		hideWhenIdle = NativeModelParameter::GetHideWhenIdle();
		LOGD(NativeCtrlModel, "Device(%d), Spawn AWaveVRNativeControllerModel, alwaysShow(%d), hideWhenIdle(%d)", deviceTypeInt, alwaysShow, (uint8)hideWhenIdle);

		FName cn = (deviceType == EWVR_DeviceType::DeviceType_Controller_Left) ? FName("LeftControllerRoot") : FName("RightControllerRoot");

		RootComponent = CreateDefaultSubobject<USceneComponent>(cn);
		PoseModeScene = CreateDefaultSubobject<USceneComponent>(TEXT("PoseModeScene"));
		SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
		//SceneComponent->SetRelativeRotation(FRotator(180, 90, 90));  // Black history
		PoseModeScene->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		SceneComponent->AttachToComponent(PoseModeScene, FAttachmentTransformRules::KeepRelativeTransform);

		AssembleController();
	}

	EmitterTransform = FTransform::Identity;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

const TArray<FButtonFeature>& AWaveVRNativeModel::GetButtonFeatureTable()
{
	static bool initialized = false;
	static TArray<FButtonFeature> table;
	if (!initialized) {
		AddButtonMapping(table, EWVR_InputId::A, FString(TEXT("__CM__ButtonA")), false);
		AddButtonMapping(table, EWVR_InputId::B, FString(TEXT("__CM__ButtonB")), false);
		AddButtonMapping(table, EWVR_InputId::X, FString(TEXT("__CM__ButtonX")), false);
		AddButtonMapping(table, EWVR_InputId::Y, FString(TEXT("__CM__ButtonY")), false);
		AddButtonMapping(table, EWVR_InputId::Bumper, FString(TEXT("__CM__DigitalTriggerKey")), false);
		AddButtonMapping(table, EWVR_InputId::Grip, FString(TEXT("__CM__Grip")), false);
		AddButtonMapping(table, EWVR_InputId::Menu, FString(TEXT("__CM__AppButton")), false);
		AddButtonMapping(table, EWVR_InputId::Thumbstick, FString(TEXT("__CM__Thumbstick")), false);
		AddButtonMapping(table, EWVR_InputId::Touchpad, FString(TEXT("__CM__Touchpad")), false);
		AddButtonMapping(table, EWVR_InputId::Trigger, FString(TEXT("__CM__TriggerKey")), false);
		AddButtonMapping(table, EWVR_InputId::Volume_Down, FString(TEXT("__CM__VolumeKey")), false);
		AddButtonMapping(table, EWVR_InputId::Volume_Up, FString(TEXT("__CM__VolumeKey")), false);
		AddButtonMapping(table, EWVR_InputId::Bumper, FString(TEXT("__CM__BumperKey")), false);

		// Touchpad has two component name...
		AddButtonMapping(table, EWVR_InputId::Touchpad, FString(TEXT("__CM__Touchpad_Touch")), true);
		initialized = true;
	}

	return table;
}

const TArray<FButtonFeature>& AWaveVRNativeModel::GetButtonFeatureTableAnime()
{
	static bool initialized = false;
	static TArray<FButtonFeature> table;
	if (!initialized) {
		AddButtonAnimation(table, EWVR_InputId::A, FString(TEXT("__CM__ButtonA")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::B, FString(TEXT("__CM__ButtonB")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::X, FString(TEXT("__CM__ButtonX")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Y, FString(TEXT("__CM__ButtonY")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Bumper, FString(TEXT("__CM__DigitalTriggerKey")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Menu, FString(TEXT("__CM__AppButton")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Volume_Down, FString(TEXT("__CM__VolumeKey")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Volume_Up, FString(TEXT("__CM__VolumeKey")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Bumper, FString(TEXT("__CM__BumperKey")), AnimationType::AnimBinary);
		AddButtonAnimation(table, EWVR_InputId::Thumbstick, FString(TEXT("__CM__Thumbstick")), AnimationType::AnimBinary);

		AddButtonAnimation(table, EWVR_InputId::Grip, FString(TEXT("__CM__Grip")), AnimationType::AnimTravel1D);
		AddButtonAnimation(table, EWVR_InputId::Trigger, FString(TEXT("__CM__TriggerKey")), AnimationType::AnimTravel1D);

		// Touchpad has two component name...
		AddButtonAnimation(table, EWVR_InputId::Thumbstick, FString(TEXT("__CM__Thumbstick")), AnimationType::AnimThumbstick);
		initialized = true;
	}

	return table;
}

void AWaveVRNativeModel::AddButtonAnimation(TArray<FButtonFeature>& table, EWVR_InputId btn, FString name, AnimationType type)
{
	int i = table.Add(FButtonFeature(btn, name, name.ToLower()));
	auto& btnFeature = table[i];

	if (type == AnimationType::AnimBinary) {
		LOGD(NativeCtrlModel, "Add %d %s to binaryMap", (uint8)btn, PLATFORM_CHAR(*name));
		btnFeature.animBinary = true;
	} else if (type == AnimationType::AnimTravel1D) {
		LOGD(NativeCtrlModel, "Add %d %s to travel1DMap", (uint8)btn, PLATFORM_CHAR(*name));
		btnFeature.animTravel1D = true;
	} else if (type == AnimationType::AnimThumbstick) {
		LOGD(NativeCtrlModel, "Add %d %s to thumbstickMap", (uint8)btn, PLATFORM_CHAR(*name));
		btnFeature.animThumbstick = true;
	}
}

void AWaveVRNativeModel::AddButtonMapping(TArray<FButtonFeature>& table, EWVR_InputId btn, FString name, bool isTouch)
{
	int i = table.Add(FButtonFeature(btn, name, name.ToLower()));
	auto& btnFeature = table[i];

	if (isTouch) {
		LOGD(NativeCtrlModel, "Add %d %s to touchMap", (uint8)btn, PLATFORM_CHAR(*name));
		btnFeature.btnTouch = true;
	} else {
		LOGD(NativeCtrlModel, "Add %d %s to pressMap", (uint8)btn, PLATFORM_CHAR(*name));
		btnFeature.btnPress = true;
	}
}

void AWaveVRNativeModel::AssembleController()
{
	EmitterTransform = FTransform::Identity;
	WVR_Result result = FWaveVRAPIWrapper::GetInstance()->GetCurrentControllerModel(deviceTypeWVR, &ctrl, false);

	if (result != WVR_Result::WVR_Success) {
		LOGE(NativeCtrlModel, "Device(%d), Get current controller model from native failed! error code = %d", deviceTypeInt, (uint8)result);
		return;
	}

	if (ctrl == nullptr) {
		LOGE(NativeCtrlModel, "Device(%d), Get current controller model is nullptr!", deviceTypeInt);
		return;
	}

	ParseMesh();

	for (int32 i = 0; i < componentCount; i++) {
		// Create COMPONENT
		FMeshComponent& comp = components[i];
		LOGD(NativeCtrlModel, "Device(%d), CreateProceduralMeshComponent(%s) defaultDraw(%d)", deviceTypeInt, PLATFORM_CHAR(*comp.name), (*ctrl).compInfos.table[i].defaultDraw);
		UProceduralMeshComponent* procMeshComp = CreateDefaultSubobject<UProceduralMeshComponent>(FName(*comp.name));

		procMeshComp->bUseAsyncCooking = true; // New in UE 4.17, multi-threaded PhysX cooking.
		if (FName(*comp.name) == FName(TEXT("__CM__Battery"))) {
			LOGD(NativeCtrlModel, "Device(%d), Battery is found", deviceTypeInt);
			hasBatteryMesh = true;
			batteryMesh = procMeshComp;
		}

		procMeshComp->SetupAttachment(SceneComponent);

		procMeshComp->SetRelativeTransform(FTransform(comp.transform));
		procMeshComp->SetVisibility((*ctrl).compInfos.table[i].defaultDraw);

		comp.mesh = procMeshComp;

		const auto& featureTableAnime = GetButtonFeatureTableAnime();
		for (int j = 0; j < featureTableAnime.Num(); j++) {
			if (comp.name != featureTableAnime[j].name) continue;
			if (!featureTableAnime[j].animBinary) continue;
			LOGD(NativeCtrlModel, "Device(%d), CreateProceduralMeshComponent(%s_o)", deviceTypeInt, PLATFORM_CHAR(*comp.name));
			FString outlineName = comp.name;
			outlineName.Append(TEXT("_o"));
			UProceduralMeshComponent* mesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName(*outlineName));
			mesh->bUseAsyncCooking = true; // New in UE 4.17, multi-threaded PhysX cooking.
			mesh->SetupAttachment(SceneComponent);
			mesh->SetRelativeTransform(FTransform(comp.transform));
			mesh->SetVisibility(false);
			comp.meshOutline = mesh;
			break;  // one component only need one outline
		}
	}

	assembleControllerDone = true;
}

// Called when the game starts or when spawned
void AWaveVRNativeModel::BeginPlay()
{
	Super::BeginPlay();
	UWaveVREventCommon::OnControllerPoseModeChangedNative.AddDynamic(this, &AWaveVRNativeModel::OnControllerPoseModeChangedHandling);
	Frame_DeviceTypeBeenSet = GFrameCounter + 1;
}

void AWaveVRNativeModel::OnControllerPoseModeChangedHandling(uint8 Device, uint8 Mode, FTransform Transform)
{
	LOGD(NativeCtrlModel, "Device(%d), AWaveVRNativeModel OnControllerPoseModeChangedHandling GFrameCounter(%llu)", deviceTypeInt, GFrameCounter);
	if (static_cast<uint8_t>(deviceType) == Device) {
		//DelayedSetRelativeTransform 2 frames later because controller poses will apply new PoseMode 2 frames later.
		SetRelativeTransformFrame = GFrameCounter + 2;
		PoseModeTransform = Transform;
		PoseMode = Mode;
	}
}

void AWaveVRNativeModel::DelayedSetRelativeTransform()
{
	LOGD(NativeCtrlModel, "Device(%d), DelayedSetRelativeTransform GFrameCounter(%llu) Mode(%d)", deviceTypeInt, GFrameCounter, PoseMode);
	if (PoseMode != 0) {
		auto rtl = PoseModeScene->GetRelativeTransform().GetLocation();
		// PoseMode is not RAW mode, update transform
		LOGD(NativeCtrlModel, "Device(%d), PoseMode is not RAW mode, update transform, origin location(%f %f %f)", deviceTypeInt, rtl.X, rtl.Y, rtl.Z);

		PoseModeScene->SetRelativeTransform(PoseModeTransform);
		auto rtl2 = PoseModeTransform.GetLocation();

		LOGD(NativeCtrlModel, "Device(%d), PoseModeTransform after change location(%f %f %f)", deviceTypeInt, rtl2.X, rtl2.Y, rtl2.Z);
	}
	else {
		PoseModeScene->SetRelativeTransform(EmitterTransform);
	}
}

// This is called when SpawnActor
void AWaveVRNativeModel::PostActorCreated()
{
	Super::PostActorCreated();
	if (componentCount > 0) CompleteMeshComponent();
	if (ctrl != nullptr) {
		FWaveVRAPIWrapper::GetInstance()->ReleaseControllerModel(&ctrl);
		ctrl = nullptr;
	}
}

// This is called when LoadMap/AddToWorld(Editor)
void AWaveVRNativeModel::PostLoad()
{
	Super::PostLoad();
	if (componentCount > 0) CompleteMeshComponent();
	if (ctrl != nullptr) {
		FWaveVRAPIWrapper::GetInstance()->ReleaseControllerModel(&ctrl);
		ctrl = nullptr;
	}
}

// Called every frame
void AWaveVRNativeModel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
	//testBatteryLevel = fmod((testBatteryLevel + 0.1f + DeltaTime / 4.0f), 1.2f) - 0.1f;

	if (printCount++ > 200) {
		printCount = 0;
		printable = true;
	}
	else {
		printable = false;
	}

	if (GFrameCounter == SetRelativeTransformFrame)
		DelayedSetRelativeTransform();

	if (!assembleControllerDone)
		return;

	//DeviceType will been set after BeginPlay. So we need to delay one frame to Update the correct hand.
	if (GFrameCounter == Frame_DeviceTypeBeenSet) {
		if (HMD != nullptr) {
			HMD->UpdatePoseModeAndBroadcast(deviceTypeWVR);
		}
	}

	// Update if input mapping pairs are changed.
	{
		uint64 currentUpdatedFrame = HMD->mappingTableHash[deviceTypeWVR];
		if (inputMappingPairUpdatedFrame < currentUpdatedFrame) {
			LOGD(NativeCtrlModel, "Device(%d), InputMappingPair is updated (%llu <= %llu)", 
				deviceTypeInt, inputMappingPairUpdatedFrame, currentUpdatedFrame);
			BuildEffectCaches();
		}
	}

	if (hasBatteryMesh) {
		if (intervalCount++ > 200) {
			intervalCount = 0;

			UpdateBattery();
		}
	}

	if (isButtonAnimation) {
		AnimateButtonPress();
		AnimateTravel1D();
		AnimateThumbstick();
	} else {
		PollingButtonPressState();
		PollingTouchState();
	}
	bool showModel = IsModelShow();
	RootComponent->SetVisibility(showModel);
	RootComponent->SetHiddenInGame(!showModel);

	if (printable) {
		FTransform t = PoseModeScene->GetRelativeTransform();
		FVector v = t.GetLocation();
		FRotator r = t.GetRotation().Rotator();

		LOGD(NativeCtrlModel, "Device(%d), Model(show=%d) PoseModeScene(hidden=%d) loc(x=%f y=%f z=%f) rot(r=%f p=%f y=%f)!", deviceTypeInt, showModel, PoseModeScene->bHiddenInGame, v.X, v.Y, v.Z, r.Roll, r.Pitch, r.Yaw);
	}
}

bool AWaveVRNativeModel::IsModelShow()
{
	bool controller_mode = (UWaveVRBlueprintFunctionLibrary::GetInteractionMode() == EWVRInteractionMode::Controller);
	bool valid_pose = UWaveVRBlueprintFunctionLibrary::GetDevicePose(devicePosition, deviceRotation, deviceType);
	bool focusCapturedBySystem = UWaveVRBlueprintFunctionLibrary::IsInputFocusCapturedBySystem();
	bool tableStatic = UWaveVRBlueprintFunctionLibrary::IsDeviceTableStatic(deviceType);
	bool hideStatic = hideWhenIdle && tableStatic;

	bool interactable = (alwaysShow || controller_mode)
		&& valid_pose
		&& (!focusCapturedBySystem)
		&& (!hideStatic);

	if (printable)
	{
		LOGD(NativeCtrlModel,
			"IsModelShow() deviceType %d, interactable %d, alwaysShow %d, controller_mode %d, focusCapturedBySystem %d, valid_pose %d, hideStatic %d, hideWhenIdle %d, tableStatic %d",
			(uint8)deviceType,
			(uint8)interactable,
			(uint8)alwaysShow,
			(uint8)controller_mode,
			(uint8)focusCapturedBySystem,
			(uint8)valid_pose,
			(uint8)hideStatic,
			(uint8)hideWhenIdle,
			(uint8)tableStatic);
	}

	return interactable;
}

void AWaveVRNativeModel::CollectTouchEffectMesh()
{
	// Create touch dynamic texture
	if (blueEffectTex == nullptr) {
		LOGE(NativeCtrlModel, "Device(%d), Can't find BlueDot.BlueDot for touch", deviceTypeInt);
		touchEffectDynamic = nullptr;
	} else {
		if (touchEffectDynamic != nullptr) {
			LOGW(NativeCtrlModel, "Device(%d), \"touchEffectDynamic\" is already created.", deviceTypeInt);
		} else {
			touchEffectDynamic = UMaterialInstanceDynamic::Create(controllerInterface, this);
			touchEffectDynamic->SetTextureParameterValue(FName(TEXT("Tex")), blueEffectTex);
		}
	}
	touchEffectMap.Empty();
	touchState.Empty();

	const auto& featureTable = GetButtonFeatureTable();

	// Search __CM__TouchPad_Touch mesh
	for (uint8 b = 0; b <= (uint8)EWVR_InputId::Thumbstick; b++)
	{
		EWVR_InputId srcBtn = UWaveVRBlueprintFunctionLibrary::GetInputMappingPair(deviceType, (EWVR_InputId)b);
		if (srcBtn == EWVR_InputId::NoUse) continue;

		int N = featureTable.Num();
		for (int i = 0; i < N; i++) {
			const auto& btnFeature = featureTable[i];
			if (btnFeature.inputId != srcBtn || !btnFeature.btnTouch) continue;

			int* ptrCompId = componentTable.Find(btnFeature.lowerName);
			if (ptrCompId == nullptr) continue;
			auto& comp = components[*ptrCompId];

			LOGD(NativeCtrlModel, "Device(%d), Desc ButtonId %d has touch effect (%s), hidden = true", deviceTypeInt, (uint8)b, PLATFORM_CHAR(*btnFeature.name));

			NButtonEffectInfo s;
			s.btn = (EWVR_InputId)b;
			s.meshName = FName(btnFeature.name);
			s.lowerMeshName = btnFeature.lowerName;
			s.MeshComp = comp.mesh;

			for (int k = 0; k < s.MeshComp->GetNumMaterials(); k++) {
				if (s.MeshComp != nullptr && touchEffectDynamic != nullptr)
					s.MeshComp->SetMaterial(k, touchEffectDynamic);
			}
			s.MeshComp->bHiddenInGame = true;

			touchEffectMap.Add(s);
			touchState.Add(false);
		}
	}

	// Get touch info from native
	SetTouchpadPlaneInfo();
}

void AWaveVRNativeModel::PollingTouchState()
{
	for (int i = 0; i < touchEffectMap.Num(); i++) {
		NButtonEffectInfo s = touchEffectMap[i];
		if (!s.meshName.IsEqual(FName(TEXT("__CM__Touchpad_Touch"))))
			continue;

		if (!touchpadPlaneInfo.valid) {
			// TODO: can't get touch info from native
			LOGE(NativeCtrlModel, "Device(%d), can't get touch info from native", deviceTypeInt);
			return;
		}

		if (!isTouchPress) {
			if (UWaveVRBlueprintFunctionLibrary::IsButtonTouched(deviceType, static_cast<EWVR_TouchId>(s.btn))) {
				s.MeshComp->SetHiddenInGame(false);

				//LOGE(NativeCtrlModel, "Device(%d), touched", deviceTypeInt);
				FVector2D axis = UWaveVRController::GetControllerAxis(deviceType, static_cast<EWVR_TouchId>(s.btn));
				const auto& tpi = touchpadPlaneInfo;
				FVector moveInBtnSpace = FVector(axis.Y * tpi.radius, axis.X * tpi.radius, tpi.floatingDistance); // (forward, right, up)
				FVector moveInCtrlSpace = tpi.matrix.TransformPosition(moveInBtnSpace);

				s.MeshComp->SetRelativeLocation(moveInCtrlSpace);

				if (printable) {
					LOGD(NativeCtrlModel, "Device(%d), get ButtonId %d axis (%6f, %6f) from native",
						deviceTypeInt, (uint8)s.btn, axis.X, axis.Y);
					LOGD(NativeCtrlModel, "Device(%d), moveInCtrlSpace(%6f, %6f, %6f)",
						deviceTypeInt, moveInCtrlSpace.X, moveInCtrlSpace.Y, moveInCtrlSpace.Z);
				}
			} else {
				s.MeshComp->SetHiddenInGame(true);
				s.MeshComp->SetRelativeLocation(touchpadPlaneInfo.center);
			}
		} else {
			s.MeshComp->SetHiddenInGame(true);
			s.MeshComp->SetRelativeLocation(touchpadPlaneInfo.center);
		}
	}
}

void AWaveVRNativeModel::SetTouchpadPlaneInfo() {
	auto& tpi = touchpadPlaneInfo;
	if (ctrl != nullptr) {
		const auto& plane = (*ctrl).touchpadPlane;
		// axis don't need meter to world scale.
		FVector r = FromGLToUnreal(plane.u, 1);  // right = u = gl.x = ul.y
		FVector u = FromGLToUnreal(plane.v, 1);  // up = v = gl.y = ul.z
		FVector f = FromGLToUnreal(plane.w, 1);  // forward = w = gl.z = ul.x
		FVector c = FromGLToUnreal(plane.center, 100);  // translate
		tpi.matrix = FMatrix(f, r, u, c);  // arranged in Unreal's xyz direction
		tpi.center = c;
		tpi.floatingDistance = plane.floatingDistance * 100;
		tpi.radius = plane.radius * 100;
		tpi.valid = plane.valid;
	}
	LOGD(NativeCtrlModel, "Device(%d), TouchPadInfo()--Floating(%f), Raidus(%f), valid(%d), Matrix:",
		deviceTypeInt, tpi.floatingDistance, tpi.radius, tpi.valid);
	DebugMatrixNM(tpi.matrix);
}

void AWaveVRNativeModel::CollectButtonEffectMesh()
{
	pressEffectMap.Empty();
	pressBtnState.Empty();

	const auto& featureTable = GetButtonFeatureTable();

	// TODO change to EWVR_InputId::Max?
	for (uint8 b = 0; b <= (uint8)EWVR_InputId::Thumbstick; b++)
	{
		EWVR_InputId srcBtn = UWaveVRBlueprintFunctionLibrary::GetInputMappingPair(deviceType, (EWVR_InputId)b);
		if (srcBtn == EWVR_InputId::NoUse) continue;

		int N = featureTable.Num();
		for (int i = 0; i < N; i++) {
			const auto& btnFeature = featureTable[i];
			if (btnFeature.inputId != srcBtn || !btnFeature.btnPress) continue;

			int* ptrCompId = componentTable.Find(btnFeature.lowerName);
			if (ptrCompId == nullptr) continue;
			auto& comp = components[*ptrCompId];

			LOGD(NativeCtrlModel, "Device(%d), Desc ButtonId %d has effect (%s)", deviceTypeInt, (uint8)b, PLATFORM_CHAR(*btnFeature.name));

			NButtonEffectInfo s;
			s.btn = (EWVR_InputId)b;
			s.meshName = FName(btnFeature.name);
			s.lowerMeshName = btnFeature.lowerName;
			s.MeshComp = comp.mesh;
			//s.originMat = comp.mesh->GetMaterial(0);  // unknown usage
			s.meshMatInst = comp.material;

			pressEffectMap.Add(s);
			pressBtnState.Add(false);
		}
	}
}

void AWaveVRNativeModel::AnimateButtonPress()
{
	for (int i = 0; i < binaryObjectMap.Num(); i++) {
		BinaryButtonObject s = binaryObjectMap[i];
		bool currState = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(deviceType, s.btn);
		if (currState == binaryState[i]) continue;
		binaryState[i] = currState;

		if (s.MeshComp == nullptr) continue;

		auto& pos = currState ? s.pressPosition : s.originPosition;
		s.MeshComp->SetRelativeLocation(pos);

		if (currState) {
			LOGD(NativeCtrlModel, "Device(%d), Button(%d) mesh(%s) clicked (anim)", deviceTypeInt, (uint8)s.btn, PLATFORM_CHAR(*s.meshName.ToString()));
		} else {
			LOGD(NativeCtrlModel, "Device(%d), Button(%d) mesh(%s) released (anim)", deviceTypeInt, (uint8)s.btn, PLATFORM_CHAR(*s.meshName.ToString()));
		}

		if (s.MeshCompOutline == nullptr) continue;
		s.MeshCompOutline->SetVisibility(currState);
		//s.MeshCompOutline->SetRelativeLocation(pos);  // Not to presss down to keep more effect
	}
}

void AWaveVRNativeModel::AnimateTravel1D()
{
	for (int i = 0; i < travel1DObjectMap.Num(); i++) {
		Travel1DObject s = travel1DObjectMap[i];
		FVector2D axis = UWaveVRController::GetControllerAxis(deviceType, static_cast<EWVR_TouchId>(s.btn));

		float xVal = ((axis.X > 0) ? axis.X : -(axis.X));

		FRotator newRot = FMath::Lerp(s.originRotation, s.pressRotation, xVal);
		FVector newPos = FMath::Lerp(s.originPosition, s.pressPosition, xVal);

		if (s.MeshComp == nullptr) continue;
		s.MeshComp->SetRelativeLocation(newPos);
		s.MeshComp->SetRelativeRotation(newRot);

		if (printable) {
			LOGD(NativeCtrlModel, "Device(%d), get ButtonId %d axis (%f, %f) from native", deviceTypeInt, (uint8)s.btn, axis.X, axis.Y);
			LOGD(NativeCtrlModel, "Device(%d), p(%f) r(%f) y(%f)", deviceTypeInt, newRot.Pitch, newRot.Roll, newRot.Yaw);
		}
	}
}

void AWaveVRNativeModel::AnimateThumbstick()
{
	for (int i = 0; i < thumbstickObjectMap.Num(); i++) {
		ThumbstickObject s = thumbstickObjectMap[i];

		FVector2D axis = UWaveVRController::GetControllerAxis(deviceType, static_cast<EWVR_TouchId>(s.btn));

		FRotator newRot = s.maxRotation;
		newRot.Roll *= axis.X;
		newRot.Pitch *= -axis.Y;
		newRot += s.centerRotation;

		if (printable) {
			LOGD(NativeCtrlModel, "Device(%d), x(%f) y(%f)", deviceTypeInt, axis.X, axis.Y);
			LOGD(NativeCtrlModel, "Device(%d), p(%f) r(%f) y(%f)", deviceTypeInt, newRot.Pitch, newRot.Roll, newRot.Yaw);
		}

		if (s.MeshComp == nullptr) continue;
		s.MeshComp->SetRelativeRotation(newRot);

		if (s.MeshCompOutline == nullptr) continue;
		bool currState = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(deviceType, s.btn);
		s.MeshCompOutline->SetVisibility(currState);
		s.MeshCompOutline->SetRelativeRotation(newRot);
	}
}

void AWaveVRNativeModel::PollingButtonPressState() {
	for (int i = 0; i < pressEffectMap.Num(); i++) {
		const auto& s = pressEffectMap[i];

		bool currState = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(deviceType, s.btn);

		FString t = FName(TEXT("__CM__TouchPad")).ToString().ToLower();

		if (s.lowerMeshName.Equals(t))
			isTouchPress = currState;

		if (currState != pressBtnState[i]) {
			pressBtnState[i] = currState;

			if (currState) {
				LOGD(NativeCtrlModel, "Device(%d), Button(%d) mesh(%s) clicked", deviceTypeInt, (uint8)s.btn, PLATFORM_CHAR(*s.meshName.ToString()));
				if (s.meshMatInst->IsValidLowLevel())
					s.meshMatInst->SetTextureParameterValue(FName(TEXT("Tex")), blueEffectTex);
			} else {
				LOGD(NativeCtrlModel, "Device(%d), Button(%d) mesh(%s) released", deviceTypeInt, (uint8)s.btn, PLATFORM_CHAR(*s.meshName.ToString()));
				if (s.meshMatInst->IsValidLowLevel())
					s.meshMatInst->SetTextureParameterValue(FName(TEXT("Tex")), bodyTex);
			}
		}
	}
}

void AWaveVRNativeModel::UpdateBattery() {
	LOGD(NativeCtrlModel, "Device(%d), hasBattery = %d, showBattery = %d", deviceTypeInt, hasBatteryMesh, showBattery);

	if (batteryMesh == nullptr)
		return;

	if (!showBattery)
		return;
	float batteryPer = FWaveVRAPIWrapper::GetInstance()->GetDeviceBatteryPercentage(deviceTypeWVR);
	//batteryPer = testBatteryLevel;
	LOGD(NativeCtrlModel, "Device(%d), battery value = %f", deviceTypeInt, batteryPer);

	if (batteryPer < 0.0f || batteryPer > 1.0f)
		return;

	bool found = false;
	int foundIdx = 0;
	for (int i = 0; i < batteryLevelInfo.Num(); i++) {
		FNBatteryLevelInfo t = batteryLevelInfo[i];
		if (batteryPer >= (t.min / 100) && batteryPer < (t.max / 100)) {
			LOGD(NativeCtrlModel, "Device(%d), battery level = %d", deviceTypeInt, (i + 1));
			found = true;
			foundIdx = i;
			break;
		}
	}
	if (!found)
		foundIdx = batteryPer < 0.5f ? 0 : batteryLevelInfo.Num() - 1;
	if (preBatteryLevel != (foundIdx + 1)) {
		preBatteryLevel = (foundIdx + 1);
		LOGD(NativeCtrlModel, "Device(%d), battery level changed to %d", deviceTypeInt, preBatteryLevel);
		if (batteryDynamic->IsValidLowLevel()) {
			if (batteryLevelInfo[foundIdx].texture->IsValidLowLevel()) {
				batteryDynamic->SetTextureParameterValue(FName(TEXT("Tex")), batteryLevelInfo[foundIdx].texture);
			} else {
				LOGE(NativeCtrlModel, "batteryLevelInfo[%d].texture is not valid", foundIdx);
			}
		} else {
			LOGE(NativeCtrlModel, "batteryDynamic is not valid");
		}
		if (batteryMesh->bHiddenInGame)
			batteryMesh->bHiddenInGame = false;
	}
}

void AWaveVRNativeModel::SetBatteryInfo() {
	batteryLevelInfo.Empty();
	if (ctrl == nullptr) return;

	for (uint32_t i = 0; i < (*ctrl).batteryLevels.size; i++) {
		FNBatteryLevelInfo info;
		info.level = i;
		info.min = (*ctrl).batteryLevels.minLvTable[i];
		info.max = (*ctrl).batteryLevels.maxLvTable[i];
		LOGD(NativeCtrlModel, "batteryLevels(%u), min=%f, max=%f", i, info.min, info.max);

		UTexture2D* _texture = nullptr;
		_texture = UTexture2D::CreateTransient((*ctrl).batteryLevels.texTable[i].width, (*ctrl).batteryLevels.texTable[i].height, PF_R8G8B8A8);
		if (_texture)
		{
			//Lock it
			void* BatteryTextureData = _texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			//Copy pixel data
			FMemory::Memcpy(BatteryTextureData, (*ctrl).batteryLevels.texTable[i].bitmap, (*ctrl).batteryLevels.texTable[i].height*(*ctrl).batteryLevels.texTable[i].stride);
			//Unlock
			_texture->PlatformData->Mips[0].BulkData.Unlock();
			//Update texture
			_texture->UpdateResource();
		}
		info.texture = _texture;
		batteryLevelInfo.Add(info);
	}
	LOGD(NativeCtrlModel, "Device(%d), BatteryLevelInfo count: %d", deviceTypeInt, batteryLevelInfo.Num());
}

void AWaveVRNativeModel::CompleteMeshComponent()
{
	if (!(componentCount > 0)) return;
	if (!assembleControllerDone) return;
	if (componentIsCompleted) return;

	UpdateMaterial();
	SetBatteryInfo();

	if (batteryInterface == nullptr) {
		batteryDynamic = nullptr;
		LOGE(NativeCtrlModel, "Device(%d), Can't find NBattery.NBattery", deviceTypeInt);
	}
	else {
		batteryDynamic = UMaterialInstanceDynamic::Create(batteryInterface, this);
	}

	// CREATE MESH
	for (int32 i = 0; i < componentCount; i++) {
		FMeshComponent& comp = components[i];
		if (comp.mesh == nullptr) continue;
		LOGD(NativeCtrlModel, "Device(%d), CreateMeshSection %s %d", deviceTypeInt, PLATFORM_CHAR(*comp.name), i);
		comp.mesh->CreateMeshSection_LinearColor(0, comp.vertices, comp.indices, comp.normals, comp.uvs, comp.vertexColors, comp.tangents, /*true*/false);

		if (comp.meshOutline != nullptr) {
			LOGD(NativeCtrlModel, "Device(%d), CreateMeshSection %s_o %d", deviceTypeInt, PLATFORM_CHAR(*comp.name), i);
			comp.meshOutline->CreateMeshSection_LinearColor(0, comp.vertices, comp.indices, comp.normals, comp.uvs, comp.vertexColors, comp.tangents, /*true*/false);
		}

		comp.vertices.Empty();
		comp.indices.Empty();
		comp.normals.Empty();
		comp.uvs.Empty();
		comp.vertexColors.Empty();
		comp.tangents.Empty();

		// Enable collision data
		comp.mesh->ContainsPhysicsTriMeshData(/*true*/false);
		//for (int k = 0; k < comp.mesh->GetNumMaterials(); k++) {  // No. we only have one material
		comp.mesh->SetMaterial(0, comp.material);

		if (comp.meshOutline != nullptr && comp.materialOutline != nullptr)
			comp.meshOutline->SetMaterial(0, comp.materialOutline);

		if (FName(*comp.name) == FName(TEXT("__CM__Battery"))) {
			LOGD(NativeCtrlModel, "Device(%d), Battery is found", deviceTypeInt);

			if (batteryLevelInfo.Num() > 0) {
				showBattery = UWaveVROEMConfig::WaveVR_IsBatteryInfo();

				if (batteryLevelInfo[0].texture != nullptr) {
					LOGD(NativeCtrlModel, "Device(%d), Set up battery material", deviceTypeInt);
					if (batteryDynamic != nullptr) {
						batteryDynamic->SetTextureParameterValue(FName(TEXT("Tex")), batteryLevelInfo[0].texture);

						for (int k = 0; k < comp.mesh->GetNumMaterials(); k++) {
							comp.mesh->SetMaterial(k, batteryDynamic);
						}
					}
				}
			}
			else {
				LOGD(NativeCtrlModel, "Device(%d), battery info is not exist", deviceTypeInt);
			}
			comp.mesh->SetVisibility(true);
			comp.mesh->bHiddenInGame = true;
		}

		if (FName(*comp.name) == FName(TEXT("__CM__Emitter"))) {
			LOGD(NativeCtrlModel, "Device(%d), Emitter is found", deviceTypeInt);

			EmitterTransform = FTransform(comp.transform);

			FVector v = EmitterTransform.GetLocation();
			FRotator r = EmitterTransform.GetRotation().Rotator();

			LOGD(NativeCtrlModel, "Device(%d), Emitter loc(x=%f y=%f z=%f) rot(r=%f p=%f y=%f)", deviceTypeInt, v.X, v.Y, v.Z, r.Roll, r.Pitch, r.Yaw);
		}
	}

	BuildEffectCaches();
	componentIsCompleted = true;
}

void AWaveVRNativeModel::BuildEffectCaches() {
	inputMappingPairUpdatedFrame = GFrameCounter;
	CheckIfButtonAnimation();

	if (!isButtonAnimation) {
		// collect button effect
		CollectButtonEffectMesh();
		CollectTouchEffectMesh();
	}
}

void AWaveVRNativeModel::CheckIfButtonAnimation()
{
	WVR_CtrlerModelAnimData* anim = nullptr;
	WVR_Result result = FWaveVRAPIWrapper::GetInstance()->GetCtrlerModelAnimNodeData(deviceTypeWVR, &anim);

	if ((result != WVR_Result::WVR_Success) || (anim == nullptr) || (anim->size == 0))
	{
		isButtonAnimation = false;
		FWaveVRAPIWrapper::GetInstance()->ReleaseCtrlerModelAnimNodeData(&anim);
		LOGI(NativeCtrlModel, "Device(%d), no animation data from native", deviceTypeInt);
		return;
	}
	isButtonAnimation = true;

	int buttonAnimCount = anim->size;

	LOGI(NativeCtrlModel, "Device(%d), Get animation data (%d) from native", deviceTypeInt, buttonAnimCount);

	binaryObjectMap.Empty();
	binaryState.Empty();
	travel1DObjectMap.Empty();
	thumbstickObjectMap.Empty();

	const auto& featureTableAnime = GetButtonFeatureTableAnime();

	for (uint8 b = 0; b <= (uint8)EWVR_InputId::Thumbstick; b++)
	{
		EWVR_InputId srcBtn = UWaveVRBlueprintFunctionLibrary::GetInputMappingPair(deviceType, (EWVR_InputId)b);
		if (srcBtn == EWVR_InputId::NoUse) continue;

		// Get button feature by inputId
		int N = featureTableAnime.Num();
		for (int i = 0; i < N; i++) {
			const auto& btnFeature = featureTableAnime[i];
			if (btnFeature.inputId != srcBtn) continue;

			// Get mesh comopnent by name
			int* ptrCompId = componentTable.Find(btnFeature.lowerName);
			if (ptrCompId == nullptr) continue;
			auto& comp = components[*ptrCompId];

			for (int k = 0; k < buttonAnimCount; k++)
			{
				WVR_CtrlerModelAnimNodeData_t animNodeData = anim->animDatas[k];
				FString meshName = animNodeData.name;
				if (meshName.ToLower() != btnFeature.lowerName) continue;

				if (btnFeature.animBinary && animNodeData.type == 1) {
					LOGI(NativeCtrlModel, "Device(%d), button name (%s) type(%d) has animation", deviceTypeInt, PLATFORM_CHAR(*meshName), animNodeData.type);

					BinaryButtonObject s;
					s.btn = srcBtn;
					s.meshName = FName(btnFeature.name);
					s.MeshComp = comp.mesh;
					s.MeshCompOutline = comp.meshOutline;

					FTransform o(comp.transform);
					FVector v = o.GetLocation();
					FRotator r = o.GetRotation().Rotator();
					s.originPosition = FromGLToUnreal(animNodeData.origin.position, 100);
					s.originRotation = FromGLToUnrealEuler(animNodeData.origin.rotation);
					LOGI(NativeCtrlModel, "Device(%d), mesh pos(%f, %f, %f) %s", deviceTypeInt, v.X, v.Y, v.Z, PLATFORM_CHAR(*comp.name));
					LOGI(NativeCtrlModel, "Device(%d), origin pos(%f, %f, %f)", deviceTypeInt, s.originPosition.X, s.originPosition.Y, s.originPosition.Z);

					s.pressPosition = FromGLToUnreal(animNodeData.pressed.position, 100);
					s.pressRotation = FromGLToUnrealEuler(animNodeData.pressed.rotation);
					float * vs = animNodeData.pressed.scale.v;
					s.pressScale = FVector(vs[0], vs[1], vs[2]);

					binaryObjectMap.Add(s);
					binaryState.Add(false);

					//LOGI(NativeCtrlModel, "Device(%d), origin pos(%f, %f, %f)", deviceTypeInt, s.originPosition.X, s.originPosition.Y, s.originPosition.Z);
					LOGI(NativeCtrlModel, "Device(%d), press pos(%f, %f, %f)", deviceTypeInt, s.pressPosition.X, s.pressPosition.Y, s.pressPosition.Z);
				}

				//LOGI(NativeCtrlModel, "Device(%d), (%d) find meshcomponent", deviceTypeInt, (uint8)t.btn);
				if (btnFeature.animTravel1D && animNodeData.type == 2) {
					LOGI(NativeCtrlModel, "Device(%d), button name (%s) type(%d) has animation", deviceTypeInt, PLATFORM_CHAR(*meshName), animNodeData.type);

					Travel1DObject s;
					s.btn = srcBtn;
					s.meshName = FName(meshName);
					s.MeshComp = comp.mesh;

					FTransform o(comp.transform);
					FVector v = o.GetLocation();
					FRotator r = o.GetRotation().Rotator();
					s.originPosition = FromGLToUnreal(animNodeData.origin.position, 100);
					s.originRotation = FromGLToUnrealEuler(animNodeData.origin.rotation);
					LOGI(NativeCtrlModel, "Device(%d), mesh pos(%f, %f, %f)", deviceTypeInt, v.X, v.Y, v.Z);
					LOGI(NativeCtrlModel, "Device(%d), mesh rot(r=%f, p=%f, y=%f)", deviceTypeInt, r.Roll, r.Pitch, r.Yaw);
					LOGI(NativeCtrlModel, "Device(%d), origin pos(%f, %f, %f)", deviceTypeInt, s.originPosition.X, s.originPosition.Y, s.originPosition.Z);
					LOGI(NativeCtrlModel, "Device(%d), origin rot(r=%f, p=%f, y=%f)", deviceTypeInt, s.originRotation.Roll, s.originRotation.Pitch, s.originRotation.Yaw);

					s.pressPosition = FromGLToUnreal(animNodeData.pressed.position, 100);
					s.pressRotation = FromGLToUnrealEuler(animNodeData.pressed.rotation);

					LOGI(NativeCtrlModel, "Device(%d), press pos(%f, %f, %f)", deviceTypeInt, s.pressPosition.X, s.pressPosition.Y, s.pressPosition.Z);
					LOGI(NativeCtrlModel, "Device(%d), press rot(r=%f, p=%f, y=%f)", deviceTypeInt, s.pressRotation.Roll, s.pressRotation.Pitch, s.pressRotation.Yaw);

					travel1DObjectMap.Add(s);
				}

				if (btnFeature.animThumbstick && animNodeData.type == 3) {
					LOGI(NativeCtrlModel, "Device(%d), button name (%s) type(%d) has animation", deviceTypeInt, PLATFORM_CHAR(*meshName), animNodeData.type);

					ThumbstickObject s;
					s.btn = srcBtn;
					s.meshName = FName(meshName);
					s.MeshComp = comp.mesh;
					s.MeshCompOutline = comp.meshOutline;

					FTransform o(comp.transform);
					FVector v = o.GetLocation();
					FRotator r = o.GetRotation().Rotator();
					s.centerPosition = FromGLToUnrealVector(animNodeData.origin.position.v, 100);
					s.centerRotation = FromGLToUnrealEuler(animNodeData.origin.rotation.v);
					LOGI(NativeCtrlModel, "Device(%d), mesh pos(%f, %f, %f)", deviceTypeInt, v.X, v.Y, v.Z);
					LOGI(NativeCtrlModel, "Device(%d), mesh rot(r=%f, p=%f, y=%f)", deviceTypeInt, r.Roll, r.Pitch, r.Yaw);

					s.upPosition = FromGLToUnrealVector(animNodeData.maxY.position.v, 100);
					s.upRotation = FromGLToUnrealEuler(animNodeData.maxY.rotation.v);

					s.rightPosition = FromGLToUnrealVector(animNodeData.maxX.position.v, 100);
					s.rightRotation = FromGLToUnrealEuler(animNodeData.maxX.rotation.v);

					s.maxRotation = FRotator((s.centerRotation - s.upRotation).Pitch, 0, (s.rightRotation - s.centerRotation).Roll);

					LOGD(NativeCtrlModel, "Device(%d), maxRotation(p=%f, r=%f, y=%f)", deviceTypeInt, s.maxRotation.Pitch, s.maxRotation.Roll, s.maxRotation.Yaw);

					thumbstickObjectMap.Add(s);
				}
			}
		}
	}

	FWaveVRAPIWrapper::GetInstance()->ReleaseCtrlerModelAnimNodeData(&anim);
}

void AWaveVRNativeModel::UpdateMaterial()
{
	if (ctrl == nullptr)
		return;

	UTexture2D* _texture = nullptr;
	const uint32_t bmSize = (*ctrl).bitmapInfos.size;
	for (uint32_t i = 0; i < bmSize; i++) {
		const auto& table = (*ctrl).bitmapInfos.table[i];
		_texture = UTexture2D::CreateTransient(table.width, table.height, PF_R8G8B8A8);
		if (_texture)
		{
			//Lock it
			void* TextureData = _texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			//Copy pixel data
			FMemory::Memcpy(TextureData, table.bitmap, table.height * table.stride);
			//Unlock
			_texture->PlatformData->Mips[0].BulkData.Unlock();
			//Update texture
			_texture->UpdateResource();
		}
	}

	// Note: the bmSize will always be 1
	bodyTex = _texture;

	for (int32 i = 0; i < componentCount; i++) {
		if (controllerInterface == nullptr) {
			LOGE(NativeCtrlModel, "Device(%d), Can't find NControllerMat.NControllerMat", deviceTypeInt);
			components[i].material = nullptr;
		} else {
			auto materialDynamic = UMaterialInstanceDynamic::Create(controllerInterface, this);
			if (materialDynamic != nullptr)
				materialDynamic->SetTextureParameterValue(FName(TEXT("Tex")), bodyTex);

			components[i].material = materialDynamic;
		}

		components[i].materialOutline = nullptr;
		if (components[i].meshOutline != nullptr) {
			if (controllerOutlineInterface != nullptr)
				components[i].materialOutline = UMaterialInstanceDynamic::Create(controllerOutlineInterface, this);
			else
				LOGE(NativeCtrlModel, "Device(%d), Can't find NControllerOutlineMat.NControllerOutlineMat", deviceTypeInt);
		}
	}
}

void AWaveVRNativeModel::ParseMesh()
{
	ClearProcessedMeshData();
	if (ctrl == nullptr) return;
	ProcessNode();
}

void AWaveVRNativeModel::ClearProcessedMeshData()
{
	components.SetNumUninitialized(0);
	componentCount = 0;
}

void AWaveVRNativeModel::ProcessNode()
{
	int N = (*ctrl).compInfos.size;
	//Init container
	components.SetNum(N);
	componentCount = N;

	for (int i = 0; i < N; i++) {
		const WVR_CtrlerCompInfo_t& compInfo = (*ctrl).compInfos.table[i];

		FMeshComponent& comp = components[i];
		comp.name = compInfo.name;
		comp.mesh = nullptr;
		comp.meshOutline = nullptr;
		comp.lowerName = comp.name.ToLower();
		comp.transform = FromGLToUnrealMatrix(compInfo.localMat, true, 100);
		DebugMatrixNM(comp.transform);

		componentTable.Add(comp.lowerName, i);

		LOGD(NativeCtrlModel, "Device(%d), componentName %s", deviceTypeInt, PLATFORM_CHAR(*comp.name));
		ProcessMesh(comp, compInfo.vertices, compInfo.normals, compInfo.texCoords, compInfo.indices);
	}
}

void AWaveVRNativeModel::ProcessMesh(FMeshComponent& comp, const WVR_VertexBuffer_t& vertices, const WVR_VertexBuffer_t& normals, const WVR_VertexBuffer_t& texCoords, const WVR_IndexBuffer_t& indices)
{
	bool _requiresFullRecreation = true;  // Quaker: always rebuilt because component is clean.

	// we check whether the current data to read has a different amount of vertices compared to the last time we generated the mesh
	// if so, it means we'll need to recreate the mesh and resupply new indices.
	//
	//if (vertexArraySize != comp.vertices.Num())
	//	_requiresFullRecreation = true;

	// we reinitialize the arrays for the new data we're reading
	uint32_t vertexArraySize = vertices.size / vertices.dimension;
	uint32_t normalArraySize = normals.size / normals.dimension;
	uint32_t uvArraySize = texCoords.size / texCoords.dimension;
	uint32_t indexArraySize = indices.size;

	comp.vertices.SetNumUninitialized(vertexArraySize);
	comp.tangents.SetNumUninitialized(vertexArraySize);
	comp.normals.SetNumUninitialized(normalArraySize);
	comp.uvs.SetNumUninitialized(uvArraySize);
	comp.vertexColors.SetNum(0);
	comp.indices.SetNumUninitialized(indexArraySize);

	/** Vertices **/
	if (vertices.dimension == 3) {
		for (uint32_t j = 0; j < vertexArraySize; j++) {
			FVector vertex = FromGLToUnrealVector(vertices.buffer + j * 3, 100);
			comp.vertices[j] = vertex;
			/** Tangents **/
			comp.tangents[j] = FProcMeshTangent(0, 1, 0);
		}
	}

	/** Normals **/
	if (normals.dimension == 3) {
		for (uint32_t j = 0; j < normalArraySize; j++) {
			FVector normal = FromGLToUnrealVector(normals.buffer + j * 3, 1);
			comp.normals[j] = normal;
		}
	}

	/** UVs **/
	if (texCoords.dimension == 2) {
		for (uint32_t j = 0; j < uvArraySize; j++) {
			FVector2D uv;
			uv.X = texCoords.buffer[j * 2];
			uv.Y = texCoords.buffer[j * 2 + 1];
			comp.uvs[j] = uv;
		}
	}

	if (_requiresFullRecreation) {
		/** Indices **/
		if (indices.type == 3) {
			for (uint32_t j = 0; j < indexArraySize; j++)
				comp.indices[j] = indices.buffer[j];
		}
	}
}
