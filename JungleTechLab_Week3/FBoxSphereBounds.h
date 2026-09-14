#pragma once
#include "Vector.h"
#include "Matrix.h"

struct FBoxSphereBounds
{
	FVector Center;
	FVector BoxHalfExtent;
	float SphereRadius;

	FBoxSphereBounds()
		: Center(0, 0, 0), BoxHalfExtent(0, 0, 0), SphereRadius(0.f) {}
	FBoxSphereBounds(const FBoxSphereBounds& InputBounds)
		: Center(InputBounds.Center), BoxHalfExtent(InputBounds.BoxHalfExtent), SphereRadius(InputBounds.SphereRadius) {}

	FBoxSphereBounds(FVector Min, FVector Max)
	{
		Center = (Max + Min) * 0.5f;
		BoxHalfExtent = (Max - Min) * 0.5f;
		SphereRadius = BoxHalfExtent.Length();
	}

	void Initialize(FVector Min, FVector Max)
	{
		Center = (Max + Min) * 0.5f;
		BoxHalfExtent = (Max - Min) * 0.5f;
		SphereRadius = BoxHalfExtent.Length();
	}


	// 새로운 World Matrix로 변환하는 함수
	FBoxSphereBounds TransformBy(const FMatrix& LocalToWorld) const;
};
