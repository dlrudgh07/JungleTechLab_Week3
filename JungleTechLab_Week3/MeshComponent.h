#pragma once
#include "Object.h"
#include "PrimitiveComponent.h"
#include "Material.h"

class UMeshComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UMeshComponent, UPrimitiveComponent)
public:
	UMeshComponent() {};
	virtual ~UMeshComponent() {};

	// factory 에서 필요함
	void Initialize() {};

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override
	{
		UPrimitiveComponent::Update(OutRenderInfos, DeltaTime);
	}
	
	void SetMaterial(int32 ElementIndex, class UMaterial* Material)
	{
		if (OverrideMaterials.Num() <= ElementIndex)
		{
			OverrideMaterials.SetNum(ElementIndex + 1);
		}
		OverrideMaterials[ElementIndex] = Material;
	}

	virtual class UMaterial* GetMaterial(int32 ElementIndex) const
	{
		if (ElementIndex > -1 && OverrideMaterials.Num() > ElementIndex && OverrideMaterials[ElementIndex] != nullptr)
		{
			return OverrideMaterials[ElementIndex];
		}
		return nullptr;
	}

protected:
	// 컴포넌트별로 다르게 설정한 머티리얼 목록 (에셋 원본을 훼손하지 않기 위함)
	TArray<class UMaterial*> OverrideMaterials;
};
