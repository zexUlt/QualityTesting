// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "InputModule/WaveVRHandPointer.h"
#include "InputModule/WaveVRInteractInterface.h"
#include "WaveVRBlueprintFunctionLibrary.h"

#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "Platforms/WaveVRAPIWrapper.h"
#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRHandPointer, Log, All);

UWaveVRHandPointer::UWaveVRHandPointer()
	: FadingParticle(nullptr)
	, SpotPointer(nullptr)
	, SpotPointerBlue(nullptr)
	, WidgetInteraction(nullptr)
	, fadingParticleSystemTemplate(nullptr)
	, fadingParticleMaterial(nullptr)
	, spotPointerMesh(nullptr)
	, spotPointerMaterial(nullptr)
	, spotPointerBlueMesh(nullptr)
	, spotPointerBlueMaterial(nullptr)
	, worldLocation(FVector::ZeroVector)
	, worldRotation(FRotator::ZeroRotator)
	, targetPoint(FVector::ZeroVector)
	, focusActor(nullptr)
	, targetDistance(TraceDistance) // Default beam length and pointer distance
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	m_TrackerType = TrackerType;
	m_Hand = Hand;
	m_UseDefaultPinch = UseDefaultPinch;
	m_PinchOnThreshold = PinchOnThreshold;

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

	FString reticle_blue1_mesh = TEXT("StaticMesh'/WaveVR/Shapes/ReticleBlue1_Mesh.ReticleBlue1_Mesh'");
	spotPointerBlueMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *reticle_blue1_mesh));
	FString m_reticle_blue1 = TEXT("Material'/WaveVR/Materials/M_ReticleBlue1.M_ReticleBlue1'");
	spotPointerBlueMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *m_reticle_blue1));
	//SpotPointerBlue = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotPointerBlue"));
	SpotPointerBlue = NewObject<UStaticMeshComponent>(this, TEXT("SpotPointerBlue"));
	SpotPointerBlue->SetupAttachment(this);

	//WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
	WidgetInteraction = NewObject<UWidgetInteractionComponent>(this, TEXT("WidgetInteraction"));
	WidgetInteraction->SetupAttachment(this);
}

void UWaveVRHandPointer::CheckLogInterval()
{
	logFrame++;
	logFrame = logFrame % klogFrameCount;
	if (logFrame == 0)
		printIntervalLog = true;
	else
		printIntervalLog = false;
}

#pragma region
void UWaveVRHandPointer::SetTracker(EWaveVRTrackerType tracker)
{
	TrackerType = tracker;
}

void UWaveVRHandPointer::SetHand(EWaveVRHandType hand)
{
	Hand = hand;
}

void UWaveVRHandPointer::SetWidgetPointerIndex(int32 index)
{
	if (WidgetInteraction)
	{
		WidgetInteraction->PointerIndex = index;
		LOGD(LogWaveVRHandPointer, "SetWidgetPointerIndex() %d", WidgetInteraction->PointerIndex);
	}
}

void UWaveVRHandPointer::SetUseDefaultPinch(bool use)
{
	UseDefaultPinch = use;
}

void UWaveVRHandPointer::SetPinchOnThreshold(float strength)
{
	PinchOnThreshold = strength;
}
#pragma endregion Public Interface

void UWaveVRHandPointer::UpdateCustomizedVariables()
{
	if (m_TrackerType != TrackerType)
	{
		m_TrackerType = TrackerType;
		LOGD(LogWaveVRHandPointer, "m_TrackerType %d, m_Hand %d, UpdateCustomizedVariables()", (uint8)m_TrackerType, (uint8)m_Hand);
	}
	if (m_Hand != Hand)
	{
		m_Hand = Hand;
		LOGD(LogWaveVRHandPointer, "m_TrackerType %d, m_Hand %d, UpdateCustomizedVariables()", (uint8)m_TrackerType, (uint8)m_Hand);
	}

	if (m_UseDefaultPinch != UseDefaultPinch)
	{
		m_UseDefaultPinch = UseDefaultPinch;
		LOGD(LogWaveVRHandPointer, "m_TrackerType %d, m_Hand %d, UpdateCustomizedVariables() m_UseDefaultPinch %d"
			, (uint8)m_TrackerType
			, (uint8)m_Hand
			, (uint8)m_UseDefaultPinch);
	}

	if (m_UseDefaultPinch)
	{
		m_PinchOnThreshold = UWaveVRHandBPLibrary::GetHandPinchThreshold(m_TrackerType);
	}
	else
	{
		if (m_PinchOnThreshold != PinchOnThreshold)
		{
			m_PinchOnThreshold = PinchOnThreshold;
			LOGD(LogWaveVRHandPointer, "m_TrackerType %d, m_Hand %d, UpdateCustomizedVariables() m_PinchOnThreshold %f"
				, (uint8)m_TrackerType
				, (uint8)m_Hand
				, m_PinchOnThreshold);
		}
	}
}

void UWaveVRHandPointer::ConfigureResources()
{
	if (FadingParticle)
	{
		if (fadingParticleSystemTemplate)
		{
			FadingParticle->SetTemplate(fadingParticleSystemTemplate);
			LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() fadingParticleSystemTemplate", (uint8)m_Hand);
		}
		if (fadingParticleMaterial)
		{
			FadingParticle->SetMaterial(fadingParticleMaterialIndex, fadingParticleMaterial);
			LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() fadingParticleMaterial", (uint8)m_Hand);
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
			LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() spotPointerMesh", (uint8)m_Hand);
		}
		if (spotPointerMaterial)
		{
			SpotPointer->SetMaterial(0, spotPointerMaterial);
			LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() spotPointerMaterial", (uint8)m_Hand);
		}
		SpotPointer->SetRelativeRotation(SpotRotationOffset);
		SpotPointer->SetRelativeScale3D(SpotScale);
		SpotPointer->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

		LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() SpotPointer", (uint8)m_Hand);
	}

	if (SpotPointerBlue)
	{
		if (spotPointerBlueMesh)
		{
			SpotPointerBlue->SetStaticMesh(spotPointerBlueMesh);
			LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() spotPointerBlueMesh", (uint8)m_Hand);
		}
		if (spotPointerBlueMaterial)
		{
			SpotPointerBlue->SetMaterial(0, spotPointerBlueMaterial);
			LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() spotPointerBlueMaterial", (uint8)m_Hand);
		}
		SpotPointerBlue->SetRelativeRotation(SpotRotationOffset);
		SpotPointerBlue->SetRelativeScale3D(SpotScale);
		SpotPointerBlue->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		SpotPointerBlue->SetHiddenInGame(true, true);

		LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() SpotPointerBlue", (uint8)m_Hand);
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

		LOGD(LogWaveVRHandPointer, "m_Hand %d, ConfigureResources() WidgetInteraction VirtualUserIndex: %d, PointerIndex: %d, TraceChannel: %d, InteractionDistance: %f, InteractionSource: %d",
			(uint8)m_Hand,
			WidgetInteraction->VirtualUserIndex,
			WidgetInteraction->PointerIndex,
			(int)WidgetInteraction->TraceChannel,
			WidgetInteraction->InteractionDistance,
			(int8)WidgetInteraction->InteractionSource);
	}
}

void UWaveVRHandPointer::SetIgnoreActors(FCollisionQueryParams& collisionParams)
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
			LOGD(LogWaveVRHandPointer, "SetIgnoreActors() %s", *actor->GetFullName());
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
			LOGD(LogWaveVRHandPointer, "SetIgnoreActors() %s", *actor->GetFullName());
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
			LOGD(LogWaveVRHandPointer, "SetIgnoreActors() %s", *actor->GetFullName());
		}
	}*/

	/*for (int i = 0; i < adaptive_controllers.Num(); i++)
	{
		AActor* actor = adaptive_controllers[i];
		LOGD(LogWaveVRHandPointer, "SetIgnoreActors() %s", *actor->GetFullName());
	}*/

	// Ignore parent actor.
	USceneComponent* parent = GetAttachParent();
	while (parent)
	{
		collisionParams.AddIgnoredActor(parent->GetOwner());
		parent = parent->GetAttachParent();
	}
}

void UWaveVRHandPointer::BeginPlay()
{
	Super::BeginPlay();

	ConfigureResources();

	worldLocation = UWaveVRHandBPLibrary::GetHandPinchOrigin(m_TrackerType, m_Hand) + GetComponentLocation();
	FVector pinch_direction = UWaveVRHandBPLibrary::GetHandPinchDirection(m_TrackerType, m_Hand);
	if (pinch_direction.X != 0)
	{
		float value_x = pinch_direction.X > 0 ? pinch_direction.X : -pinch_direction.X;
		pinch_direction = (pinch_direction / value_x); // Normalize
	}
	FVector end_pos = (pinch_direction * TraceDistance) + worldLocation;

	if (FadingParticle)
	{
		FadingParticle->SetBeamSourcePoint(BeamEmitterIndex, worldLocation, BeamSourceIndex);
		FadingParticle->SetBeamTargetPoint(BeamEmitterIndex, end_pos, BeamSourceIndex);
	}

	if (SpotPointer)
	{
		SpotPointer->SetWorldLocation(end_pos);
	}

	if (SpotPointerBlue)
	{
		SpotPointerBlue->SetWorldLocation(end_pos);
	}
}

void UWaveVRHandPointer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckLogInterval();

	UpdateCustomizedVariables();

	if (!IsHandInteractable())
		return;

	tracedPhysics = false;
	tracedGraphics = false;

	worldLocation = UWaveVRHandBPLibrary::GetHandPinchOrigin(m_TrackerType, m_Hand) + GetComponentLocation();
	FVector pinch_direction = UWaveVRHandBPLibrary::GetHandPinchDirection(m_TrackerType, m_Hand);
	if (pinch_direction.X != 0)
	{
		float value_x = pinch_direction.X > 0 ? pinch_direction.X : -pinch_direction.X;
		pinch_direction = (pinch_direction / value_x); // Normalize
	}
	FVector end_pos = (pinch_direction * TraceDistance) + worldLocation;
	worldRotation = UKismetMathLibrary::FindLookAtRotation(worldLocation, end_pos);

	targetPoint = (pinch_direction * targetDistance) + worldLocation;

	bool clicked = IsClicked();

	//FCollisionQueryParams collision_params;
	//SetIgnoreActors(collision_params);
	//tracedPhysics = GetWorld()->LineTraceSingleByChannel(hit, worldLocation, end_pos, ECC_Camera, collision_params);

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
			//LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Hitting: %s (%s)", (uint8)m_Hand, *hit_actor->GetName(), *outHit.ImpactPoint.ToString());

			tracedPhysics = true;
			if (focusActor != hit_actor) // Enter & Exit
			{
				if (focusActor && (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
				{
					// Exit previous actor
					IWaveVRInteractInterface::Execute_OnExit(focusActor, GetOwner());
#if PLATFORM_ANDROID
					LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Exit: %s", (uint8)m_Hand, TCHAR_TO_ANSI(*focusActor->GetName()));
#else
					LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Exit: %s", (uint8)m_Hand, *focusActor->GetName());
#endif
				}
				if (hit_actor && (hit_actor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
				{
					// Enter current actor
					IWaveVRInteractInterface::Execute_OnEnter(hit_actor, GetOwner());
#if PLATFORM_ANDROID
					LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Enter: %s", (uint8)m_Hand, TCHAR_TO_ANSI(*hit_actor->GetName()));
#else
					LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Enter: %s", (uint8)m_Hand, *hit_actor->GetName());
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
						LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Click: %s", (uint8)m_Hand, TCHAR_TO_ANSI(*hit_actor->GetName()));
#else
						LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Click: %s", (uint8)m_Hand, *hit_actor->GetName());
#endif
					}

					// Change the beam length and pointer distance
					//targetDistance = FVector::Distance(worldLocation, hit.ImpactPoint);
					targetDistance = FVector::Distance(worldLocation, outHit.ImpactPoint);
					targetPoint = (pinch_direction * targetDistance) + worldLocation;
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
				LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Exit: %s", (uint8)m_Hand, TCHAR_TO_ANSI(*focusActor->GetName()));
#else
				LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Exit: %s", (uint8)m_Hand, *focusActor->GetName());
#endif
			}
			focusActor = nullptr;
		}

		if (WidgetInteraction) // Click
		{
			WidgetInteraction->SetWorldLocationAndRotation(worldLocation, worldRotation);
			tracedGraphics = WidgetInteraction->IsOverFocusableWidget();
			if (tracedGraphics)
			{
				UWidgetComponent* widget = WidgetInteraction->GetHoveredWidgetComponent();
				UUserWidget* user_widget = widget->GetUserWidgetObject();
				tracedWidgetPosition = (static_cast<USceneComponent *>(widget))->GetComponentLocation();

				// Change the beam length and pointer distance
				targetDistance = FVector::Distance(worldLocation, tracedWidgetPosition);
				//LOGD(LogWaveVRHandPointer, "TickComponent() Hovering widget: %s, targetDistance: %f", TCHAR_TO_ANSI(*user_widget->GetFullName()), targetDistance);
				targetPoint = (pinch_direction * targetDistance) + worldLocation;

				if (clicked)
				{
					WidgetInteraction->PressPointerKey(FKey("LeftMouseButton"));
					WidgetInteraction->ReleasePointerKey(FKey("LeftMouseButton"));
#if PLATFORM_ANDROID
					LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Click widget: %s", (uint8)m_Hand, TCHAR_TO_ANSI(*user_widget->GetFullName()));
#else
					LOGD(LogWaveVRHandPointer, "m_Hand %d, TickComponent() Click widget: %s", (uint8)m_Hand, *user_widget->GetFullName());
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

	FVector pointer_scale = SpotScale;
	if (targetDistance > k_DefaultPointerDistance)
		pointer_scale = SpotScale * (targetDistance / k_DefaultPointerDistance);
	if (SpotPointer)
	{
		SpotPointer->SetWorldLocation(targetPoint);
		SpotPointer->SetRelativeScale3D(pointer_scale);
	}
	if (SpotPointerBlue)
	{
		SpotPointerBlue->SetWorldLocation(targetPoint);
		SpotPointerBlue->SetRelativeScale3D(pointer_scale);
	}

	ActivateBeamPointer();
}

void UWaveVRHandPointer::ActivateBeamPointer()
{
	if (UWaveVRHandBPLibrary::GetHandConfidence(m_TrackerType, m_Hand) < .3f)
	{
		if (FadingParticle)
			FadingParticle->SetHiddenInGame(true, true);
		if (SpotPointer)
			SpotPointer->SetHiddenInGame(true, true);
		if (SpotPointerBlue)
			SpotPointerBlue->SetHiddenInGame(true, true);
		return;
	}
	else
	{
		if (FadingParticle)
			FadingParticle->SetHiddenInGame(false, true);
	}
	if (!tracedPhysics && !tracedGraphics)
	{
		if (SpotPointer)
			SpotPointer->SetHiddenInGame(true, true);
		if (SpotPointerBlue)
			SpotPointerBlue->SetHiddenInGame(true, true);
		return;
	}
	if (UWaveVRHandBPLibrary::GetHandPinchStrength(m_TrackerType, m_Hand) > m_PinchOnThreshold)
	{
		// Show the blue pointer when pinch on.
		if (SpotPointer)
			SpotPointer->SetHiddenInGame(true, true);
		if (SpotPointerBlue)
			SpotPointerBlue->SetHiddenInGame(false, true);
	}
	else
	{
		// Show the default pointer when pinch off.
		if (SpotPointer)
			SpotPointer->SetHiddenInGame(false, true);
		if (SpotPointerBlue)
			SpotPointerBlue->SetHiddenInGame(true, true);
	}
}
bool UWaveVRHandPointer::IsHandInteractable()
{
	bool hand_mode = (UWaveVRBlueprintFunctionLibrary::GetInteractionMode() == EWVRInteractionMode::Hand);
	// HandPointer is used along with Natural Hand tracker.
	bool valid_pose = UWaveVRHandBPLibrary::GetHandConfidence(EWaveVRTrackerType::Natural, m_Hand) > 0 ? true : false;
	bool focusCapturedBySystem = UWaveVRBlueprintFunctionLibrary::IsInputFocusCapturedBySystem();
	bool valid_motion = (UWaveVRHandBPLibrary::GetHandMotion(EWaveVRTrackerType::Natural, m_Hand) != EWaveVRHandMotion::Invalid);

	bool interactable = (AlwaysEnable || hand_mode)
		&& valid_pose
		&& (!focusCapturedBySystem)
		&& valid_motion;

	if (printIntervalLog)
	{
		LOGD(LogWaveVRHandPointer,
			"IsHandInteractable() m_Hand %d, interactable %d, AlwaysEnable %d, hand_mode %d, focusCapturedBySystem %d, valid_pose %d, valid_motion %d",
			(uint8)m_Hand, (uint8)interactable, (uint8)AlwaysEnable, (uint8)hand_mode, (uint8)focusCapturedBySystem, (uint8)valid_pose, (uint8)valid_motion);
	}

	if (WidgetInteraction)
	{
		if ((interactable && !WidgetInteraction->IsActive()) ||
			(!interactable && WidgetInteraction->IsActive()))
		{
			LOGD(LogWaveVRHandPointer, "IsHandInteractable() %d", (uint8)interactable);
		}
		WidgetInteraction->SetActive(interactable);
	}
	if (SpotPointer)
		SpotPointer->SetHiddenInGame(!interactable, true);
	if (SpotPointerBlue)
		SpotPointerBlue->SetHiddenInGame(!interactable, true);
	if (FadingParticle)
		FadingParticle->SetHiddenInGame(!interactable, true);

	return interactable;
}

bool UWaveVRHandPointer::IsClicked()
{
	bool pressed = UWaveVRHandBPLibrary::GetHandPinchStrength(m_TrackerType, m_Hand) > m_PinchOnThreshold;

	bool clicked = (pressState == false && pressed == true);
	pressState = pressed;

	return clicked;
}
