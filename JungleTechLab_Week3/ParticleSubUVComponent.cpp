#include "ParticleSubUVComponent.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"

void UParticleSubUVComponent::Initialize()
{

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
		Cols = static_cast<decltype(Cols)>(P.at("Cols").ToInt());
	if (P.hasKey("Rows"))
		Rows = static_cast<decltype(Rows)>(P.at("Rows").ToInt());
	if (P.hasKey("Elapsed"))
		Elapsed = NumberFromJson(P.at("Elapsed"));
	if (P.hasKey("PlayRate"))
		PlayRate = NumberFromJson(P.at("PlayRate"));
	if (P.hasKey("bLoop"))
		bLoop = static_cast<decltype(bLoop)>(P.at("bLoop").ToBool());
	if (P.hasKey("scaleX"))
		scaleX = NumberFromJson(P.at("scaleX"));
	if (P.hasKey("scaleY"))
		scaleY = NumberFromJson(P.at("scaleY"));
	if (P.hasKey("offsetX"))
		offsetX = NumberFromJson(P.at("offsetX"));
	if (P.hasKey("offsetY"))
		offsetY = NumberFromJson(P.at("offsetY"));
	if (Cols <= 0 || Rows <= 0)
		throw std::runtime_error("Invalid SubUV grid");
}

//AActor* UParticleSubUVComponent::SpawnSubUVActor(FTransform Transform, const FResourceManager& RM)
//{
//	AActor* NewActor = FObjectFactory::ConstructObject<AActor>();
//	UParticleSubUVComponent* Comp = FObjectFactory::ConstructObject<UParticleSubUVComponent>();
//
//	Comp->SetStaticMesh(RM.GetStaticMesh("Quad"));
//	Comp->SetMaterial(0, RM.GetMaterial("SubUVMaterial"));
//
//	Comp->SetRelativeLocation(Transform.Location);
//	Comp->SetRelativeRotation(Transform.Rotation);
//	Comp->SetRelativeScale3D(Transform.Scale);
//
//	NewActor->AddRootSceneComponent(Comp);
//	AddActor(NewActor);
//	return NewActor;
//}
