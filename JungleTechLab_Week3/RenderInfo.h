#pragma once

#include "Transform.h"
#include "Object.h"
#include <d3d11.h>

struct FCharDataInfo
{
	//아틀라스 내 x,y 좌표(시작 지점)
	int32 CharX, CharY;

	//글자의 크기
	int32 CharWidth, CharHeight;

	//아틀라스 Width, Height
	int32 AtlasWidth = 0;

	int32 AtlasHeight = 0;
};

struct FRenderInfo
{
	EPrimitive ePrimitive;
	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
	FCharDataInfo CharData;
	ID3D11ShaderResourceView* SRV;
};


