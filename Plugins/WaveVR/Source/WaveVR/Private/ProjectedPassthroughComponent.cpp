// Fill out your copyright notice in the Description page of Project Settings.
#include "ProjectedPassthroughComponent.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "Platforms/WaveVRAPIWrapper.h"
#include "wvr_system.h"

DEFINE_LOG_CATEGORY(ProjPT);

// Sets default values for this component's properties
UProjectedPassthroughComponent::UProjectedPassthroughComponent()
	: worldToMeters(100)
	, alpha(1.0f)
	, verticesOutside(nullptr)
	, verticesOutsideNum(0)
	, indicesOutside(nullptr)
	, indicesOutsideNum(0)
	, projPTEnabled(false)
	, dirty(true)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	WVR_Pose_t* wpose = (WVR_Pose_t*)pose;
	wpose->position = WVR_Vector3f_t{ 0, 0, -0 };
	wpose->rotation = WVR_Quatf_t{ -1, 0, 0, -0 };
}


// Called when the game starts
void UProjectedPassthroughComponent::BeginPlay()
{
	Super::BeginPlay();
	verticesOutside = nullptr;
	indicesOutside = nullptr;
	worldToMeters = GWorld->GetWorldSettings()->WorldToMeters;
}


// Called every frame
void UProjectedPassthroughComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//LOGD(ProjPT, "TickComponent(): d=%d, en=%d a=%f vs=%d is=%d", dirty, projPTEnabled, alpha, verticesOutsideNum, indicesOutsideNum);

	if (!dirty)
		return;
	dirty = false;

	WVR()->ShowProjectedPassthrough(projPTEnabled);
	if (!projPTEnabled)
		return;
	WVR()->SetProjectedPassthroughAlpha(alpha);
	WVR()->SetProjectedPassthroughPose((WVR_Pose_t*)pose);
	if (verticesOutsideNum == 0 || indicesOutsideNum == 0 || verticesOutside == nullptr || indicesOutside == nullptr)
		return;
	WVR()->SetProjectedPassthroughMesh(verticesOutside, verticesOutsideNum, indicesOutside, indicesOutsideNum);
}


UProjectedPassthroughComponent::~UProjectedPassthroughComponent()
{
	ClearBuffers();
}

//void UProjectedPassthroughComponent::BeginDestroy() {
//	if (projPTEnabled)
//		WVR()->ShowProjectedPassthrough(false);
//	ClearBuffers();
//}

void UProjectedPassthroughComponent::SetReferenceTransform(const FTransform& ref) {
	auto l = ref.GetLocation();
	//LOGD(ProjPT, "SetReferenceTransform() l=(%.3f, %.3f, %.3f)", l.X, l.Y, l.Z);
	refTransform = ref;
}

/*
	  Outter face                 Inner face
	   7---6     +z       or        3---2     -z
	   |\  |\                       |\  |\
	   4-\-5 \                      0-\-1 \
	    \ 3---2                      \ 7---6
	     \|   |                       \|   |
	      0---1  -z                    4---5  +z
	{ 0, 1, 2, 3 },     // back, -z
	{ 5, 4, 7, 6 },     // front, +z
	{ 4, 0, 3, 7 },     // left, -x
	{ 1, 5, 6, 2 },     // right, +x,
	{ 4, 5, 1, 0 },     // bottom, -y
	{ 3, 2, 6, 7 }      // top, +y
*/
const uint32_t ProjectedPassthroughMeshCreatorIndex[2 * 6 * 3] = {
	0, 1, 2, 0, 2, 3,     // back, -z
	5, 4, 7, 5, 7, 6,     // front, +z
	4, 0, 3, 4, 3, 7,     // left, -x
	1, 5, 6, 1, 6, 2,     // right, +x,
	4, 5, 1, 4, 1, 0,     // bottom, -y
	3, 2, 6, 3, 6, 7      // top, +y
};

// Diagram in OrientedBoxTypes.h for index / corner mapping.
// corners [ (-x,-y), (x,-y), (x,y), (-x,y) ], -z, then +z
//
//   7---6     +z       or        3---2     -z
//   |\  |\                       |\  |\
//   4-\-5 \                      0-\-1 \
//    \ 3---2                      \ 7---6   
//     \|   |                       \|   |
//      0---1  -z                    4---5  +z
//
// @todo does this ordering make sense for UE? we are in LHS instead of RHS here
// if this is modified, likely need to update IndexUtil::BoxFaces and BoxFaceNormals


// Basic mesh vertices { 0-1-1, 01-1, 011, 0-11 } indices { 0, 1, 2, 0, 2, ,3 }
bool UProjectedPassthroughComponent::SetVerticesAndIndices(const TArray<FVector>& vertices, const TArray<int32>& indices, bool isFromModel)
{
	//LOGD(ProjPT, "SetVerticesAndIndices() vs=%d is=%d", vertices.Num(), indices.Num());

	int numVertices = vertices.Num();
	int numIndices = indices.Num();
	if ((numIndices % 3) != 0)
		return false;

	dirty = true;

	ClearBuffers();

	verticesOutsideNum = numVertices * 3;
	verticesOutside = new float[verticesOutsideNum];
	for (int i = 0, ii = 0; i < numVertices; i++, ii += 3) {
		const auto& v = vertices[i] / worldToMeters;

		if (isFromModel) {
			// In ModelSpace, UE [xyz] = GL [-yzx]
			verticesOutside[ii]     = -v.Y;
			verticesOutside[ii + 1] =  v.Z;
			verticesOutside[ii + 2] =  v.X;
		} else {
			// In TrackingSpace, UE [xyz] = GL [yz-x] 
			verticesOutside[ii]     =  v.Y;
			verticesOutside[ii + 1] =  v.Z;
			verticesOutside[ii + 2] = -v.X;
		}
	}

	indicesOutsideNum = numIndices;
	indicesOutside = new uint32_t[numIndices];
	for (int i = 0, ii = 0; i < numIndices / 3; i++, ii += 3) {
		indicesOutside[ii] = indices[ii];
		indicesOutside[ii + 1] = indices[ii + 1];
		indicesOutside[ii + 2] = indices[ii + 2];
		// left hand to right hand?
		//indicesOutside[ii + 1] = indices[ii + 2];
		//indicesOutside[ii + 2] = indices[ii + 1];
	}
	return true;
}

/*
//    3---2
//    |   |
//    0---1
*/
void UProjectedPassthroughComponent::UseBuiltInQuad(const FVector& scale) {
	//LOGD(ProjPT, "UseBuiltInQuad() s=(%.3f, %.3f, %.3f)", scale.X, scale.Y, scale.Z);

	dirty = true;
	FVector s = ClampVector(scale, FVector(0.01f), FVector(10));
	s /= 2;
	float vertices[] = {
		-s.X, -s.Y, 0,
		 s.X, -s.Y, 0,
		 s.X,  s.Y, 0,
		-s.X,  s.Y, 0,
	};

	uint32_t indices[] = {
		0, 1, 2, 0, 2, 3,
	};

	ClearBuffers();

	verticesOutsideNum = sizeof(vertices) / sizeof(*vertices);
	verticesOutside = new float[verticesOutsideNum];

	indicesOutsideNum = sizeof(indices) / sizeof(*indices);
	indicesOutside = new uint32_t[indicesOutsideNum];

	for (int i = 0; i < verticesOutsideNum; i++)
		verticesOutside[i] = vertices[i];

	for (int i = 0; i < indicesOutsideNum; i++)
		indicesOutside[i] = indices[i];
}

/*
//   7---6     +z  
//   |\  |\        
//   4-\-5 \       
//    \ 3---2      
//     \|   |      
//      0---1  -z  
*/
void UProjectedPassthroughComponent::UseBuiltInCuboid(const FVector& scale) {
	//LOGD(ProjPT, "UseBuiltInCuboid() s=(%.3f, %.3f, %.3f)", scale.X, scale.Y, scale.Z);

	dirty = true;
	FVector s = ClampVector(scale, FVector(0.01f), FVector(10));
	s /= 2;
	float vertices[] = {
		-s.X, -s.Y,  s.Z,
		 s.X, -s.Y,  s.Z,
		 s.X,  s.Y,  s.Z,
		-s.X,  s.Y,  s.Z,
		-s.X, -s.Y, -s.Z,
		 s.X, -s.Y, -s.Z,
		 s.X,  s.Y, -s.Z,
		-s.X,  s.Y, -s.Z,
	};

	ClearBuffers();

	verticesOutsideNum = sizeof(vertices) / sizeof(*vertices);
	verticesOutside = new float[verticesOutsideNum];

	indicesOutsideNum = sizeof(ProjectedPassthroughMeshCreatorIndex) / sizeof(*ProjectedPassthroughMeshCreatorIndex);
	indicesOutside = new uint32_t[indicesOutsideNum];
	for (int i = 0; i < verticesOutsideNum; i++)
		verticesOutside[i] = vertices[i];

	for (int i = 0; i < indicesOutsideNum; i++)
		indicesOutside[i] = ProjectedPassthroughMeshCreatorIndex[i];
}

void UProjectedPassthroughComponent::SetAlpha(float a) {
	LOGD(ProjPT, "SetAlpha() a=%f", a);

	dirty = true;
	alpha = a;
}

void UProjectedPassthroughComponent::SetTransform(const FTransform& transform) {
	auto l = transform.GetLocation();
	//LOGD(ProjPT, "SetTransform() l=(%.3f, %.3f, %.3f)", l.X, l.Y, l.Z);

	dirty = true;
	transformOutside = transform;
	auto p = transformOutside.GetLocation() / worldToMeters;
	auto q = transformOutside.GetRotation();
	WVR_Pose_t* wpose = (WVR_Pose_t*)pose;
	wpose->position = WVR_Vector3f_t{ p.Y, p.Z, -p.X };
	wpose->rotation = WVR_Quatf_t{ -q.W, q.Y, q.Z, -q.X };
}

void UProjectedPassthroughComponent::SetWorldTransform(const FTransform& transform) {
	auto l = transform.GetLocation();
	//LOGD(ProjPT, "SetWorldTransform() l=(%.3f, %.3f, %.3f)", l.X, l.Y, l.Z);
	dirty = true;
	transformOutside = transform * refTransform.Inverse();
	auto p = transformOutside.GetLocation() / worldToMeters;
	auto q = transformOutside.GetRotation();
	WVR_Pose_t* wpose = (WVR_Pose_t*)pose;
	wpose->position = WVR_Vector3f_t{ p.Y, p.Z, -p.X };
	wpose->rotation = WVR_Quatf_t{ -q.W, q.Y, q.Z, -q.X };
}

void UProjectedPassthroughComponent::EnableProjectedPassthrough() {
	LOGD(ProjPT, "EnableProjectedPassthrough()");
	projPTEnabled = true;
	dirty = true;
	WVR()->ShowProjectedPassthrough(projPTEnabled);
}

void UProjectedPassthroughComponent::DisableProjectedPassthrough() {
	LOGD(ProjPT, "DisableProjectedPassthrough()");
	projPTEnabled = false;
	dirty = true;
	WVR()->ShowProjectedPassthrough(projPTEnabled);
}

void UProjectedPassthroughComponent::SetDirty() {
	dirty = true;
}

void UProjectedPassthroughComponent::ClearBuffers() {
	if (verticesOutside != nullptr)
		delete[] verticesOutside;
	verticesOutside = nullptr;
	verticesOutsideNum = 0;

	if (indicesOutside != nullptr)
		delete[] indicesOutside;
	indicesOutside = nullptr;
	indicesOutsideNum = 0;
}
