// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QualityTestingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUALITYTESTING_API UQualityTestingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQualityTestingComponent();

	UFUNCTION(BlueprintCallable)
	bool FindSaddlePoint(const TArray<FVector> Points,
		const FVector& MinBound, const FVector& MaxBound);

	UFUNCTION(BlueprintPure)
	float EvaluateScore(FVector TestingPoint) const;

	UFUNCTION(BlueprintPure)
	int32 GetSaddlePointIdx() const;

private:
	int32 SaddlePointIdx;
	float J0;
	FVector SaddlePoint;
	FVector MinReachableBound;
	FVector MaxReachableBound;
};
