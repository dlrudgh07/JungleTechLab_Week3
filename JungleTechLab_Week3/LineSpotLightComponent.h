#pragma once
#include "PrimitiveComponent.h"
#include "StaticMesh.h"

class ULineSpotLightComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(ULineSpotLightComponent, UPrimitiveComponent)
public:
	void Initialize();

	// --- 히트박스(투명 큐브) ---
	void AddRenderInfos(TArray<FRenderInfo>* Out) const override;
	FBoxSphereBounds CalculateBounds(const FMatrix& L2W) const override;   // FIX: 반환형
	FBoxSphereBounds GetLocalBounds() const override;                      // FIX: 반환형
	void GetVertices(std::vector<FVertexSimple>& Out) const override { if (HitMesh) Out = HitMesh->CPUVertices; }
	void GetIndices(std::vector<uint32>& Out)        const override { if (HitMesh) Out = HitMesh->CPUIndices; }
	// --- 스포트라이트 ---
	void Update(TArray<FRenderInfo>* Out, float Dt) override;
	void SerializeClass(json::JSON& OutJson) const override;
	void DeserializeClass(const json::JSON& InJson) override;

	float    OuterAngle = 30.f;   // degree
	float    length = 5.0f;
	int		 Segments = 100;
	FVector4 LineColor = { 1, 1, 1, 0.3f };
	FVector3 Point = {};

private:
	void DrawCone() const;

	UStaticMesh* HitMesh = nullptr;   // ResourceManager "Cube" 빌림. 소유 X
};
