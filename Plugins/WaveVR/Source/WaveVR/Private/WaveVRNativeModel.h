// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"

// Wave
#include "WaveVRBlueprintFunctionLibrary.h"
#include "Platforms/WaveVRAPIWrapper.h"

#include "WaveVRNativeModel.generated.h"

USTRUCT()
struct FNBatteryLevelInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int level;

	UPROPERTY()
	float min;

	UPROPERTY()
	float max;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	UTexture2D* texture;
};

struct NTouchpadPlaneInfo {
	FMatrix matrix;  // Button space to controller space transform
	FVector center;
	float floatingDistance;
	float radius;
	bool valid;
};

struct NButtonMeshNameMapping {
	EWVR_InputId btn;
	FName meshName;
	FString lowerMeshName;
};

struct NButtonEffectInfo {
	EWVR_InputId btn;
	FName meshName;
	FString lowerMeshName;
	UProceduralMeshComponent *MeshComp;
	UMaterialInterface *originMat;
	UMaterialInstanceDynamic *meshMatInst;
};

struct BinaryButtonObject
{
	EWVR_InputId btn;
	FName meshName;
	UProceduralMeshComponent *MeshComp;
	UProceduralMeshComponent *MeshCompOutline;
	FVector originPosition;
	FRotator originRotation;
	FVector originScale;
	FVector pressPosition;
	FRotator pressRotation;
	FVector pressScale;
};

struct Travel1DObject
{
	EWVR_InputId btn;
	FName meshName;
	UProceduralMeshComponent *MeshComp;
	FVector originPosition;
	FRotator originRotation;
	FVector originScale;
	FVector pressPosition;
	FRotator pressRotation;
	FVector pressScale;

	FVector scalePosition;
	FRotator scaleRotation;
	FVector scaleScale;
};

struct ThumbstickObject
{
	EWVR_InputId btn;
	FName meshName;
	UProceduralMeshComponent *MeshComp;
	UProceduralMeshComponent *MeshCompOutline;
	FVector centerPosition;
	FRotator centerRotation;
	FVector centerScale;
	FVector upPosition;
	FRotator upRotation;
	FVector upScale;
	FVector rightPosition;
	FRotator rightRotation;
	FVector rightScale;
	float radius;

	FVector ptW;
	FVector ptU;
	FVector ptV;
	FRotator rtW;
	FRotator rtU;
	FRotator rtV;

	FRotator maxRotation;
};

enum AnimationType
{
	AnimBinary,
	AnimTravel1D,
	AnimThumbstick
};

struct FMeshComponent {
	UProceduralMeshComponent* mesh;
	UProceduralMeshComponent* meshOutline;
	FString name;
	FString lowerName;
	TArray<FVector> vertices;
	TArray<int32> indices;
	TArray<FVector> normals;
	TArray<FVector2D> uvs;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	FMatrix transform;
	UMaterialInstanceDynamic* material;
	UMaterialInstanceDynamic* materialOutline;
};

struct FButtonFeature {
	EWVR_InputId inputId;
	FString name;
	FString lowerName;

	// Button features
	bool btnTouch;
	bool btnPress;

	// Animation features
	bool animBinary;
	bool animTravel1D;
	bool animThumbstick;

	FButtonFeature() : 
		inputId(EWVR_InputId::NoUse)
	{
	}

	FButtonFeature(EWVR_InputId _inputId, const FString& _name, const FString& _lowerName) :
		inputId(_inputId), name(_name), lowerName(_lowerName), btnTouch(false), btnPress(false), animBinary(false), animTravel1D(false), animThumbstick(false) {
	}
};

UCLASS(ClassGroup = (WaveVR))
class AWaveVRNativeModel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaveVRNativeModel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// This is called when actor is spawned (at runtime or when you drop it into the world in editor)
	virtual void PostActorCreated() override;
	// This is called when actor is already in level and map is opened
	virtual void PostLoad() override;

	bool alwaysShow;
	EWVR_DeviceType deviceType;
	int deviceTypeInt;
	WVR_DeviceType deviceTypeWVR;
	bool hideWhenIdle = true;

	USceneComponent* SceneComponent;
	USceneComponent* PoseModeScene;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnControllerPoseModeChangedHandling(uint8 Device, uint8 Mode, FTransform Transform);

private:
	// Get resources from native and create mesh component
	void AssembleController();
	void UpdateMaterial();

	// Parse mesh related functions
	void ParseMesh();
	void ClearProcessedMeshData();
	void ProcessNode();
	void ProcessMesh(FMeshComponent& comp, const WVR_VertexBuffer_t& vertices, const WVR_VertexBuffer_t& normals, const WVR_VertexBuffer_t& texCoords, const WVR_IndexBuffer_t& indices);

	void CompleteMeshComponent();

	// Button feature hardcode definifitions.
	static void AddButtonMapping(TArray<FButtonFeature>& table, EWVR_InputId btn, FString meshName, bool isTouch);
	static void AddButtonAnimation(TArray<FButtonFeature>& table, EWVR_InputId btn, FString meshName, AnimationType type);

	// Build caches by input mapping pairs
	void CheckIfButtonAnimation();
	bool isButtonAnimation = false;
	void CollectButtonEffectMesh();
	void CollectTouchEffectMesh();
	void SetTouchpadPlaneInfo();

	void BuildEffectCaches();

	// Do Button effect
	void PollingButtonPressState();
	void PollingTouchState();

	// Do Button animation
	void AnimateButtonPress();
	void AnimateTravel1D();
	void AnimateThumbstick();

	// Battery
	void UpdateBattery();
	void SetBatteryInfo();

	// Pose mode
	void DelayedSetRelativeTransform();

	// Interaction mode
	bool IsModelShow();

private:
	WVR_CtrlerModel_t* ctrl = nullptr;
	bool isOneBone = true;

	TArray<FMeshComponent> components;
	int32 componentCount;

	UPROPERTY()
	TArray<FNBatteryLevelInfo> batteryLevelInfo;

	NTouchpadPlaneInfo touchpadPlaneInfo;

	//UMaterialInterface* MaterialInterface;
	UMaterialInterface* controllerInterface;
	UMaterialInterface* controllerOutlineInterface;

	UMaterialInterface* batteryInterface;
	UMaterialInstanceDynamic* batteryDynamic;
	UProceduralMeshComponent* batteryMesh;

	UMaterialInstanceDynamic* touchEffectDynamic;

	UTexture2D* blueEffectTex;
	UTexture2D* bodyTex;

	TArray<NButtonEffectInfo> pressEffectMap;  // componentCount
	TArray<bool> pressBtnState;  // componentCount

	TArray<NButtonEffectInfo> touchEffectMap;  // componentCount
	TArray<bool> touchState;  // componentCount

	// The key is lowerName, and the value is index of components
	TMap<FString, int> componentTable;

	// These are hardcode data, and should be static.
	static const TArray<FButtonFeature>& GetButtonFeatureTable();
	static const TArray<FButtonFeature>& GetButtonFeatureTableAnime();
	uint64 inputMappingPairUpdatedFrame;

	// Update each frame
	TArray<BinaryButtonObject> binaryObjectMap;
	TArray<bool> binaryState;
	TArray<Travel1DObject> travel1DObjectMap;
	TArray<ThumbstickObject> thumbstickObjectMap;

	bool hasBatteryMesh;
	bool showBattery;
	int intervalCount;
	int printCount;
	bool isTouchPress;
	bool printable;
	int preBatteryLevel;
	uint8 PoseMode;

	uint64_t SetRelativeTransformFrame;
	uint64_t Frame_DeviceTypeBeenSet;
	FTransform PoseModeTransform;
	FTransform EmitterTransform;
	bool assembleControllerDone;
	bool componentIsCompleted;

	FVector devicePosition = FVector::ZeroVector;
	FRotator deviceRotation = FRotator::ZeroRotator;

	friend class UWaveVRControllerModel;
	//float testBatteryLevel;
};
