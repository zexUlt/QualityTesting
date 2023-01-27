// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "Logging/LogMacros.h"

#include "ProjectedPassthroughComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ProjPT, Log, All);

UCLASS( ClassGroup=(WaveVR), meta=(BlueprintSpawnableComponent) )
class WAVEVR_API UProjectedPassthroughComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProjectedPassthroughComponent();
	~UProjectedPassthroughComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Set the origin point (i.g. Pawn's position)"))
		void SetReferenceTransform(const FTransform& ref);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Start to show passthrough by the mesh"))
		void EnableProjectedPassthrough();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Stop to show passthrough"))
		void DisableProjectedPassthrough();

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Set a custom mesh with vertices and indices in Unreal's coordination and index order.  If vertices is get from a model, Wave help will turn 180 degree to let it face front (X axis) in tracking space."))
		bool SetVerticesAndIndices(const TArray<FVector>& vertices, const TArray<int32>& indices, bool isFromModel);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "A built-in quad (1mx1m) for projected passthrough.  You can change its scale(x=width, y=height, z=dontcare) any time. The range of scale is [0.01, 10]."))
		void UseBuiltInQuad(const FVector& scale);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "A built-in cube (1mx1mx1m) for projected passthrough.  You can change its scale(x=width, y=height, z=dontcare) any time. The range of scale is [0.01, 10]."))
		void UseBuiltInCuboid(const FVector& scale);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Set alpha"))
		void SetAlpha(float alpha);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Set ProjPT's model transform based on tracking space."))
		void SetTransform(const FTransform& transform);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "Set ProjPT's world transform."))
		void SetWorldTransform(const FTransform& transform);

	UFUNCTION(BlueprintCallable, Category = "WaveVR|ProjPT",
		meta = (ToolTip = "If need force update, i.g. app resume, invoke this API."))
		void SetDirty();

private:
	void ClearBuffers();

private:
	bool started = false;
	float worldToMeters;
	float alpha;
	FTransform refTransform;
	bool update = false;
	float * verticesOutside;
	int verticesOutsideNum;
	uint32_t* indicesOutside;
	int indicesOutsideNum;
	FTransform transformOutside;
	float pose[8];  // 	WVR_Pose_t has 7 float
	bool projPTEnabled;
	bool dirty;
};

extern const uint32_t ProjectedPassthroughMeshCreatorIndex[36];
