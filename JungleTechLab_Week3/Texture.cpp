#include "Texture.h"
#include "SceneSerialization.h"
void UTexture::SerializeClass(json::JSON& outJson) const
{
	UObject::SerializeClass(outJson);
	auto& P = outJson["Properties"];
	P["SourcePath"] = SourcePath;
	P["Width"] = Width;
	P["Height"] = Height;
}
void UTexture::DeserializeClass(const json::JSON& inJson)
{
	UObject::DeserializeClass(inJson);
	const auto& P = inJson.at("Properties");
	SourcePath = FString(P.at("SourcePath").ToString());
	Width = static_cast<int32>(P.at("Width").ToInt());
	Height = static_cast<int32>(P.at("Height").ToInt());
}
