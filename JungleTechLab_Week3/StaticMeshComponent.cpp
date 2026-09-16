#include "Actor.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include <DirectXMath.h>


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

	Info.Color = UPrimitiveComponent::GetPrimitiveColor();

	if (CurrentMaterial)
	{
		Info.BaseTexture = CurrentMaterial->BaseTexture;

		// 머티리얼이 존재한다면 해당 tint color를 곱하여 블렌딩
		// 1. 벡터(SIMD 레지스터)로 로드
		DirectX::XMVECTOR ColorVec = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&Info.Color));
		DirectX::XMVECTOR TintVec = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&CurrentMaterial->TintColor));

		// 2. 단 한 번의 명령어로 곱셈 연산
		DirectX::XMVECTOR ResultVec = DirectX::XMVectorMultiply(ColorVec, TintVec);

		// 3. 단 한 번의 명령어로 최대값 1.0f로 Clamp
		DirectX::XMVECTOR MaxVec = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		ResultVec = DirectX::XMVectorMin(ResultVec, MaxVec);

		// 4. 결과를 다시 구조체에 저장
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&Info.Color), ResultVec);
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
#include "SceneSerialization.h"
void UStaticMeshComponent::SerializeClass(json::JSON& outJson) const
{
	UMeshComponent::SerializeClass(outJson);
	outJson["Properties"]["StaticMeshGUID"] = ObjectReference(StaticMesh);
}
void UStaticMeshComponent::DeserializeClass(const json::JSON& inJson)
{
	UMeshComponent::DeserializeClass(inJson);
	const auto& P = inJson.at("Properties");
	if (P.hasKey("StaticMeshGUID"))
		SetStaticMesh(ResolveReference<UStaticMesh>(P.at("StaticMeshGUID")));
	UpdateBounds();
}
