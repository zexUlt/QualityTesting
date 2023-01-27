// Fill out your copyright notice in the Description page of Project Settings.


#include "QualityTestingComponent.h"

#include "QualityTestingFL.h"

// Sets default values for this component's properties
UQualityTestingComponent::UQualityTestingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

bool UQualityTestingComponent::FindSaddlePoint(const TArray<FVector> Points, const FVector& MinBound,
	const FVector& MaxBound)
{
	MinReachableBound = MinBound;
	MaxReachableBound = MaxBound;

	SaddlePointIdx = UQualityTestingFL::FindSaddlePoint(Points, MinBound, MaxBound, J0);

	if (SaddlePointIdx != -1) {
		SaddlePoint = Points[SaddlePointIdx];
	}

	return SaddlePointIdx != -1;
}

float UQualityTestingComponent::EvaluateScore(FVector TestingPoint) const
{
	return UQualityTestingFL::EvaluateScore(J0, TestingPoint);
}

int32 UQualityTestingComponent::GetSaddlePointIdx() const
{
	return SaddlePointIdx;
}

