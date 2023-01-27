#pragma once

UENUM(BlueprintType)
enum class EExperimentState : uint8
{
	WaitingForStart 	UMETA(DisplayName="WaitingForStart", Hidden),
	AdjustOrigin		UMETA(DisplayName="AdjustOrigin"),
	ReachableZoneSearch	UMETA(DisplayName="ReachableZoneSearch"),
	PerturbedMotion 	UMETA(DisplayName="PerturbedMotion"),
	Finished			UMETA(DisplayName="Finished"),
};

UENUM(BlueprintType)
enum class EMovementPhase : uint8
{
	Acceleration	UMETA(DisplayName="Acceleration"),
	Drift			UMETA(DisplayName="Drift"),
	Decceleration	UMETA(DisplayName="Decceleration"),
	Finished		UMETA(DisplayName="Finished"),
	Idle			UMETA(DisplayName="Idle"),
};
