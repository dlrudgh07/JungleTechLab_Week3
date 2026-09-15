#include "Actor.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"

void UStaticMeshComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const 
{
	if (StaticMesh == nullptr || StaticMesh->VertexBuffer == nullptr)
		return;

	UMaterial* CurrentMaterial = GetMaterial(0);

	FRenderInfo Info;
	Info.VertexBuffer = StaticMesh->VertexBuffer;

	// CPU 데이터 주소와 개수를 넘김
	Info.CollisionVertices = StaticMesh->CPUVertices.data();
	Info.CollisionVertexCount = static_cast<int32>(StaticMesh->CPUVertices.size());

	if (CurrentMaterial)
	{
		Info.BaseTexture = CurrentMaterial->BaseTexture;
		Info.Color = CurrentMaterial->TintColor;
	}

	Info.LocalBoundsCenter = StaticMesh->LocalBounds.Center;
	Info.LocalBoundsHalfExtent = StaticMesh->LocalBounds.BoxHalfExtent;

	Info.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();
	Info.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };

	outRenderInfos->Add(Info);
}

void UStaticMeshComponent::GetVertices(std::vector<FVertexSimple>& OutVertices) const
{
	OutVertices = GetStaticMesh()->CPUVertices;
}

void UStaticMeshComponent::GetIndices(std::vector<uint32>& OutIndices) const
{
	OutIndices = GetStaticMesh()->CPUIndices;
}


FBoxSphereBounds UStaticMeshComponent::CalculateBounds(const FMatrix& LocalToWorld) const
{
	if (StaticMesh == nullptr)
		return FBoxSphereBounds();

	return StaticMesh->LocalBounds.TransformBy(LocalToWorld);
}

FBoxSphereBounds UStaticMeshComponent::GetLocalBounds() const
{
	if (StaticMesh)
		return StaticMesh->LocalBounds;
	return FBoxSphereBounds{};
}
