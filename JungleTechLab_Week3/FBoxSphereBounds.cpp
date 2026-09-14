#pragma once
#include "FBoxSphereBounds.h"

FBoxSphereBounds FBoxSphereBounds::TransformBy(const FMatrix& LocalToWorld) const
{
	// Center & Extent 방식으로 효율적으로 계산
	FBoxSphereBounds Result;

	Result.Center = LocalToWorld.TransformPosition(this->Center);

	// Extent는 행렬의 회전/스케일 성분의 '절댓값'을 곱해서 계산
	Result.BoxHalfExtent.x = FMath::Abs(LocalToWorld.M[0][0]) * BoxHalfExtent.x + FMath::Abs(LocalToWorld.M[1][0]) * BoxHalfExtent.y + FMath::Abs(LocalToWorld.M[2][0]) * BoxHalfExtent.z;
	Result.BoxHalfExtent.y = FMath::Abs(LocalToWorld.M[0][1]) * BoxHalfExtent.x + FMath::Abs(LocalToWorld.M[1][1]) * BoxHalfExtent.y + FMath::Abs(LocalToWorld.M[2][1]) * BoxHalfExtent.z;
	Result.BoxHalfExtent.z = FMath::Abs(LocalToWorld.M[0][2]) * BoxHalfExtent.x + FMath::Abs(LocalToWorld.M[1][2]) * BoxHalfExtent.y + FMath::Abs(LocalToWorld.M[2][2]) * BoxHalfExtent.z;

	// Radius는 3축 스케일 중 가장 큰 값을 기존 반지름에 곱해줌
	float MaxScale = FMath::Max3(
		LocalToWorld.GetUnitAxis(EAxis::X).Length(),
		LocalToWorld.GetUnitAxis(EAxis::Y).Length(),
		LocalToWorld.GetUnitAxis(EAxis::Z).Length()
	);
	Result.SphereRadius = this->SphereRadius * MaxScale;

	return Result;
}
