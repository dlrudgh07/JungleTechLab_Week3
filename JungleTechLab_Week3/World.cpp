#include "World.h"

#include <format>

#include "RenderInfo.h"
#include "JsonUtil.h"
#include "Console.h"

#include "ResourceManager.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"

UWorld::~UWorld()
{
	for (AActor* CurrentActor : Actors)
	{
		delete CurrentActor;
	}
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
	UObject::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("Actors") || propertiesJson.at("Actors").JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error(std::format("{}: Actors requires an array", GetRuntimeClass()->Name));
	}

	const json::JSON& actorsJson = propertiesJson.at("Actors");

	for (const auto& actorJson : actorsJson.ArrayRange())
	{
		if (!actorJson.hasKey("ClassName") || actorJson.at("ClassName").JSONType() != json::JSON::Class::String)
		{
			throw std::runtime_error(std::format("{}: ClassName requires a string", GetRuntimeClass()->Name));
		}
		FString className(actorJson.at("ClassName").ToString());

		const FClassInfo* classInfo = FObjectFactory::GetClassInfoByName(className);
		if (!classInfo)
		{
			throw std::runtime_error(std::format("{}: Unknown class name: {}", GetRuntimeClass()->Name, className));
		}
		AActor* actor = static_cast<AActor*>(FObjectFactory::LoadObject(classInfo, actorJson));
		AddActor(actor);
	}
}

void UWorld::AddActor(AActor* actor)
{
	assert(actor != nullptr);
	// todo : 이거 왜 안되는지 확인해야함
	//assert(GetActorIndex(actor->ObjectID.GUID) == -1);

	Actors.Add(actor);
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

const TArray<FRenderInfo> UWorld::GetRenderInfos()
{
	return RenderInfos;
}

void UWorld::Update(float DeltaTime)
{
	RenderInfos.Reset(DEFAULT_RESERVE_MEM);

	for (AActor* CurrentActor : Actors)
	{
		if (!CurrentActor->IsPendingKill())
		{
			CurrentActor->Update(&RenderInfos, DeltaTime);
		}
	}
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
	Actors.Add(NewActor);

	return NewActor;
}
