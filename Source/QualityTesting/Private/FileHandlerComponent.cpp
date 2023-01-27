// Fill out your copyright notice in the Description page of Project Settings.


#include "FileHandlerComponent.h"
#include "Misc/Paths.h"

// Sets default values for this component's properties
UFileHandlerComponent::UFileHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UFileHandlerComponent::AddToWriteQueue(const FExperimentTextLine& TextLine)
{
	this->WriteCache.Add(SerializeTextLine(TextLine));
}


bool UFileHandlerComponent::ReadFromFile(const FString& FilePath)
{
	auto& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	bool bSuccess = false;

	if (FileManager.FileExists(*FilePath)) {
		bSuccess = FFileHelper::LoadFileToStringArray(this->ReadCache, *FilePath);

		if (bSuccess) {
			UE_LOG(LogTemp, Log, TEXT("File successfully read."));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Something went wrong. Couldn't read file %s."), *FilePath);
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Cannot open %s for reading. File does not exist."), *FilePath);
	}

	return bSuccess;
}

FString UFileHandlerComponent::SerializeTextLine(const FExperimentTextLine& TextLine)
{
	return FString::Printf(TEXT("%s, %f, %f, %f"), *TextLine.ExperimentState, TextLine.Location.X, TextLine.Location.Y, TextLine.Location.Z);

}

FPerturbationsInfo UFileHandlerComponent::DeserializeTextLine(const FString& TextLine)
{
	FPerturbationsInfo Ret;
	TArray<FString> Splitted;
	
	TextLine.ParseIntoArray(Splitted, TEXT(","));

	for (int i = 0; i < 2; ++i) {
		Ret.PerturbationsScaleList.Add(FCString::Atof(*Splitted[i]));
	}

	for (int i = 0; i < 2; ++i) {
		Ret.TorqueScaleList.Add(FCString::Atof(*Splitted[2 + i]));
	}

	return Ret;
}

bool UFileHandlerComponent::SaveToFile(const FString& FileName, const FString& FileExtension)
{
	const FString BaseDir = FPaths::Combine(FPaths::ProjectSavedDir(), "Results");
	FString AbsolutePath = FPaths::Combine(BaseDir, FString::Printf(TEXT("%s.%s"), *FileName, *FileExtension));
	
	auto& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	bool bSuccess = FFileHelper::SaveStringArrayToFile(this->WriteCache, *AbsolutePath);

	if (!bSuccess) {
		UE_LOG(LogTemp, Error, TEXT("Couldn't write to file %s. Write failed."), *AbsolutePath);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Successfully saved file %s."), *AbsolutePath);
	}

	return bSuccess;
}

TArray<FPerturbationsInfo> UFileHandlerComponent::GetPerturbationsInfo(FVector& OutOffset) const
{
	TArray<FPerturbationsInfo> Ret;

	for (int32 i = 0; i < ReadCache.Num(); ++i) {
		if (i == 0) {
			TArray<FString> Splitted;
			ReadCache[i].ParseIntoArray(Splitted, TEXT(","));
			OutOffset.X = FCString::Atof(*Splitted[0]);
			OutOffset.Y = FCString::Atof(*Splitted[1]);
			OutOffset.Z = FCString::Atof(*Splitted[2]);
		}
		else {
			Ret.Add(DeserializeTextLine(ReadCache[i]));
		}
	}

	return Ret;
}
