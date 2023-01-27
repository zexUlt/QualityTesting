// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mover.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class QUALITYTESTING_API AMover : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMover();

	UFUNCTION(BlueprintCallable)
	void SetAccelerationTime(float NewTime);

	UFUNCTION(BlueprintCallable)
	void SetDriftTime(float NewTime);

	UFUNCTION(BlueprintCallable)
	void SetDeccelerationTime(float NewTime);

	UFUNCTION(BlueprintCallable)
	void SetForceScale(float NewScale);

	UFUNCTION(BlueprintCallable)
	void SetTorqueScale(float NewScale);

	UFUNCTION(BlueprintCallable)
	void SetPerturbationsScale(float NewScale);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProbeMesh;

	UPROPERTY(Category = Configuration, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"), BlueprintSetter = SetAccelerationTime)
	float AccelerationTime;

	UPROPERTY(Category = Configuration, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"), BlueprintSetter = SetDriftTime)
	float DriftTime;

	UPROPERTY(Category = Configuration, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"), BlueprintSetter = SetDeccelerationTime)
	float DeccelerationTime;

	UPROPERTY(Category = Configuration, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"), BlueprintSetter = SetForceScale)
	float ForceScale;
	
	UPROPERTY(Category = Configuration, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"), BlueprintSetter = SetTorqueScale)
	float TorqueScale;

	UPROPERTY(Category = Configuration, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"), BlueprintSetter = SetTorqueScale)
	float PerturbationsScale;
};
