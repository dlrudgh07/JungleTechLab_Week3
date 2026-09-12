#pragma once

#include "Transform.h"
#include "Object.h"

struct FRenderInfo {
	const struct FBuffer* VertexBuffer = nullptr;
	const struct FBuffer* IndexBuffer = nullptr;

	const struct FVertexSimple* CollisionVertices = nullptr;
	uint32 CollisionVertexCount = 0;
	const uint32* CollisionIndices = nullptr; 
	uint32 CollisionIndexCount = 0;

	class UTexture* BaseTexture = nullptr;

	FVector BoundsCenter = FVector(0.0f, 0.0f, 0.0f);
	FVector BoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);

	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
};
