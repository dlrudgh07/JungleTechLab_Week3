#include "Material.h"
#include "SceneSerialization.h"
void UMaterial::SerializeClass(json::JSON& outJson) const
{
	UObject::SerializeClass(outJson);
	outJson["Properties"]["BaseTextureGUID"] = ObjectReference(BaseTexture);
	auto A = json::JSON::Make(json::JSON::Class::Array);
	for (float F : {TintColor.x, TintColor.y, TintColor.z, TintColor.w})
		A.append(F);
	outJson["Properties"]["TintColor"] = A;
}
void UMaterial::DeserializeClass(const json::JSON& inJson)
{
	UObject::DeserializeClass(inJson);
	const auto& P = inJson.at("Properties");
	BaseTexture = ResolveReference<UTexture>(P.at("BaseTextureGUID"));
	const auto& A = P.at("TintColor");
	if (A.JSONType() != json::JSON::Class::Array || A.length() != 4)
		throw std::runtime_error("Invalid tint");
	TintColor =
		FVector4(NumberFromJson(A.at(0)), NumberFromJson(A.at(1)), NumberFromJson(A.at(2)), NumberFromJson(A.at(3)));
}
