#pragma once
#include "MeshComponent.h"
#include "StaticMesh.h"
#include "Material.h"

class UStaticMeshComponent : public UMeshComponent
{
public:
	UStaticMeshComponent() = default;
	virtual ~UStaticMeshComponent() { StaticMesh = nullptr; }

	// factory 에서 필요함
	void Initialize() {};


	void SetStaticMesh(UStaticMesh* NewMesh)
	{
		StaticMesh = NewMesh;
	}
	UStaticMesh* GetStaticMesh() const { return StaticMesh; }


	// 머티리얼 가져오기 (오버라이드 확인 후, 없으면 에셋의 기본 머티리얼 반환)
	virtual UMaterial* GetMaterial(int32 ElementIndex) const override
	{
		// 1. 부모(UMeshComponent)에게 "혹시 이 컴포넌트 전용으로 덮어씌운 재질 있어?" 하고 물어봄
		UMaterial* Material = UMeshComponent::GetMaterial(ElementIndex);

		// 2. 덮어씌운 게 없다면, 내 원본 에셋(StaticMesh)에 구워져 있는 기본 재질을 꺼내옴
		if (Material == nullptr && StaticMesh != nullptr && ElementIndex > -1 && ElementIndex < StaticMesh->StaticMaterials.Num())
		{
			Material = StaticMesh->StaticMaterials[ElementIndex];
		}
		return Material;
	}

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override
	{
		UMeshComponent::Update(OutRenderInfos, DeltaTime);

		AddRenderInfos(OutRenderInfos);
	}

	// 핵심: 렌더러로 데이터 넘기기
	void AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override;

	REFLECT_CLASS(UStaticMeshComponent, UMeshComponent)

private:
	UStaticMesh* StaticMesh = nullptr;
};
