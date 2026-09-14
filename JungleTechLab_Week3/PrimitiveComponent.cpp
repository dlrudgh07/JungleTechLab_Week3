#include "PrimitiveComponent.h"
#include "StaticMeshComponent.h"

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
