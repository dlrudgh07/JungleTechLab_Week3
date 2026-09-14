#pragma once

#include "Transform.h"
#include "Object.h"

struct FRenderInfo
{
	struct FBuffer* VertexBuffer = nullptr;
	
	const struct FVertexSimple* CollisionVertices = nullptr;
	uint32 CollisionVertexCount = 0;

	class UTexture* BaseTexture = nullptr;

	FVector LocalBoundsCenter = FVector(0.0f, 0.0f, 0.0f);
	FVector LocalBoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);

	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
};
