#include "SceneSerialization.h"
#include "Actor.h"

#include <format>

#include "World.h"
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

void AActor::Destroy()
{
	// 이미 사형 선고를 받았다면 중복 등록 방지
	if (IsPendingKill()) return;

	bPendingKill = true;

	// 소속된 월드의 대기열에 나를 등록해달라고 요청
	if (GetWorld())
	{
		GetWorld()->RequestDestroyActor(this);
	}
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
	outJson["Properties"]["RootComponentGUID"] = ObjectReference(RootComponent);
}

void AActor::DeserializeClass(const json::JSON& inJson)
{
	std::unique_ptr<FSceneLoadScope> OwnScope;
	if (!FSceneLoadScope::Current)
	{
		OwnScope = std::make_unique<FSceneLoadScope>();
		OwnScope->Register(this, inJson);
	}
	UObject::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("Components") || propertiesJson.at("Components").JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error(std::format("{}: Components requires an array", GetRuntimeClass()->Name));
	}

	const json::JSON& componentsJson = propertiesJson.at("Components");

	if (Components.Num() == 0)
		PreloadComponents(inJson);
	if (Components.Num() != componentsJson.length())
		throw std::runtime_error("Preloaded component count mismatch");
	int Index = 0;
	for (const auto& Data : componentsJson.ArrayRange())
		Components[Index++]->DeserializeClass(Data);

	const auto& Root = propertiesJson.at("RootComponentGUID");
	if (Root.JSONType() == json::JSON::Class::String && Root.ToString() == "-1")
		RootComponent = nullptr;
	else
		RootComponent = ResolveReference<USceneComponent>(Root);
	if (RootComponent && RootComponent->GetOwner() != this)
		throw std::runtime_error("Root component belongs to another actor");
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

const TArray<UActorComponent*>& AActor::GetComponents() const
{
	return Components;
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

void AActor::PreloadComponents(const json::JSON& inJson)
{
	const auto& Data = inJson.at("Properties").at("Components");
	if (Data.JSONType() != json::JSON::Class::Array)
		throw std::runtime_error("Components requires array");
	for (const auto& Entry : Data.ArrayRange())
	{
		auto Component = PreloadObject<UActorComponent>(Entry);
		AddComponent(Component.get());
		Component.release();
	}
}
