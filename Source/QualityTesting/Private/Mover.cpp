// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

// Sets default values
AMover::AMover()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> ProbeMesh;
		FConstructorStatics() :
			ProbeMesh(TEXT("StaticMesh'/Game/Geometry/Meshes/Probe/Drone_Scout.Drone_Scout'"))
		{

		}
	};
	static FConstructorStatics ConstructorStatics;

	ProbeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProbeMesh"));
	ProbeMesh->SetStaticMesh(ConstructorStatics.ProbeMesh.Get());
	ProbeMesh->SetSimulatePhysics(true);
	ProbeMesh->SetEnableGravity(false);
	// ProbeMesh->SetMassOverrideInKg(NAME_None, 225.f, true);
	ProbeMesh->BodyInstance.bOverrideMass = true;
	ProbeMesh->BodyInstance.SetMassOverride(225.f);
	RootComponent = ProbeMesh;
}

void AMover::SetAccelerationTime(float NewTime)
{
	this->AccelerationTime = NewTime;
}

void AMover::SetDriftTime(float NewTime)
{
	this->DriftTime = NewTime;
}

void AMover::SetDeccelerationTime(float NewTime)
{
	this->DeccelerationTime = NewTime;
}

void AMover::SetForceScale(float NewScale)
{
	this->ForceScale = NewScale;
}

void AMover::SetTorqueScale(float NewScale)
{
	this->TorqueScale = NewScale;
}

void AMover::SetPerturbationsScale(float NewScale)
{
	this->PerturbationsScale = NewScale;
}

// Called when the game starts or when spawned
void AMover::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMover::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

