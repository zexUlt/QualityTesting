// Fill out your copyright notice in the Description page of Project Settings.


#include "ExperimentExecutionComponent.h"

#include "PhysicsBodyInterface.h"

// Sets default values for this component's properties
UExperimentExecutionComponent::UExperimentExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bIsCalibration = false;
	CurrentPerturbationIndex = 0;
	ExperimentState = EExperimentState::WaitingForStart;
	CurrentPhase = EMovementPhase::Acceleration;
	StartDelay = CurrentTime = AccelerationTime =
		DriftTime = DecelerationTime = ThrustForce = ControlForceAbs = 0.f;
	InitialPosition = ActorInertiaTensor = FVector::ZeroVector;
	InitialRotation = FRotator::ZeroRotator;
	TargetActor = nullptr;
	PhysicsBody = nullptr;
}


// Called when the game starts
void UExperimentExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, "Component Begin play");
}

void UExperimentExecutionComponent::ForwardMotion(float TimeDelta)
{
	switch (CurrentPhase)
	{
	case EMovementPhase::Acceleration:
		{
			if (CurrentTime <= AccelerationTime)
			{
				PhysicsBody->AddForce(PhysicsBody->GetForwardVector() * ThrustForce);
			}
			else
			{
				CurrentPhase = EMovementPhase::Drift;
			}
		}
		break;
	case EMovementPhase::Drift:
		{
			if (CurrentTime > DriftTime)
			{
				CurrentPhase = EMovementPhase::Decceleration;
			}
		}
		break;
	case EMovementPhase::Decceleration:
		{
			if (CurrentTime <= DecelerationTime)
			{
				PhysicsBody->AddForce(PhysicsBody->GetForwardVector() * (-ThrustForce));
			}
			else
			{
				CurrentPhase = EMovementPhase::Finished;
			}
		}
		break;
	case EMovementPhase::Finished:
		{
			PhysicsBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
			PhysicsBody->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
			CurrentPhase = EMovementPhase::Idle;
		}
		break;
	case EMovementPhase::Idle:
		ExperimentState = EExperimentState::Finished;
		break;
	}

	CurrentTime += TimeDelta;
}

void UExperimentExecutionComponent::ControllableMotion(float TimeDelta)
{
	ApplyControl();
	ForwardMotion(TimeDelta);
}

void UExperimentExecutionComponent::PerturbedMotion(float TimeDelta)
{
	TArray<float> Torques, Perturbations;
	if (CurrentPerturbationIndex < PerturbationsInfos.Num())
	{
		Torques = PerturbationsInfos[CurrentPerturbationIndex].TorqueScaleList;
		Perturbations = PerturbationsInfos[CurrentPerturbationIndex].PerturbationsScaleList;
	}

	switch (CurrentPhase)
	{
	case EMovementPhase::Acceleration:
		{
			if (CurrentTime <= AccelerationTime)
			{
				if (Perturbations.Num() > 0 && Torques.Num() > 0)
				{
					PhysicsBody->AddForce(PhysicsBody->GetForwardVector() * (ThrustForce + Perturbations[0]));
					PhysicsBody->AddTorqueInRadians(
						NewtonsTorqueToRadians(PhysicsBody->GetUpVector(), Torques[0])
					);
				}
			}
			else
			{
				CurrentPhase = EMovementPhase::Drift;
			}
		}
		break;
	case EMovementPhase::Drift:
		{
			if (CurrentTime > DriftTime)
			{
				CurrentPhase = EMovementPhase::Decceleration;
			}
		}
		break;
	case EMovementPhase::Decceleration:
		{
			if (CurrentTime <= DecelerationTime)
			{
				if (Perturbations.Num() > 1 && Torques.Num() > 1)
				{
					PhysicsBody->AddForce(PhysicsBody->GetForwardVector() * (Perturbations[1] - ThrustForce));
					PhysicsBody->AddTorqueInRadians(
						NewtonsTorqueToRadians(PhysicsBody->GetUpVector(), Torques[1])
					);
				}
			}
			else
			{
				CurrentPhase = EMovementPhase::Finished;
			}
		}
		break;
	case EMovementPhase::Finished:
		{
			if (bIsCalibration)
			{
				if (CurrentPerturbationIndex < PerturbationsInfos.Num() - 1)
				{
					CurrentPerturbationIndex++;
					CurrentPhase = EMovementPhase::Acceleration;
					CurrentTime = 0.f;
					PhysicsBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
					PhysicsBody->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
					OnSinglePerturbationComplete.Broadcast();
					TargetActor->SetActorLocation(InitialPosition);
					TargetActor->SetActorRotation(InitialRotation);
				}
				else
				{
					ExperimentState = EExperimentState::Finished;
					OnSinglePerturbationComplete.Broadcast();
				}
			}
			else
			{
				ExperimentState = EExperimentState::Finished;
			}
		}
		break;
	}

	CurrentTime += TimeDelta;
}

void UExperimentExecutionComponent::ApplyControl() const
{
	PhysicsBody->AddForce(PhysicsBody->GetRightVector() * ControlForceAbs);
}

FVector UExperimentExecutionComponent::NewtonsTorqueToRadians(FVector Direction, float TorqueScale) const
{
	return (Direction * TorqueScale) / ActorInertiaTensor;
}


// Called every frame
void UExperimentExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OnComponentUpdate.Broadcast(CurrentTime);

	switch (ExperimentState)
	{
	case EExperimentState::AdjustOrigin:
		ForwardMotion(DeltaTime);
		break;
	case EExperimentState::ReachableZoneSearch:
		ControllableMotion(DeltaTime);
		break;
	case EExperimentState::PerturbedMotion:
		PerturbedMotion(DeltaTime);
		break;
	case EExperimentState::Finished:
		OnComponentFinished.Broadcast();
		break;
	}
}

void UExperimentExecutionComponent::SetupComponent(const FExecutionComponentInitializer& ComponentInitializer)
{
	CurrentTime = 0.f;
	ExperimentState = ComponentInitializer.InitialState;
	StartDelay = ComponentInitializer.StartDelay;
	AccelerationTime = ComponentInitializer.PhaseDelta;
	DriftTime = AccelerationTime + ComponentInitializer.PhaseDelta;
	DecelerationTime = DriftTime + ComponentInitializer.PhaseDelta;
	ThrustForce = ComponentInitializer.ThrustForce;
	ControlForceAbs = ComponentInitializer.ControlForceAbs;
	PerturbationsInfos = ComponentInitializer.Perturbations;
	bIsCalibration = ComponentInitializer.bIsExperimentCalibration;
	CurrentPerturbationIndex = bIsCalibration ? 0 : ComponentInitializer.SaddlePointIndex;

	TargetActor = ComponentInitializer.TargetActor;
	InitialPosition = TargetActor->GetActorLocation();
	InitialRotation = TargetActor->GetActorRotation();

	if (TargetActor->GetClass()->ImplementsInterface(UPhysicsBodyInterface::StaticClass()))
	{
		PhysicsBody = IPhysicsBodyInterface::Execute_GetPhysicsBody(TargetActor.Get());
		ActorInertiaTensor = IPhysicsBodyInterface::Execute_GetInertiaTensor(TargetActor.Get());
		FinalizeSetupComponent();
	}
	else
	{
		OnSetupFailed.Broadcast();
	}
}

void UExperimentExecutionComponent::FinalizeSetupComponent()
{
	if (StartDelay > 0)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UExperimentExecutionComponent::StartComponentWork,
		                                       StartDelay);
	}
	else
	{
		StartComponentWork();
	}
}

void UExperimentExecutionComponent::StartComponentWork()
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, "Component Start");
	SetComponentTickEnabled(true);
}

EExperimentState UExperimentExecutionComponent::GetCurrentState() const
{
	return ExperimentState;
}

EMovementPhase UExperimentExecutionComponent::GetCurrentPhase() const
{
	return CurrentPhase;
}
