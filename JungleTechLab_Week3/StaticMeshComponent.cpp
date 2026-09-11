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

	Info.BoundsCenter = FVector(0.0f, 0.0f, 0.0f);
	Info.BoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);
	Info.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();
	Info.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };

	outRenderInfos->Add(Info);
}
