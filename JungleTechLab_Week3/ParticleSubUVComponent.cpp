#include "ParticleSubUVComponent.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"
	
void UParticleSubUVComponent::Initialize()
{
	bIsBillboard = true;
}


void UParticleSubUVComponent::Update(TArray<FRenderInfo>* outRenderInfos, float DeltaTime)
{
	Elapsed += DeltaTime;
	UStaticMeshComponent::Update(outRenderInfos, DeltaTime);
}

void UParticleSubUVComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	const int32 Before = outRenderInfos->Num(); 
	UStaticMeshComponent::AddRenderInfos(outRenderInfos);

	// 부모가 조기 return 했으면(메시 없음) 건드릴 게 없다
	if (outRenderInfos->Num() == Before)
		return;

	int32 Total = Cols * Rows;
	int32 UVIndex = static_cast<int32> (Elapsed * PlayRate);
	if (bLoop) { UVIndex = UVIndex % Total; } // 범위초과시 다시 0으로
	else
	{
		if (UVIndex >= Total) UVIndex = Total - 1;
	}

	const int32 Col = UVIndex % static_cast<int32>(Cols);
	const int32 Row = UVIndex / static_cast<int32>(Cols);

	FRenderInfo& Info = (*outRenderInfos)[outRenderInfos->Num() - 1];
	Info.BlendMode = EBlendMode::Translucent;
	Info.UVTransform.x = 1.0f/Cols;
	Info.UVTransform.y = 1.0f/Rows;
	Info.UVTransform.z = Col / static_cast<float>(Cols);
	Info.UVTransform.w = Row / static_cast<float>(Rows);
}

void UParticleSubUVComponent::SetScaleAndOffset(FVector4 scaleoffset)
{
	scaleX = scaleoffset.x;
	scaleY = scaleoffset.y;
	offsetX = scaleoffset.z;
	offsetY = scaleoffset.w;
}

#include "SceneSerialization.h"
void UParticleSubUVComponent::SerializeClass(json::JSON& outJson) const
{
	UStaticMeshComponent::SerializeClass(outJson);
	outJson["Properties"]["Cols"] = Cols;
	outJson["Properties"]["Rows"] = Rows;
	outJson["Properties"]["Elapsed"] = Elapsed;
	outJson["Properties"]["PlayRate"] = PlayRate;
	outJson["Properties"]["bLoop"] = bLoop;
	outJson["Properties"]["scaleX"] = scaleX;
	outJson["Properties"]["scaleY"] = scaleY;
	outJson["Properties"]["offsetX"] = offsetX;
	outJson["Properties"]["offsetY"] = offsetY;
}
void UParticleSubUVComponent::DeserializeClass(const json::JSON& inJson)
{
	UStaticMeshComponent::DeserializeClass(inJson);
	const auto& P = inJson.at("Properties");
	if (P.hasKey("Cols"))
		Cols = IntegerFromJson(P.at("Cols"), 1, 32767);
	if (P.hasKey("Rows"))
		Rows = IntegerFromJson(P.at("Rows"), 1, 32767);
	if (P.hasKey("Elapsed"))
		Elapsed = NumberFromJson(P.at("Elapsed"));
	if (P.hasKey("PlayRate"))
		PlayRate = NumberFromJson(P.at("PlayRate"));
	if (P.hasKey("bLoop"))
		bLoop = BoolFromJson(P.at("bLoop"));
	if (P.hasKey("scaleX"))
		scaleX = NumberFromJson(P.at("scaleX"));
	if (P.hasKey("scaleY"))
		scaleY = NumberFromJson(P.at("scaleY"));
	if (P.hasKey("offsetX"))
		offsetX = NumberFromJson(P.at("offsetX"));
	if (P.hasKey("offsetY"))
		offsetY = NumberFromJson(P.at("offsetY"));
	if (Cols <= 0 || Rows <= 0 || Elapsed < 0 || PlayRate < 0 || static_cast<double>(Elapsed) * PlayRate > INT32_MAX)
		throw std::runtime_error("Invalid SubUV grid");
}
