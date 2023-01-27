// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QualityTestingFL.generated.h"

/**
 * 
 */
UCLASS()
class QUALITYTESTING_API UQualityTestingFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static int32 FindSaddlePoint(const TArray<FVector>& Points,
	                             const FVector& RZ_LowerBound, const FVector& RZ_UpperBound,
	                             float& OutJ0);

	UFUNCTION(BlueprintCallable)
	static float EvaluateScore(const float& J0, const FVector& TestingPoint);

	static float DistanceToInterval(FVector Point, const TPair<FVector, FVector>& Interval, FVector& OutProjection);
};
