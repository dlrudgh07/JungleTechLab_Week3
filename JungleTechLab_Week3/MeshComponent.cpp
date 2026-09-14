#include "SceneSerialization.h"
#include "MeshComponent.h"

void UMeshComponent::SerializeClass(json::JSON& outJson) const
{
	UPrimitiveComponent::SerializeClass(outJson);
	auto Values = json::JSON::Make(json::JSON::Class::Array);
	for (const auto* Material : OverrideMaterials)
		Values.append(ObjectReference(Material));
	outJson["Properties"]["OverrideMaterials"] = Values;
}
void UMeshComponent::DeserializeClass(const json::JSON& inJson)
{
	UPrimitiveComponent::DeserializeClass(inJson);
	const auto& P = inJson.at("Properties");
	OverrideMaterials.Reset(0);
	if (P.hasKey("OverrideMaterials"))
	{
		if (P.at("OverrideMaterials").JSONType() != json::JSON::Class::Array)
			throw std::runtime_error("Expected material array");
		for (const auto& Value : P.at("OverrideMaterials").ArrayRange())
			OverrideMaterials.Add(ResolveReference<UMaterial>(Value));
	}
}
