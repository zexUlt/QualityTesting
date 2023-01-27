// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "InputModule/WaveVRControllerPointer.h"
#include "InputModule/WaveVRInteractInterface.h"

#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Platforms/WaveVRLogWrapper.h"
#include "Platforms/WaveVRAPIWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRControllerPointer, Log, All);

UWaveVRControllerPointer::UWaveVRControllerPointer()
	: UseInputManager(false)
	, FadingParticle(nullptr)
	, SpotPointer(nullptr)
	, WidgetInteraction(nullptr)
	, fadingParticleSystemTemplate(nullptr)
	, fadingParticleMaterial(nullptr)
	, spotPointerMesh(nullptr)
	, spotPointerMaterial(nullptr)
	, targetDistance(TraceDistance) // Default beam length and pointer distance
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	m_RightClickButtons = RightClickButtons;
	m_LeftClickButtons = LeftClickButtons;

	FString p_fadinglasersystem = TEXT("ParticleSystem'/WaveVR/Materials/P_FadingLaserSystem.P_FadingLaserSystem'");
	fadingParticleSystemTemplate = Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(), NULL, *p_fadinglasersystem));
	FString m_fadinglasersystem = TEXT("Material'/WaveVR/Materials/M_FadingLaserSystem.M_FadingLaserSystem'");
	fadingParticleMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *m_fadinglasersystem));
	//FadingParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FadingParticle"));
	FadingParticle = NewObject<UParticleSystemComponent>(this, TEXT("FadingParticle"));
	FadingParticle->SetupAttachment(this);

	FString reticle_mesh = TEXT("StaticMesh'/WaveVR/Shapes/Reticle_Mesh.Reticle_Mesh'");
	spotPointerMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *reticle_mesh));
	FString m_reticle = TEXT("Material'/WaveVR/Materials/M_Reticle.M_Reticle'");
	spotPointerMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *m_reticle));
	//SpotPointer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotPointer"));
	SpotPointer = NewObject<UStaticMeshComponent>(this, TEXT("SpotPointer"));
	SpotPointer->SetupAttachment(this);

	//WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
	WidgetInteraction = NewObject<UWidgetInteractionComponent>(this, TEXT("WidgetInteraction"));
	WidgetInteraction->SetupAttachment(this);
}

void UWaveVRControllerPointer::CheckLogInterval()
{
	logFrame++;
	logFrame = logFrame % klogFrameCount;
	if (logFrame == 0)
		printIntervalLog = true;
	else
		printIntervalLog = false;
}

#pragma region
void UWaveVRControllerPointer::SetDeviceType(EWVR_DeviceType deviceType)
{
	DeviceType = deviceType;
	LOGD(LogWaveVRControllerPointer, "Device %d, SetDeviceType()", (uint8)DeviceType);
}
void UWaveVRControllerPointer::SetWidgetPointerIndex(int32 pointerIndex)
{
	if (WidgetInteraction)
	{
		WidgetPointerIndex = pointerIndex;
		WidgetInteraction->PointerIndex = WidgetPointerIndex;
		LOGD(LogWaveVRControllerPointer, "Device %d, SetWidgetPointerIndex() %d", (uint8)DeviceType, WidgetInteraction->PointerIndex);
	}
}
void UWaveVRControllerPointer::SetupControllerButton(const TArray<EWVR_InputId> leftButtons, const TArray<EWVR_InputId> rightButtons)
{
	if (rightButtons.Num() > 0)
	{
		RightClickButtons = rightButtons;
		for (int i = 0; i < RightClickButtons.Num(); i++)
		{
			LOGD(LogWaveVRControllerPointer, "Device %d, SetupControllerButton() right %d", (uint8)DeviceType, (uint8)RightClickButtons[i]);
		}
	}
	if (leftButtons.Num() > 0)
	{
		LeftClickButtons = leftButtons;
		for (int i = 0; i < LeftClickButtons.Num(); i++)
		{
			LOGD(LogWaveVRControllerPointer, "Device %d, SetupControllerButton() left %d", (uint8)DeviceType, (uint8)LeftClickButtons[i]);
		}
	}
}
void UWaveVRControllerPointer::SetUseInputManager(bool use)
{
	LOGD(LogWaveVRControllerPointer, "Device %d, SetUseInputManager() %d", (uint8)DeviceType, (uint8)use);
	UseInputManager = use;
}
#pragma endregion Public Interface

void UWaveVRControllerPointer::UpdateCustomizedVariables()
{
	if (m_DeviceType != DeviceType)
	{
		if (DeviceType == EWVR_DeviceType::DeviceType_Controller_Right || DeviceType == EWVR_DeviceType::DeviceType_Controller_Left)
		{
			m_DeviceType = DeviceType;
			LOGD(LogWaveVRControllerPointer, "Device %d, UpdateCustomizedVariables()", (uint8)m_DeviceType);
		}
	}
	if (m_RightClickButtons != RightClickButtons || m_LeftClickButtons != LeftClickButtons)
	{
		m_RightClickButtons = RightClickButtons;
		m_LeftClickButtons = LeftClickButtons;
		LOGD(LogWaveVRControllerPointer, "Device %d, UpdateCustomizedVariables() click buttons changed.", (uint8)m_DeviceType);
	}
}

void UWaveVRControllerPointer::ConfigureResources()
{
	if (FadingParticle)
	{
		if (fadingParticleSystemTemplate)
		{
			FadingParticle->SetTemplate(fadingParticleSystemTemplate);
			LOGD(LogWaveVRControllerPointer, "ConfigureResources() fadingParticleSystemTemplate");
		}
		if (fadingParticleMaterial)
		{
			FadingParticle->SetMaterial(fadingParticleMaterialIndex, fadingParticleMaterial);
			LOGD(LogWaveVRControllerPointer, "ConfigureResources() fadingParticleMaterial");
		}
		FVector vec_color = UKismetMathLibrary::Conv_LinearColorToVector(FadingParticleColor);
		FadingParticle->SetVectorParameter(k_InitialColor, vec_color);
		FadingParticle->SetVectorParameter(k_InitialSize, FadingParticleSize);
	}

	if (SpotPointer)
	{
		if (spotPointerMesh)
		{
			SpotPointer->SetStaticMesh(spotPointerMesh);
			LOGD(LogWaveVRControllerPointer, "ConfigureResources() spotPointerMesh");
		}
		if (spotPointerMaterial)
		{
			SpotPointer->SetMaterial(0, spotPointerMaterial);
			LOGD(LogWaveVRControllerPointer, "ConfigureResources() spotPointerMaterial");
		}
		SpotPointer->SetRelativeRotation(SpotRotationOffset);
		SpotPointer->SetRelativeScale3D(SpotScale);
		SpotPointer->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

		LOGD(LogWaveVRControllerPointer, "ConfigureResources() SpotPointer");
	}

	if (WidgetInteraction)
	{
		WidgetInteraction->VirtualUserIndex = WidgetVirtualUserIndex;
		WidgetInteraction->PointerIndex = WidgetPointerIndex;
		WidgetInteraction->TraceChannel = ECollisionChannel::ECC_Visibility;
		WidgetInteraction->InteractionDistance = TraceDistance;
		WidgetInteraction->InteractionSource = EWidgetInteractionSource::World;
		//WidgetInteraction->bEnableHitTesting = true;
		//WidgetInteraction->bShowDebug = true;
		//WidgetInteraction->DebugColor = FLinearColor::Red;

		LOGD(LogWaveVRControllerPointer, "ConfigureResources() WidgetInteraction VirtualUserIndex %d, PointerIndex %d, TraceChannel: %d, InteractionDistance: %f, InteractionSource: %d",
			WidgetInteraction->VirtualUserIndex,
			WidgetInteraction->PointerIndex,
			(int)WidgetInteraction->TraceChannel,
			WidgetInteraction->InteractionDistance,
			(int8)WidgetInteraction->InteractionSource);
	}
}

void UWaveVRControllerPointer::SetIgnoreActors(FCollisionQueryParams& collisionParams)
{
	// Ignore WaveVR_LinePointer blueprint actor.
	/*FString pointer_path = TEXT("Blueprint'/WaveVR/Blueprints/ControllerModel/WaveVR_LinePointer.WaveVR_LinePointer'");
	UBlueprint* pointer_blueprint = Cast<UBlueprint>(StaticLoadObject(UObject::StaticClass(), nullptr, *pointer_path));
	if (pointer_blueprint && pointer_blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
	{
		TArray<AActor *> pointer_ignored;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), *pointer_blueprint->GeneratedClass, pointer_ignored);
		collisionParams.AddIgnoredActors(pointer_ignored);
		for (int i = 0; i < pointer_ignored.Num(); i++)
		{
			AActor* actor = pointer_ignored[i];
			LOGD(LogWaveVRControllerPointer, "SetIgnoreActors() %s", *actor->GetFullName());
		}
	}*/

	// Ignore WaveVR_AdaptiveController blueprint actor.
	/*FString controller_path = TEXT("Blueprint'/WaveVR/Blueprints/ControllerModel/WaveVR_AdaptiveController.WaveVR_AdaptiveController'");
	UBlueprint* controller_blueprint = Cast<UBlueprint>(StaticLoadObject(UObject::StaticClass(), nullptr, *controller_path));
	if (controller_blueprint && controller_blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
	{
		TArray<AActor *> controller_ignored;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), *controller_blueprint->GeneratedClass, controller_ignored);
		collisionParams.AddIgnoredActors(controller_ignored);
		for (int i = 0; i < controller_ignored.Num(); i++)
		{
			AActor* actor = controller_ignored[i];
			LOGD(LogWaveVRControllerPointer, "SetIgnoreActors() %s", *actor->GetFullName());
		}
	}*/

	// Ignore WaveVRInputModuleGaze blueprint actor.
	/*FString gaze_path = TEXT("Blueprint'/WaveVR/Blueprints/WaveVRInputModuleGaze.WaveVRInputModuleGaze'");
	UBlueprint* gaze_blueprint = Cast<UBlueprint>(StaticLoadObject(UObject::StaticClass(), nullptr, *gaze_path));
	if (gaze_blueprint && gaze_blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
	{
		TArray<AActor *> gaze_ignored;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), *gaze_blueprint->GeneratedClass, gaze_ignored);
		collisionParams.AddIgnoredActors(gaze_ignored);
		for (int i = 0; i < gaze_ignored.Num(); i++)
		{
			AActor* actor = gaze_ignored[i];
			LOGD(LogWaveVRControllerPointer, "SetIgnoreActors() %s", *actor->GetFullName());
		}
	}*/

	/*for (int i = 0; i < adaptive_controllers.Num(); i++)
	{
		AActor* actor = adaptive_controllers[i];
		LOGD(LogWaveVRControllerPointer, "SetIgnoreActors() %s", *actor->GetFullName());
	}*/

	// Ignore the Pawn.
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		APawn* pawn = UGameplayStatics::GetPlayerPawn(Context.World(), PlayerIndex);
		if (pawn)
		{
			collisionParams.AddIgnoredActor(pawn);
			//LOGD(LogWaveVRControllerPointer, "SetIgnoreActors() %s", *pawn->GetFullName());
		}
	}

	// Ignore parent actor.
	USceneComponent* parent = GetAttachParent();
	while (parent)
	{
		collisionParams.AddIgnoredActor(parent->GetOwner());
		parent = parent->GetAttachParent();
	}
}

void UWaveVRControllerPointer::BeginPlay()
{
	Super::BeginPlay();

	ConfigureResources();

	worldLocation = GetComponentLocation();
	FVector end_pos = (GetForwardVector() * TraceDistance) + worldLocation;

	if (FadingParticle)
	{
		FadingParticle->SetBeamSourcePoint(BeamEmitterIndex, worldLocation, BeamSourceIndex);
		FadingParticle->SetBeamTargetPoint(BeamEmitterIndex, end_pos, BeamSourceIndex);
	}
	if (SpotPointer)
	{
		SpotPointer->SetWorldLocation(end_pos);
	}
}

void UWaveVRControllerPointer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckLogInterval();

	UpdateCustomizedVariables();

	if (!IsControllerInteractable())
		return;

	tracedPhysics = false;

	worldLocation = GetComponentLocation();
	FVector end_pos = (GetForwardVector() * TraceDistance) + worldLocation;
	worldRotation = GetComponentRotation();

	targetPoint = (GetForwardVector() * targetDistance) + worldLocation;

	bool clicked = IsClicked();

	//FCollisionQueryParams CollisionParams;
	//SetIgnoreActors(CollisionParams);
	//tracedPhysics = GetWorld()->LineTraceSingleByChannel(hit, worldLocation, end_pos, ECC_Camera, CollisionParams);

	FCollisionShape collision_shape;// = FCollisionShape::MakeSphere(500.0f);
	bool casted = GetWorld()->SweepMultiByChannel(outHits, worldLocation, end_pos, FQuat::Identity, ECC_WorldStatic, collision_shape);
	if (casted)
	{
		for (auto& outHit : outHits)
		{
			//AActor* hit_actor = hit.GetActor();
			AActor* hit_actor = outHit.GetActor();

			if (!hit_actor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass()))
				continue;
			//LOGD(LogWaveVRControllerPointer, "TickComponent() Hitting: %s (%s)", *hit_actor->GetName(), *outHit.ImpactPoint.ToString());

			tracedPhysics = true;
			if (focusActor != hit_actor) // Enter & Exit
			{
				if (focusActor && (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
				{
					// Exit previous actor
					IWaveVRInteractInterface::Execute_OnExit(focusActor, GetOwner());
#if PLATFORM_ANDROID
					LOGD(LogWaveVRControllerPointer, "TickComponent() Exit: %s", TCHAR_TO_ANSI(*focusActor->GetName()));
#else
					LOGD(LogWaveVRControllerPointer, "TickComponent() Exit: %s", *focusActor->GetName());
#endif
				}
				if (hit_actor && (hit_actor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
				{
					// Enter current actor
					IWaveVRInteractInterface::Execute_OnEnter(hit_actor, GetOwner());
#if PLATFORM_ANDROID
					LOGD(LogWaveVRControllerPointer, "TickComponent() Enter: %s", TCHAR_TO_ANSI(*hit_actor->GetName()));
#else
					LOGD(LogWaveVRControllerPointer, "TickComponent() Enter: %s", *hit_actor->GetName());
#endif
				}

				focusActor = hit_actor;
				break; // Use only the 1st hitted actor;
			}
			else // Hover & Click
			{
				if (focusActor && (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
				{
					IWaveVRInteractInterface::Execute_OnHover(focusActor, GetOwner());
					if (clicked)
					{
						IWaveVRInteractInterface::Execute_OnClick(focusActor, GetOwner());
#if PLATFORM_ANDROID
						LOGD(LogWaveVRControllerPointer, "TickComponent() Click: %s", TCHAR_TO_ANSI(*hit_actor->GetName()));
#else
						LOGD(LogWaveVRControllerPointer, "TickComponent() Click: %s", *hit_actor->GetName());
#endif
					}

					// Change the beam length and pointer distance
					//targetDistance = FVector::Distance(worldLocation, hit.ImpactPoint);
					targetDistance = FVector::Distance(worldLocation, outHit.ImpactPoint);
					targetPoint = (GetForwardVector() * targetDistance) + worldLocation;
					break; // Use only the 1st hitted actor;
				}
			}
		}
	}

	if (!tracedPhysics)
	{
		if (focusActor)
		{
			if (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass()))
			{
				// Exit previous actor
				IWaveVRInteractInterface::Execute_OnExit(focusActor, GetOwner());
#if PLATFORM_ANDROID
				LOGD(LogWaveVRControllerPointer, "TickComponent() Exit: %s", TCHAR_TO_ANSI(*focusActor->GetName()));
#else
				LOGD(LogWaveVRControllerPointer, "TickComponent() Exit: %s", *focusActor->GetName());
#endif
			}
			focusActor = nullptr;
		}

		if (WidgetInteraction) // Click
		{
			WidgetInteraction->SetWorldLocationAndRotation(worldLocation, worldRotation);
			if (WidgetInteraction->IsOverFocusableWidget())
			{
				UWidgetComponent* widget = WidgetInteraction->GetHoveredWidgetComponent();
				UUserWidget* user_widget = widget->GetUserWidgetObject();
				tracedWidgetPosition = (static_cast<USceneComponent *>(widget))->GetComponentLocation();

				// Change the beam length and pointer distance
				targetDistance = FVector::Distance(worldLocation, tracedWidgetPosition);
				targetPoint = (GetForwardVector() * targetDistance) + worldLocation;

				if (clicked)
				{
					WidgetInteraction->PressPointerKey(FKey("LeftMouseButton"));
					WidgetInteraction->ReleasePointerKey(FKey("LeftMouseButton"));
#if PLATFORM_ANDROID
					LOGD(LogWaveVRControllerPointer, "TickComponent() Click widget: %s", TCHAR_TO_ANSI(*user_widget->GetFullName()));
#else
					LOGD(LogWaveVRControllerPointer, "TickComponent() Click widget: %s", *user_widget->GetFullName());
#endif
				}
			}
		}
	}

	if (FadingParticle)
	{
		FadingParticle->SetBeamSourcePoint(BeamEmitterIndex, worldLocation, BeamSourceIndex);
		FadingParticle->SetBeamTargetPoint(BeamEmitterIndex, targetPoint, BeamSourceIndex);
	}
	if (SpotPointer)
	{
		SpotPointer->SetWorldLocation(targetPoint);
		FVector pointer_scale = SpotScale;
		if (targetDistance > k_DefaultPointerDistance)
			pointer_scale = SpotScale * (targetDistance / k_DefaultPointerDistance);
		SpotPointer->SetRelativeScale3D(pointer_scale);
	}
}

bool UWaveVRControllerPointer::IsControllerInteractable()
{
	bool controller_mode = (UWaveVRBlueprintFunctionLibrary::GetInteractionMode() == EWVRInteractionMode::Controller);
	bool isFocusedController = (!UseInputManager)
		|| (UWaveVRBlueprintFunctionLibrary::GetFocusController() == m_DeviceType);
	bool valid_pose = UWaveVRBlueprintFunctionLibrary::GetDevicePose(devicePosition, deviceRotation, DeviceType);
	bool focusCapturedBySystem = UWaveVRBlueprintFunctionLibrary::IsInputFocusCapturedBySystem();
	bool tableStatic = UWaveVRBlueprintFunctionLibrary::IsDeviceTableStatic(DeviceType);
	bool hideStatic = HideWhenIdle && tableStatic;

	bool interactable = (AlwaysEnable || controller_mode)
		&& isFocusedController
		&& valid_pose
		&& (!focusCapturedBySystem)
		&& (!hideStatic);

	if (printIntervalLog)
	{
		LOGD(LogWaveVRControllerPointer,
			"IsControllerInteractable() DeviceType %d, interactable %d, isFocusedController %d, AlwaysEnable %d, controller_mode %d, focusCapturedBySystem %d, valid_pose %d, hideStatic %d, HideWhenIdle %d, tableStatic %d",
			(uint8)DeviceType,
			(uint8)interactable,
			(uint8)isFocusedController,
			(uint8)AlwaysEnable,
			(uint8)controller_mode,
			(uint8)focusCapturedBySystem,
			(uint8)valid_pose,
			(uint8)hideStatic,
			(uint8)HideWhenIdle,
			(uint8)tableStatic);
	}

	if (WidgetInteraction)
	{
		if ((interactable && !WidgetInteraction->IsActive()) ||
			(!interactable && WidgetInteraction->IsActive()))
		{
			LOGD(LogWaveVRControllerPointer, "Device %d, IsControllerInteractable() %d", (uint8)m_DeviceType, (uint8)interactable);
		}
		WidgetInteraction->SetActive(interactable);
	}
	if (SpotPointer)
		SpotPointer->SetHiddenInGame(!interactable, true);
	if (FadingParticle)
		FadingParticle->SetHiddenInGame(!interactable, true);

	return interactable;
}

bool UWaveVRControllerPointer::IsClicked()
{
	bool pressed = false;
	int pointerIndex = 0;
	while (!pressed && pointerIndex < m_RightClickButtons.Num())
	{
		pressed = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_Controller_Right, m_RightClickButtons[pointerIndex++]);
	}
	pointerIndex = 0;
	while (!pressed && pointerIndex < m_LeftClickButtons.Num())
	{
		pressed = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_Controller_Left, m_LeftClickButtons[pointerIndex++]);
	}

	bool clicked = (pressState == false && pressed == true);
	pressState = pressed;

	return clicked;
}
