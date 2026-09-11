#pragma once

#include "SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
	REFLECT_CLASS(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent();

	void Initialize(EPrimitive ePrimitive);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);

	virtual ~UPrimitiveComponent();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	void Update(TArray<FRenderInfo>* outRenderInfos) override;
	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override;

protected:
	EPrimitive mePrimitive;
};
