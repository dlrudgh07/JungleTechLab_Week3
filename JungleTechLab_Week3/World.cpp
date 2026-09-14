#include "SceneSerialization.h"
#include "World.h"

#include <format>

#include "RenderInfo.h"
#include "JsonUtil.h"
#include "Console.h"

#include "ResourceManager.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "UTextComponent.h"

UWorld::~UWorld()
{
	for (AActor* CurrentActor : Actors)
	{
		delete CurrentActor;
	}
	Actors.Reset(0);
}

void UWorld::SerializeClass(json::JSON& outJson) const
{
	UObject::SerializeClass(outJson);
	json::JSON actorsJson = json::JSON::Make(json::JSON::Class::Array);

	for (const AActor* actor : Actors)
	{
		json::JSON actorJson;
		actor->SerializeClass(actorJson);
		actorsJson.append(std::move(actorJson));
	}
	outJson["Properties"]["Actors"] = actorsJson;
}

void UWorld::DeserializeClass(const json::JSON& inJson)
{
	std::unique_ptr<FSceneLoadScope> OwnScope;
	if (!FSceneLoadScope::Current)
	{
		OwnScope = std::make_unique<FSceneLoadScope>();
		OwnScope->Register(this, inJson);
	}
	if (Actors.Num() != 0)
		throw std::runtime_error("Deserialize world requires an empty world");
	UObject::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("Actors") || propertiesJson.at("Actors").JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error(std::format("{}: Actors requires an array", GetRuntimeClass()->Name));
	}

	const json::JSON& actorsJson = propertiesJson.at("Actors");

	// Register every actor before preloading any components or resolving references.
	for (const auto& Data : actorsJson.ArrayRange())
	{
		auto Actor = PreloadObject<AActor>(Data);
		AddActor(Actor.get());
		Actor.release();
	}
	int Index = 0;
	for (const auto& Data : actorsJson.ArrayRange())
		Actors[Index++]->PreloadComponents(Data);
	Index = 0;
	for (const auto& Data : actorsJson.ArrayRange())
		Actors[Index++]->DeserializeClass(Data);
}
void UWorld::AddActor(AActor* Actor)
{
	assert(Actor != nullptr);
	// todo : 이거 왜 안되는지 확인해야함
	//assert(GetActorIndex(actor->ObjectID.GUID) == -1);

	Actors.Add(Actor);
	Actor->Initialize(this);
}

bool UWorld::RemoveActor(FGuid TargetComponentGuid)
{
	int32 index = GetActorIndex(TargetComponentGuid);
	if (index != -1)
	{
		Actors[index]->Destroy(); // 배열에서 빼지 말고 플래그만 세움
		return true;
	}
	return false;
}

const TArray<FRenderInfo>& UWorld::GetRenderInfos() const
{
	return RenderInfos;
}

void UWorld::Update(float DeltaTime)
{
	RenderInfos.Reset(DEFAULT_RESERVE_MEM);

	for (AActor* CurrentActor : Actors)
	{
		CurrentActor->Update(&RenderInfos, DeltaTime);
	}
}

void UWorld::RequestDestroyActor(AActor* Actor)
{
	PendingKillList.Add(Actor);
}

void UWorld::ProcessPendingKills()
{
	if (PendingKillList.Num() == 0) return; 

	for (AActor* DeadActor : PendingKillList)
	{
		int32 index = GetActorIndex(DeadActor->ObjectID.GUID);
		if (index != -1)
		{
			Actors.RemoveAtSwap(index);
		}

		delete DeadActor;
	}

	PendingKillList.Reset(0);
}


int32 UWorld::GetActorIndex(FGuid TargetGuid) const
{
	for (int32 i = 0; i < Actors.Num(); ++i)
	{
		if (Actors[i]->ObjectID.GUID == TargetGuid)
		{
			return i;
		}
	}

	return -1;
}

AActor* UWorld::SpawnStaticMeshActor(const std::string& AssetName, FTransform Transform, const FResourceManager &ResourceManager)
{
	// 1. 리소스 매니저에서 에셋(UStaticMesh) 검색
	UStaticMesh* LoadedMesh = ResourceManager.GetStaticMesh(AssetName);
	if (LoadedMesh == nullptr)
	{
		// 에셋을 못 찾았을 경우 에러 처리
		return nullptr;
	}


	// 2. 팩토리를 통해 빈 액터와 컴포넌트 생성 후 에셋 할당
	AActor* NewActor = FObjectFactory::ConstructObject<AActor>();
	UStaticMeshComponent* MeshComponent = FObjectFactory::ConstructObject<UStaticMeshComponent>();

	MeshComponent->SetStaticMesh(LoadedMesh); // 컴포넌트에 에셋 장착

	MeshComponent->SetRelativeLocation(Transform.Location);
	MeshComponent->SetRelativeRotation(Transform.Rotation);
	MeshComponent->SetRelativeScale3D(Transform.Scale);

	NewActor->AddRootSceneComponent(MeshComponent); // 액터의 루트로 등록

	// 3. 씬의 액터 목록(Level 배열)에 추가
	AddActor(NewActor);

	return NewActor;
}

AActor* UWorld::SpawnTextMeshActor(FTransform Transform, const FResourceManager& ResourceManager)
{
	//리소스 매니저에서 굴림체 폰트 가져오기
	FFontAsset* LoadFont = ResourceManager.GetFontAsset("Gulim");

	// 2. 팩토리를 통해 빈 액터와 컴포넌트 생성 후 에셋 할당
	AActor* NewActor = FObjectFactory::ConstructObject<AActor>();
	UTextComponent* TextComponent = FObjectFactory::ConstructObject<UTextComponent>();

	TextComponent->SetRelativeLocation(Transform.Location);
	TextComponent->SetRelativeRotation(Transform.Rotation);
	TextComponent->SetRelativeScale3D(Transform.Scale);
	TextComponent->SetFontAsset(LoadFont);
	TextComponent->SetText(L"여기에 입력하세요.");

	NewActor->AddRootSceneComponent(TextComponent); // 액터의 루트로 등록

	// 3. 씬의 액터 목록(Level 배열)에 추가
	AddActor(NewActor);
	return NewActor;
}


#include "ParticleSubUVComponent.h"
AActor* UWorld::SpawnSubUVActor(FTransform Transform, const FResourceManager& RM)
{
	AActor* NewActor = FObjectFactory::ConstructObject<AActor>();
	UParticleSubUVComponent* Comp = FObjectFactory::ConstructObject<UParticleSubUVComponent>();

	Comp->SetStaticMesh(RM.GetStaticMesh("Quad"));
	Comp->SetMaterial(0, RM.GetMaterial("SubUVMaterial"));

	Comp->SetRelativeLocation(Transform.Location);
	Comp->SetRelativeRotation(Transform.Rotation);
	Comp->SetRelativeScale3D(Transform.Scale);

	NewActor->AddRootSceneComponent(Comp);
	AddActor(NewActor);
	return NewActor;
}
