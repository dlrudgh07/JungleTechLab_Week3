#include "PrimitiveComponent.h"
#include "StaticMeshComponent.h"
#include "FEditorViewportClient.h"
#include "JsonUtil.h"

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

	outJson["Properties"]["PrimitiveColor"] = FVector4ToJson(PrimitiveColor);
	outJson["Properties"]["bIsOriginalColor"] = bIsOriginalColor;
	outJson["Properties"]["bIsBillboard"] = bIsBillboard;
}
void UPrimitiveComponent::DeserializeClass(const json::JSON& inJson)
{
	USceneComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (propertiesJson.hasKey("PrimitiveColor") &&
		(propertiesJson.at("PrimitiveColor").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("PrimitiveColor").length() != 4))
	{
		throw std::runtime_error(std::format("{}: PrimitiveColor property requires an array of length 4", GetRuntimeClass()->Name));
	}

	if (propertiesJson.hasKey("PrimitiveColor"))
		PrimitiveColor = FVector4FromJson(propertiesJson.at("PrimitiveColor"));
	if (propertiesJson.hasKey("bIsOriginalColor"))
		bIsOriginalColor = BoolFromJson(propertiesJson.at("bIsOriginalColor"));
	if (propertiesJson.hasKey("bIsBillboard"))
		bIsBillboard = BoolFromJson(propertiesJson.at("bIsBillboard"));
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
	FQuaternion CameraRot = FEditorViewportClient::GetCamera().Transform.Rotation;
	SetRelativeRotation(CameraRot);
}

FVector4 UPrimitiveComponent::GetPrimitiveColor() const
{
	return PrimitiveColor;
}

void UPrimitiveComponent::SetPrimitiveColor(FVector4& NewColor)
{
	PrimitiveColor = NewColor;
}

const bool UPrimitiveComponent::IsOriginalColor() const
{
	return (bIsOriginalColor);
}

void UPrimitiveComponent::SetOriginalColor(bool Value)
{
	bIsOriginalColor = Value;
}
