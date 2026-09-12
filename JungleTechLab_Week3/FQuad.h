#pragma once

#include "Vector.h"

struct FQuad
{
	FVector LeftUp;
	FVector LeftDown;
	FVector RightUp;
	FVector RightDown;
	float u[2];
	float v[2];

	FQuad() : LeftUp(FVector()), LeftDown(FVector()), RightUp(FVector()), RightDown(FVector()), u{}, v{} {}
	FQuad(FVector _LeftUp, FVector _LeftDown, FVector _RightUp, FVector _RightDown, float _u[2], float _v[2]) :
		LeftUp(_LeftUp), LeftDown(_LeftDown), RightUp(_RightUp), RightDown(_RightDown), u{_u[0], _u[1]}, v(_v[0], _v[1]) {
	}
};
