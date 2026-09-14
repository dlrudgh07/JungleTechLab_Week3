#pragma once

#include "Transform.h"
#include "Object.h"

enum class EBlendMode
{
	Opaque,      // 블렌딩 없음 (BlendState = nullptr)
	Translucent, // 알파 블렌딩 사용
};

struct FRenderInfo
{
	struct FBuffer* VertexBuffer = nullptr;
	const struct FVertexSimple* CollisionVertices = nullptr;
	uint32 CollisionVertexCount = 0;

	class UTexture* BaseTexture = nullptr;

	//BlendMode 임시 설정
	//머티리얼 쪽에 있으면 좋을 거 같긴하다.
	EBlendMode BlendMode = EBlendMode::Opaque;

	FVector BoundsCenter = FVector(0.0f, 0.0f, 0.0f);
	FVector BoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);

	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
};
