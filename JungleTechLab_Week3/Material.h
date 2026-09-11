#pragma once
#include "Object.h"
#include "Vector.h"
#include "Texture.h"

class UMaterial : public UObject
{
public:
	// factory 에서 필요함
	void Initialize() {};


	class UTexture* BaseTexture = nullptr;

	FVector4 TintColor = FVector4(0.0f, 0.0f, 0.0f, 0.0f);

	UMaterial() = default;
	~UMaterial() = default;

	REFLECT_CLASS(UMaterial, UObject)

};
