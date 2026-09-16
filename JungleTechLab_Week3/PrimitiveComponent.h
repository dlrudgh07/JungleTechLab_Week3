#pragma once

#include "SceneComponent.h"
#include "FBoxSphereBounds.h"
#include "Renderer.h"

class UPrimitiveComponent : public USceneComponent
{
	REFLECT_CLASS(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent() {};

	// factory 에서 필요함
	void Initialize() { UpdateBounds(); };
	virtual ~UPrimitiveComponent() {};


	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override;

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	FBoxSphereBounds GetWorldBounds() const
	{
		return Bounds;
	}
	virtual FBoxSphereBounds GetLocalBounds() const
	{
		return FBoxSphereBounds{};
	}


	void UpdateBounds();

	virtual void GetVertices(std::vector<FVertexSimple>& OutVertices)const {};
	virtual void GetIndices(std::vector<uint32>& OutIndices)const {};

	bool GetIsBillboard()const;
	void SetIsBillboard(bool pIsBillboard);

	//bIsBillboard가 true라면 카메라 반대방향으로 SetRelativeRotation을 합니다.
	//상속 컴포넌트 별로 다른 방향 처리를 해야한다면 override해야합니다.
	virtual void SetBillboardTransfom();

protected:
	virtual FBoxSphereBounds CalculateBounds(const FMatrix& LocalToWorld) const
	{
		return Bounds;
	}


	FBoxSphereBounds Bounds{};

	//빌보드 옵션
	//Primitive Component에서 빌보드 처리를 하진 않습니다.
	//각 컴포넌트 별로 빌보드 방향 처리가 다를 수 있기에 
	bool bIsBillboard = false;
};
