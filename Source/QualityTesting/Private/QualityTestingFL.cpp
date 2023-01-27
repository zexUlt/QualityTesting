// Fill out your copyright notice in the Description page of Project Settings.


#include "QualityTestingFL.h"


int32 UQualityTestingFL::FindSaddlePoint(const TArray<FVector>& Points,
                                         const FVector& RZ_LowerBound, const FVector& RZ_UpperBound,
                                         float& OutJ0)
{
	const TPair<FVector, FVector> Interval(RZ_LowerBound, RZ_UpperBound);
	float MaxDistToRZ{ 0.f };
	int32 SaddlePointIndex{ -1 };
	FVector SaddlePointProj;

	for (int i = 0; i < Points.Num(); ++i)
	{
		FVector Proj;
		const float DistToRZ = DistanceToInterval(Points[i], Interval, Proj);
		if (MaxDistToRZ < DistToRZ) {
			MaxDistToRZ = DistToRZ;
			SaddlePointIndex = i;
			SaddlePointProj = Proj;
		}		
	}

	int32 PointsCovered{ 0 };
	for (int j = 0; j < Points.Num(); ++j)
	{
		if (FVector::Distance(SaddlePointProj, Points[j]) <= MaxDistToRZ)
		{
			PointsCovered++;
		}
		else
		{
			break;
		}
	}

	OutJ0 = MaxDistToRZ;
	
	if(PointsCovered != Points.Num())
	{
		SaddlePointIndex = -1;
		OutJ0 = NAN;
	}
	
	return SaddlePointIndex;
}

float UQualityTestingFL::EvaluateScore(const float& J0, const FVector& TestingPoint)
{
	return J0 / TestingPoint.Size() * 100.f;
}

float UQualityTestingFL::DistanceToInterval(FVector Point, const TPair<FVector, FVector>& Interval, FVector& OutProjection)
{
	const float LengthSquared = (Interval.Get<0>() - Interval.Get<1>()).SizeSquared();
	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line. 
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2
	// We clamp t from [0,1] to handle points outside the segment vw.
	const float Param = FMath::Max(0.f,
	                               FMath::Min(1.f,
	                                          FVector::DotProduct(
	                                          	Point - Interval.Get<0>(),
	                                            Interval.Get<1>() - Interval.Get<0>()) /
	                                          LengthSquared));
	OutProjection = Interval.Get<0>() + Param * (Interval.Get<1>() - Interval.Get<0>());

	return FVector::Distance(Point, OutProjection);
}
