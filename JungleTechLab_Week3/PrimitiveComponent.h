#pragma once

#include "SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
	REFLECT_CLASS(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent() {};

	// factory 에서 필요함
	void Initialize() {};

	virtual ~UPrimitiveComponent() {};

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override
	{
		USceneComponent::Update(OutRenderInfos, DeltaTime);
	}

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;
};
