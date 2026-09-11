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
};
