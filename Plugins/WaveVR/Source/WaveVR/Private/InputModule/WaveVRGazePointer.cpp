// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "InputModule/WaveVRGazePointer.h"
#include "InputModule/WaveVRInteractInterface.h"
#include "Eye/WaveVREyeBPLibrary.h"
#include "WaveVRHMD.h"

#include "CollisionQueryParams.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Blueprint.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "EyeTrackerFunctionLibrary.h"
#include "XRTrackingSystemBase.h"

#include "Platforms/WaveVRLogWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRGazePointer, Log, All);

UWaveVRGazePointer::UWaveVRGazePointer()
	: SpotPointer(nullptr)
	, RingPointer(nullptr)
	, RollingRingPointer(nullptr)
	, WidgetInteraction(nullptr)
	, spotPointerMesh(nullptr)
	, spotPointerMaterial(nullptr)
	, ringPointerMesh(nullptr)
	, ringPointerMaterial(nullptr)
	, rollingRingPointerMesh(nullptr)
	, rollingRingPointerMaterial(nullptr)
	, worldLocation(FVector::ZeroVector)
	, worldRotation(FRotator::ZeroRotator)
	, tracedPhysics(false)
	, hit(ForceInit)
	, focusActor(nullptr)
	, tracedWidget(false)
	, tracedWidgetPosition(FVector::ZeroVector)
	, gazeOnTime(0)
	, pressState(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	FString reticle_mesh = TEXT("StaticMesh'/WaveVR/Shapes/Reticle_Mesh.Reticle_Mesh'");
	spotPointerMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *reticle_mesh));
	FString m_reticle = TEXT("Material'/WaveVR/Materials/M_Reticle.M_Reticle'");
	spotPointerMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *m_reticle));
	//SpotPointer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotPointer"));
	SpotPointer = NewObject<UStaticMeshComponent>(this, TEXT("SpotPointer"));
	SpotPointer->SetupAttachment(this);

	FString static_circle = TEXT("StaticMesh'/WaveVR/Shapes/StaticCircle.StaticCircle'");
	ringPointerMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *static_circle));
	FString m_staticcircle = TEXT("Material'/WaveVR/Materials/M_Circle.M_Circle'");
	ringPointerMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *m_staticcircle));
	//RingPointer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingPointer"));
	RingPointer = NewObject<UStaticMeshComponent>(this, TEXT("RingPointer"));
	RingPointer->SetupAttachment(this);

	FString rolling_circle = TEXT("StaticMesh'/WaveVR/Shapes/SpinningCircle.SpinningCircle'");
	rollingRingPointerMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *rolling_circle));
	FString m_rollingcircle = TEXT("Material'/WaveVR/Materials/M_RollingCircle.M_RollingCircle'");
	rollingRingPointerMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *m_rollingcircle));
	//RollingRingPointer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RollingRingPointer"));
	RollingRingPointer = NewObject<UStaticMeshComponent>(this, TEXT("RollingRingPointer"));
	RollingRingPointer->SetupAttachment(this);

	//WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
	WidgetInteraction = NewObject<UWidgetInteractionComponent>(this, TEXT("WidgetInteraction"));
	WidgetInteraction->SetupAttachment(this);
}

void UWaveVRGazePointer::CheckLogInterval()
{
	logFrame++;
	logFrame = logFrame % klogFrameCount;
	if (logFrame == 0)
		printIntervalLog = true;
	else
		printIntervalLog = false;
}

#pragma region
void UWaveVRGazePointer::SetWidgetPointerIndex(int32 pointerIndex)
{
	if (WidgetInteraction)
	{
		WidgetPointerIndex = pointerIndex;
		WidgetInteraction->PointerIndex = WidgetPointerIndex;
		LOGD(LogWaveVRGazePointer, "SetWidgetPointerIndex() %d", WidgetInteraction->PointerIndex);
	}
}
void UWaveVRGazePointer::SetTimerEnable(bool enable)
{
	TimerControl = enable;
}
void UWaveVRGazePointer::SetupGazeTimer(float duration)
{
	GazeTimer = duration;
}
void UWaveVRGazePointer::SetButtonEnable(bool enable)
{
	ButtonControl = enable;
}
void UWaveVRGazePointer::SetupGazeButton(const TArray<EWVR_InputId> leftButtons, const TArray<EWVR_InputId> rightButtons)
{
	if (rightButtons.Num() > 0)
		RightClickButtons = rightButtons;
	if (leftButtons.Num() > 0)
		LeftClickButtons = leftButtons;
}
#pragma endregion Public APIs

void UWaveVRGazePointer::UpdateCustomizedVariables()
{
	if (m_RightClickButtons != RightClickButtons || m_LeftClickButtons != LeftClickButtons)
	{
		m_RightClickButtons = RightClickButtons;
		m_LeftClickButtons = LeftClickButtons;

		for (int i = 0; i < m_RightClickButtons.Num(); i++)
			LOGD(LogWaveVRGazePointer, "UpdateCustomizedVariables() ButtonControl %d, m_RightClickButtons[%d]=%d", (uint8)ButtonControl, i, (uint8)m_RightClickButtons[i]);
		for (int i = 0; i < m_LeftClickButtons.Num(); i++)
			LOGD(LogWaveVRGazePointer, "UpdateCustomizedVariables() ButtonControl %d, m_LeftClickButtons[%d]=%d", (uint8)ButtonControl, i, (uint8)m_LeftClickButtons[i]);
	}

	if (m_GazeTimer != GazeTimer)
	{
		m_GazeTimer = GazeTimer;
		m_GazeTimer = FMath::Clamp(m_GazeTimer, kGazeTimerMin, kGazeTimerMax);
		LOGD(LogWaveVRGazePointer, "UpdateCustomizedVariables() TimerControl %d,  m_GazeTimer %f", (uint8)TimerControl, m_GazeTimer);
	}
}

void UWaveVRGazePointer::BeginPlay()
{
	Super::BeginPlay();

	m_GazeTimer = GazeTimer;
	m_RightClickButtons = RightClickButtons;
	m_LeftClickButtons = LeftClickButtons;

	ConfigureResources();
	ForceUpdateGazeType();
}
void UWaveVRGazePointer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckLogInterval();

	UpdateCustomizedVariables();
	UpdateGazeType();

	if (!IsGazeInteractable())
		return;

        //Prevent users from interacting while transition is showing.
        FWaveVRHMD* HMD = FWaveVRHMD::GetInstance();
        if (HMD != nullptr && HMD->IsVRTransitionShowing())
                gazeOnTime = UGameplayStatics::GetTimeSeconds(GetWorld());

	if (GEngine) {
		if (GEngine->XRSystem)
			GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, CameraRotation, CameraPosition);
	}

	worldLocation = GetComponentLocation();// GetOwner()->GetActorLocation();
	worldRotation = GetEyeRotation();// GetComponentRotation();
	m_EyeVector = GetEyeVector();
	FVector end_pos = (m_EyeVector * TraceDistance) + worldLocation;

	bool clicked = IsClicked();

	FCollisionQueryParams CollisionParams;
	SetIgnoreActors(CollisionParams);
	tracedPhysics = GetWorld()->LineTraceSingleByChannel(hit, worldLocation, end_pos, ECC_Camera, CollisionParams);
	//DrawDebugLine(GetWorld(), worldLocation, end_pos, FColor::Green, false, 0.5f);
	if (tracedPhysics)
	{
		AActor* hit_actor = hit.GetActor();
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Trace hit: %s"), *hit.GetActor()->GetName()));
		//LOGD(LogWaveVRGazePointer, "TickComponent() Hitting: %s (%s)", *hit_actor->GetName(), *hit.ImpactPoint.ToString());

		if (focusActor != hit_actor) // Enter & Exit
		{
			if (focusActor && (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
			{
				// Exit previous actor
				IWaveVRInteractInterface::Execute_OnExit(focusActor, GetOwner());
#if PLATFORM_ANDROID
				LOGD(LogWaveVRGazePointer, "TickComponent() Exit: %s", TCHAR_TO_ANSI(*focusActor->GetName()));
#else
				LOGD(LogWaveVRGazePointer, "TickComponent() Exit: %s", *focusActor->GetName());
#endif
			}
			if (hit_actor && (hit_actor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
			{
				// Enter current actor
				IWaveVRInteractInterface::Execute_OnEnter(hit_actor, GetOwner());
#if PLATFORM_ANDROID
				LOGD(LogWaveVRGazePointer, "TickComponent() Enter: %s", TCHAR_TO_ANSI(*hit_actor->GetName()));
#else
				LOGD(LogWaveVRGazePointer, "TickComponent() Enter: %s", *hit_actor->GetName());
#endif

				// Reset the timer when switching the physical gaze target.
				gazeOnTime = UGameplayStatics::GetTimeSeconds(GetWorld());
			}

			focusActor = hit_actor;
		}
		else // Hover & Click
		{
			if (focusActor && (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass())))
			{
				IWaveVRInteractInterface::Execute_OnHover(focusActor, GetOwner());
				if (clicked || IsTimeout())
				{
					IWaveVRInteractInterface::Execute_OnClick(focusActor, GetOwner());
#if PLATFORM_ANDROID
					LOGD(LogWaveVRGazePointer, "TickComponent() Click: %s", TCHAR_TO_ANSI(*hit_actor->GetName()));
#else
					LOGD(LogWaveVRGazePointer, "TickComponent() Click: %s", *hit_actor->GetName());
#endif

					// Reset the gaze timer on click.
					gazeOnTime = UGameplayStatics::GetTimeSeconds(GetWorld());
				}
			}
		}
	}
	else
	{
		if (focusActor)
		{
			if (focusActor->GetClass()->ImplementsInterface(UWaveVRInteractInterface::StaticClass()))
			{
				// Exit previous actor
				IWaveVRInteractInterface::Execute_OnExit(focusActor, GetOwner());
				LOGD(LogWaveVRGazePointer, "TickComponent() Exit: %s", TCHAR_TO_ANSI(*focusActor->GetName()));
			}
			focusActor = nullptr;
		}

		if (WidgetInteraction) // Click
		{
			WidgetInteraction->SetWorldLocationAndRotation(worldLocation, worldRotation);
			tracedWidget = WidgetInteraction->IsOverFocusableWidget();
			if (tracedWidget)
			{
				UWidgetComponent* widget = WidgetInteraction->GetHoveredWidgetComponent();
				UUserWidget* user_widget = widget->GetUserWidgetObject();
				tracedWidgetPosition = (static_cast<USceneComponent *>(widget))->GetComponentLocation();
				if (clicked || IsTimeout())
				{
					WidgetInteraction->PressPointerKey(FKey("LeftMouseButton"));
					WidgetInteraction->ReleasePointerKey(FKey("LeftMouseButton"));
					LOGD(LogWaveVRGazePointer, "TickComponent() Click widget: %s", TCHAR_TO_ANSI(*user_widget->GetFullName()));

					// Reset the gaze timer on click.
					gazeOnTime = UGameplayStatics::GetTimeSeconds(GetWorld());
				}
			}
			else
			{
				// Reset the timer when not gazing on any widget.
				gazeOnTime = UGameplayStatics::GetTimeSeconds(GetWorld());
			}
		}
		else
		{
			tracedWidget = false;
		}
	}

	ActivatePointer();
}

void UWaveVRGazePointer::ConfigureResources()
{
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

		LOGD(LogWaveVRGazePointer, "ConfigureResources() WidgetInteraction");
	}

	if (SpotPointer)
	{
		if (spotPointerMesh)
		{
			SpotPointer->SetStaticMesh(spotPointerMesh);
			LOGD(LogWaveVRGazePointer, "ConfigureResources() spotPointerMesh");
		}
		if (spotPointerMaterial)
		{
			SpotPointer->SetMaterial(0, spotPointerMaterial);
			LOGD(LogWaveVRGazePointer, "ConfigureResources() spotPointerMaterial");
		}
		SpotPointer->SetRelativeRotation(SpotRotationOffset);
		SpotPointer->SetRelativeScale3D(SpotScale);
		SpotPointer->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

		LOGD(LogWaveVRGazePointer, "ConfigureResources() SpotPointer");
	}

	if (RingPointer)
	{
		if (ringPointerMesh)
		{
			RingPointer->SetStaticMesh(ringPointerMesh);
			LOGD(LogWaveVRGazePointer, "ConfigureResources() ringPointerMesh");
		}
		if (ringPointerMaterial)
		{
			RingPointer->SetMaterial(0, ringPointerMaterial);
			LOGD(LogWaveVRGazePointer, "ConfigureResources() ringPointerMaterial");
		}
		RingPointer->SetRelativeRotation(RingRotationOffset);
		RingPointer->SetRelativeScale3D(RingScale);
		RingPointer->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		RingPointer->SetHiddenInGame(true, true);

		LOGD(LogWaveVRGazePointer, "ConfigureResources() RingPointer");
	}

	if (RollingRingPointer)
	{
		if (rollingRingPointerMesh)
		{
			RollingRingPointer->SetStaticMesh(rollingRingPointerMesh);
			LOGD(LogWaveVRGazePointer, "ConfigureResources() rollingRingPointerMesh");
		}
		if (rollingRingPointerMaterial)
		{
			RollingRingPointer->SetMaterial(0, rollingRingPointerMaterial);
			LOGD(LogWaveVRGazePointer, "ConfigureResources() rollingRingPointerMaterial");
		}
		RollingRingPointer->SetRelativeRotation(RingRotationOffset);
		RollingRingPointer->SetRelativeScale3D(RingScale);
		RollingRingPointer->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		RollingRingPointer->SetHiddenInGame(true, true);

		LOGD(LogWaveVRGazePointer, "ConfigureResources() RollingRingPointer");
	}
}
void UWaveVRGazePointer::ActivatePointer()
{
	if (SpotPointer)
	{
		SpotPointer->SetWorldLocation((m_EyeVector * SpotDistance) + worldLocation);
		if (tracedPhysics || tracedWidget)
			SpotPointer->SetHiddenInGame(true, true);
		else
			SpotPointer->SetHiddenInGame(false, true);
	}
	if (RingPointer)
	{
		RingPointer->SetWorldLocation((m_EyeVector * RingDistance) + worldLocation);
		RingPointer->SetWorldRotation(worldRotation + RingRotationOffset);
		if (TimerControl)
			RingPointer->SetHiddenInGame(true, true);
		else
		{
			if (tracedPhysics || tracedWidget)
				RingPointer->SetHiddenInGame(false, true);
			else
				RingPointer->SetHiddenInGame(true, true);
		}
	}
	if (RollingRingPointer)
	{
		RollingRingPointer->SetWorldLocation((m_EyeVector * RingDistance) + worldLocation);
		RollingRingPointer->SetWorldRotation(worldRotation + RingRotationOffset);
		if (!TimerControl)
			RollingRingPointer->SetHiddenInGame(true, true);
		else
		{
			if (tracedPhysics || tracedWidget)
				RollingRingPointer->SetHiddenInGame(false, true);
			else
				RollingRingPointer->SetHiddenInGame(true, true);
		}
	}
}
bool UWaveVRGazePointer::IsGazeInteractable()
{
	bool gaze_mode = (UWaveVRBlueprintFunctionLibrary::GetInteractionMode() == EWVRInteractionMode::Gaze);
	bool focusCapturedBySystem = UWaveVRBlueprintFunctionLibrary::IsInputFocusCapturedBySystem();

	bool interactable = (AlwaysEnable || gaze_mode)
		&& (!focusCapturedBySystem);

	if (printIntervalLog)
	{
		LOGD(LogWaveVRGazePointer,
			"IsGazeInteractable() interactable %d, AlwaysEnable %d, gaze_mode %d, focusCapturedBySystem %d",
			(uint8)interactable, (uint8)AlwaysEnable, (uint8)gaze_mode, (uint8)focusCapturedBySystem);
	}

	if (WidgetInteraction)
	{
		if ((interactable && !WidgetInteraction->IsActive()) ||
			(!interactable && WidgetInteraction->IsActive()))
		{
			LOGD(LogWaveVRGazePointer, "IsGazeInteractable() %d", (uint8)interactable);
		}
		WidgetInteraction->SetActive(interactable);
	}
	if (!interactable)
	{
		if (SpotPointer)
			SpotPointer->SetHiddenInGame(true, true);
		if (RingPointer)
			RingPointer->SetHiddenInGame(true, true);
		if (RollingRingPointer)
			RollingRingPointer->SetHiddenInGame(true, true);
	}

	return interactable;
}

void UWaveVRGazePointer::SetIgnoreActors(FCollisionQueryParams& collisionParams)
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
			LOGD(LogWaveVRGazePointer, "SetIgnoreActors() %s", *actor->GetFullName());
		}
	}*/

	// Ignore the Pawn.
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		APawn* pawn = UGameplayStatics::GetPlayerPawn(Context.World(), PlayerIndex);
		if (pawn)
		{
			collisionParams.AddIgnoredActor(pawn);
			//LOGD(LogWaveVRGazePointer, "SetIgnoreActors() %s", *pawn->GetFullName());
		}
	}

	// Ignore parent actor.
	USceneComponent* parent = GetAttachParent();
	while (parent)
	{
		collisionParams.AddIgnoredActor(parent->GetOwner());
		//LOGD(LogWaveVRGazePointer, "SetIgnoreActors() %s", *parent->GetOwner()->GetFullName());
		parent = parent->GetAttachParent();
	}
}

FVector UWaveVRGazePointer::GetEyeVector()
{
	FVector vec = GetForwardVector(); // Forward vector = camera rotation * (1, 0, 0)
	EWVR_CoordinateSystem space = UWaveVREyeBPLibrary::GetEyeSpace();

	if (EyeTracking && UEyeTrackerFunctionLibrary::GetGazeData(m_GazeData))
	{
		vec = m_GazeData.GazeDirection;

		if (space == EWVR_CoordinateSystem::Local)
			vec = CameraRotation * vec;
	}
	else
	{
		vec = 100 * vec;
	}

	if (printIntervalLog)
	{
		LOGD(LogWaveVRGazePointer, "GetEyeVector() EyeTracking %d, space %d, vec (%f, %f, %f)", (uint8)EyeTracking, (uint8)space, vec.X, vec.Y, vec.Z);
	}

	return vec;
}
FRotator UWaveVRGazePointer::GetEyeRotation()
{
	FRotator rot = GetComponentRotation();
	FVector eyeVector = GetEyeVector();

	rot = UKismetMathLibrary::MakeRotFromX(eyeVector);
	return rot;
}

#pragma region
void UWaveVRGazePointer::ForceUpdateGazeType()
{
	if (TimerControl && ButtonControl)
	{
		m_GazeType = EWVRGazeTriggerType::TimeoutButton;
	}
	else if (ButtonControl)
	{
		m_GazeType = EWVRGazeTriggerType::Button;
	}
	else if (TimerControl)
	{
		m_GazeType = EWVRGazeTriggerType::Timeout;
	}
	else // !m_TimerControl && !m_ButtonControl
	{
		TimerControl = true;
		m_GazeType = EWVRGazeTriggerType::Timeout;
	}

	m_TimerControl = TimerControl;
	m_ButtonControl = ButtonControl;
	m_GazeTypeEx = m_GazeType;

	bool ret = UWaveVRBlueprintFunctionLibrary::SetGazeTriggerType(m_GazeType);
	LOGD(LogWaveVRGazePointer, "ForceUpdateGazeType() Set gaze trigger type to %d, result %d", (uint8)m_GazeType, (uint8)ret);
}
void UWaveVRGazePointer::UpdateGazeType()
{
	if (m_TimerControl != TimerControl || m_ButtonControl != ButtonControl)
	{
		LOGD(LogWaveVRGazePointer, "UpdateGazeType()");
		ForceUpdateGazeType();
	}
	else
	{
		m_GazeType = UWaveVRBlueprintFunctionLibrary::GetGazeTriggerType();
		if (m_GazeTypeEx != m_GazeType)
		{
			m_GazeTypeEx = m_GazeType;
			LOGD(LogWaveVRGazePointer, "UpdateGazeType() Gaze trigger type is set to %d", (uint8)m_GazeType);

			switch (m_GazeType)
			{
			case EWVRGazeTriggerType::Timeout:
				TimerControl = true;
				ButtonControl = false;
				break;
			case EWVRGazeTriggerType::Button:
				TimerControl = false;
				ButtonControl = true;
				break;
			case EWVRGazeTriggerType::TimeoutButton:
				TimerControl = true;
				ButtonControl = true;
				break;
			default:
				break;
			}

			m_TimerControl = TimerControl;
			m_ButtonControl = ButtonControl;
		}
	}
}
bool UWaveVRGazePointer::IsTimeout()
{
	if (!TimerControl)
		return false;

	return UGameplayStatics::GetTimeSeconds(GetWorld()) > gazeOnTime + GazeTimer;
}
bool UWaveVRGazePointer::IsClicked()
{
	if (!ButtonControl)
		return false;

	bool pressed = false;
	int index = 0;
	while (!pressed && index < m_RightClickButtons.Num())
	{
		pressed = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_Controller_Right, m_RightClickButtons[index++]);
	}
	index = 0;
	while (!pressed && index < m_LeftClickButtons.Num())
	{
		pressed = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_Controller_Left, m_LeftClickButtons[index++]);
	}

	bool clicked = (pressState == false && pressed == true);
	pressState = pressed;

	return clicked;
}
#pragma endregion Trigger Type
