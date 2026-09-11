#pragma once

#include "Transform.h"
#include "Object.h"

struct FRenderInfo
{
	EPrimitive ePrimitive;
	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;

	class FTexture* Texture = nullptr; // texture mapping
	FVector4 UVScaleOffset = FVector4(1.0f, 1.0f, 0.0f, 0.0f); // sub uv
};
