#include "Actor.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"

void UStaticMeshComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	if (StaticMesh == nullptr) return;

	FRenderInfo Info;

	// 1. GPU 및 CPU 데이터 연결
	Info.VertexBuffer = &StaticMesh->VertexBufferGPU;
	Info.IndexBuffer = &StaticMesh->IndexBufferGPU;
	Info.CollisionVertices = StaticMesh->VertexBufferCPU.data();
	Info.CollisionVertexCount = static_cast<uint32>(StaticMesh->VertexBufferCPU.size());
	Info.CollisionIndices = StaticMesh->IndexBufferCPU.data();
	Info.CollisionIndexCount = static_cast<uint32>(StaticMesh->IndexBufferCPU.size());

	// ==========================================
	// 2. 머티리얼 슬롯(Slot) 처리 로직 (우선순위 판별)
	// ==========================================

	// 1순위: 컴포넌트에 오버라이드된 0번 슬롯 머티리얼이 있는지 확인
	UMaterial* TargetMaterial = GetMaterial(0);

	// 2순위: 없다면, 에셋(StaticMesh)이 가진 기본 0번 머티리얼을 사용
	if (TargetMaterial == nullptr && StaticMesh->StaticMaterials.Num() > 0)
	{
		TargetMaterial = StaticMesh->StaticMaterials[0];
	}

	// 3. 최종 결정된 머티리얼을 RenderInfo에 세팅
	if (TargetMaterial != nullptr)
	{
		Info.BaseTexture = TargetMaterial->BaseTexture;
		Info.Color = TargetMaterial->TintColor;
	}
	else
	{
		// 3순위: 머티리얼이 아예 없는 경우의 방어 코드 (렌더러에서 디폴트 화이트로 처리됨)
		Info.BaseTexture = nullptr;
		Info.Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// ==========================================
	// 3. 트랜스폼 및 기타 정보 세팅
	// ==========================================
	Info.BoundsCenter = FVector(0.0f, 0.0f, 0.0f);
	Info.BoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);
	Info.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();
	Info.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };

	outRenderInfos->Add(Info);
}
