#include "MeshComponent.h"

void UMeshComponent::SerializeClass(json::JSON& outJson) const
{
	UPrimitiveComponent::SerializeClass(outJson);
}
void UMeshComponent::DeserializeClass(const json::JSON& inJson)
{
	UPrimitiveComponent::DeserializeClass(inJson);
}

