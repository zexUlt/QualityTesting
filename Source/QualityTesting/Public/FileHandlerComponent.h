// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FileHandlerComponent.generated.h"


USTRUCT(BlueprintType)
struct QUALITYTESTING_API FExperimentTextLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ExperimentState;

	UPROPERTY(BlueprintReadWrite)
	FVector Location;

};

USTRUCT(BlueprintType)
struct QUALITYTESTING_API FPerturbationsInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TArray<float> PerturbationsScaleList;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<float> TorqueScaleList;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUALITYTESTING_API UFileHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFileHandlerComponent();

	UFUNCTION(BlueprintCallable)
	void AddToWriteQueue(const FExperimentTextLine& TextLine);

	UFUNCTION(BlueprintCallable)
	bool SaveToFile(const FString& FileName, const FString& FileExtension);

	UFUNCTION(BlueprintPure)
	TArray<FPerturbationsInfo> GetPerturbationsInfo(FVector& OutOffset) const;

	UFUNCTION(BlueprintCallable)
	bool ReadFromFile(const FString& FilePath);
	
private:
	static FString SerializeTextLine(const FExperimentTextLine& TextLine);
	static FPerturbationsInfo DeserializeTextLine(const FString& TextLine);

	TArray<FString> WriteCache;
	TArray<FString> ReadCache;
};
