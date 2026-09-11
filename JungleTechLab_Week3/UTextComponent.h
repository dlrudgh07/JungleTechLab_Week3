#pragma once

#include "Core.h"
#include "PrimitiveComponent.h"

class UTextComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UTextComponent, UPrimitiveComponent)
public:
	UTextComponent();

	/*
	void Initialize(GraphicsManager* graphicsManager);
	void Initialize(GraphicsManager* graphicsManager, FVector location, FRotator rotation, FVector scale3D);
	*/

	virtual void SerializeClass(json::JSON& outJson) const override {};
	virtual void DeserializeClass(const json::JSON& inJson) override {};

	void Initialize();
	void Initialize(FVector location, FRotator rotation, FVector scale3D);

	virtual ~UTextComponent();

	FString Text;		//  todo: 이건 private으로 가야함
};
