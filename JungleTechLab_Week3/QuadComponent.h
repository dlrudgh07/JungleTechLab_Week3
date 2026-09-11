#pragma once

#include "PrimitiveComponent.h"

class UQuadComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UQuadComponent, UPrimitiveComponent)
public:
	UQuadComponent();

	/*
	void Initialize(GraphicsManager* graphicsManager);
	void Initialize(GraphicsManager* graphicsManager, FVector location, FRotator rotation, FVector scale3D);
	*/

	virtual void SerializeClass(json::JSON& outJson) const override {};
	virtual void DeserializeClass(const json::JSON& inJson) override {};

	void Initialize();
	void Initialize(FVector location, FRotator rotation, FVector scale3D);

	virtual ~UQuadComponent();

};
