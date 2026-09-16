#include "PrimitiveComponent.h"
#include "StaticMeshComponent.h"
#include "FEditorViewportClient.h"

void UPrimitiveComponent::Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime)
{
	USceneComponent::Update(OutRenderInfos, DeltaTime);

	if (bIsBillboard)
	{
		SetBillboardTransfom();
	}
}

void UPrimitiveComponent::SerializeClass(json::JSON& outJson) const
{
	USceneComponent::SerializeClass(outJson);
}
void UPrimitiveComponent::DeserializeClass(const json::JSON& inJson)
{
	USceneComponent::DeserializeClass(inJson);
}

void UPrimitiveComponent::UpdateBounds()
{
	Bounds = CalculateBounds(GetTransformMatrix().MakeMatrix());
}

bool UPrimitiveComponent::GetIsBillboard() const
{
	return bIsBillboard;
}

void UPrimitiveComponent::SetIsBillboard(bool pIsBillboard)
\
{
	bIsBillboard = pIsBillboard;
}

void UPrimitiveComponent::SetBillboardTransfom()
{
	//카메라의 반대 방향으로 회전을 설정합니다.
	FQuaternion CameraRot = FEditorViewportClient::GetCamera().Transform.Rotation.Conjugate();
	SetRelativeRotation(CameraRot);
}
