#include "Actor.h"

#include <format>

#include "JsonUtil.h"
#include "RenderInfo.h"
#include "SceneComponent.h"

AActor::~AActor()
{
	for (UActorComponent* CurrentComponent : Components)
	{
		delete CurrentComponent;
	}
}

void AActor::Initialize()
{
	UObject::Initialize();

	bPressed = false;
	bStarted = false;
}

void AActor::SerializeClass(json::JSON& outJson) const
{
	UObject::SerializeClass(outJson);
	json::JSON componentsJson = json::JSON::Make(json::JSON::Class::Array);

	for (const UActorComponent* component : Components)
	{
		json::JSON componentJson;
		component->SerializeClass(componentJson);
		componentsJson.append(std::move(componentJson));
	}
	outJson["Properties"]["Components"] = componentsJson;
	outJson["Properties"]["RootComponentGUID"] = RootComponent ? RootComponent->ObjectID.GUID.ToString() : std::to_string(-1);
}

void AActor::DeserializeClass(const json::JSON& inJson)
{
	UObject::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("Components") || propertiesJson.at("Components").JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error(std::format("{}: Components requires an array", GetRuntimeClass()->Name));
	}

	const json::JSON& componentsJson = propertiesJson.at("Components");

	for (const auto& componentJson : componentsJson.ArrayRange())
	{
		if (!componentJson.hasKey("ClassName") || componentJson.at("ClassName").JSONType() != json::JSON::Class::String)
		{
			throw std::runtime_error(std::format("{}: ClassName requires a string", GetRuntimeClass()->Name));
		}
		FString className(componentJson.at("ClassName").ToString());

		const FClassInfo* classInfo = FObjectFactory::GetClassInfoByName(className);
		if (!classInfo)
		{
			throw std::runtime_error(std::format("{}: Unknown class name: {}", GetRuntimeClass()->Name, className));
		}
		UActorComponent* component = static_cast<UActorComponent*>(FObjectFactory::LoadObject(classInfo, componentJson));
		AddComponent(component);
	}

	if (!propertiesJson.hasKey("RootComponentGUID") || propertiesJson.at("RootComponentGUID").JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error(std::format("{}: RootComponentGUID requires a string", GetRuntimeClass()->Name));
	}
	FGuid RootComponentGUID;
	FString GUID{ propertiesJson.at("RootComponentGUID").ToString() };
	RootComponentGUID.Parse(GUID);
	if (RootComponentGUID.IsValid() == false)
	{
		RootComponent = nullptr;
	}
	else
	{
		int32 rootComponentIndex = GetComponentIndex(RootComponentGUID);
		if (rootComponentIndex == -1)
		{
			throw std::runtime_error(std::format("{}: Invalid root component GUID: {}", GetRuntimeClass()->Name, RootComponentGUID.ToString()));
		}
		RootComponent = static_cast<USceneComponent*>(Components[rootComponentIndex]);
	}
}

void AActor::AddComponent(UActorComponent* actorComponent)
{
	assert(actorComponent);
	assert(GetComponentIndex(actorComponent->ObjectID.GUID) == -1);

	Components.Add(actorComponent);
	actorComponent->SetOwner(this);
}

void AActor::AddRootSceneComponent(USceneComponent* sceneComponent)
{
	assert(sceneComponent);
	assert(GetComponentIndex(sceneComponent->ObjectID.GUID) == -1);

	RootComponent = sceneComponent;
	AddComponent(sceneComponent);
}

bool AActor::RemoveComponent(FGuid TargetComponentGuid)
{
	int32 componentIndex = GetComponentIndex(TargetComponentGuid);
	if (componentIndex == -1)
	{
		return false;
	}

	Components.RemoveAtSwap(componentIndex);

	return true;
}

FTransform AActor::GetTransform() const
{
	if (RootComponent)
	{
		return RootComponent->GetTransformMatrix();
	}
	else
	{
		return FTransform();
	}
}


void AActor::Update(TArray<FRenderInfo>* outRenderInfos, float DeltaTime)
{
	for (UActorComponent* component : Components)
	{
		component->Update(outRenderInfos, DeltaTime);
	}
}

void AActor::GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	assert(outRenderInfos);

	for (const UActorComponent* component : Components)
	{
		component->AddRenderInfos(outRenderInfos);
	}
}

bool AActor::GetFirstRenderInfo(FRenderInfo &outRenderInfo) const
{
	TArray<FRenderInfo> renderInfos;
	GetRenderInfos(&renderInfos);

	if (renderInfos.Num() == 0)
	{
		return false;
	}

	outRenderInfo = renderInfos[0];

	return true;
}

void AActor::SetLocation(FVector location)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeLocation(location);
	}
}

void AActor::SetRotation(FRotator rotation)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeRotation(rotation);
	}
}

void AActor::SetScale(FVector scale)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeScale3D(scale);
	}
}

int32 AActor::GetComponentIndex(FGuid TargetComponentGuid) const
{
	for (int32 i = 0; i < Components.Num(); ++i)
	{
		if (Components[i]->ObjectID.GUID == TargetComponentGuid)
		{
			return i;
		}
	}

	return -1;
}
