// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FileHandlerComponent.h"
#include "QualityTestingTypes.h"
#include "Components/ActorComponent.h"
#include "ExperimentExecutionComponent.generated.h"

USTRUCT(BlueprintType)
struct FExecutionComponentInitializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bIsExperimentCalibration;

	UPROPERTY(BlueprintReadWrite)
	int32 SaddlePointIndex;
	
	UPROPERTY(BlueprintReadWrite)
	EExperimentState InitialState;
	
	UPROPERTY(BlueprintReadWrite)
	float StartDelay;

	UPROPERTY(BlueprintReadWrite)
	float PhaseDelta;

	UPROPERTY(BlueprintReadWrite)
	float ControlForceAbs;

	UPROPERTY(BlueprintReadWrite)
	float ThrustForce;

	UPROPERTY(BlueprintReadWrite)
	AActor* TargetActor;

	UPROPERTY(BlueprintReadWrite)
	TArray<FPerturbationsInfo> Perturbations;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExecutionComponentFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExecutionComponentSinglePerturbationComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExecutionComponentSetupFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExecutionComponentUpdate, float, CurrentTime);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUALITYTESTING_API UExperimentExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UExperimentExecutionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetupComponent(const FExecutionComponentInitializer& ComponentInitializer);

	void FinalizeSetupComponent();
	void StartComponentWork();

	UFUNCTION(BlueprintPure)
	EExperimentState GetCurrentState() const;

	UFUNCTION(BlueprintPure)
	EMovementPhase GetCurrentPhase() const;

	UPROPERTY(BlueprintAssignable)
	FExecutionComponentFinished OnComponentFinished;

	UPROPERTY(BlueprintAssignable)
	FExecutionComponentSinglePerturbationComplete OnSinglePerturbationComplete;

	UPROPERTY(BlueprintAssignable)
	FExecutionComponentUpdate OnComponentUpdate;

	UPROPERTY(BlueprintAssignable)
	FExecutionComponentSetupFailed OnSetupFailed;
	
private:
	bool bIsCalibration;
	int32 CurrentPerturbationIndex;
	EExperimentState ExperimentState;
	EMovementPhase CurrentPhase;
	float StartDelay;
	float CurrentTime;
	float AccelerationTime;
	float DriftTime;
	float DecelerationTime;
	float ThrustForce;
	float ControlForceAbs;
	FVector InitialPosition;
	FRotator InitialRotation;
	FVector ActorInertiaTensor;
	TArray<FPerturbationsInfo> PerturbationsInfos;
	TWeakObjectPtr<UPrimitiveComponent> PhysicsBody;
	TWeakObjectPtr<AActor> TargetActor;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void ForwardMotion(float TimeDelta);
	void ControllableMotion(float TimeDelta);
	void PerturbedMotion(float TimeDelta);
	void ApplyControl() const;

	FVector NewtonsTorqueToRadians(FVector Direction, float TorqueScale) const;
};
