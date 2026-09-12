#include "ObjectFactory.h"

#include "Json/json.hpp"
#include "Actor.h"
#include "PrimitiveComponent.h"

UObject* FObjectFactory::ConstructUnInitializedObject(const FClassInfo* classInfo)
{
	if (!classInfo || !classInfo->Constructor)
	{
		return nullptr;
	}

	UObject* instance = classInfo->CreateInstance();
	if (instance)
	{
		instance->mClassInfo = classInfo;
	}
	return instance;
}

UObject* FObjectFactory::LoadObject(const FClassInfo* classInfo, const json::JSON& inJson)
{
	UObject* instance = ConstructUnInitializedObject(classInfo);

	if (instance)
	{
		instance->DeserializeClass(inJson);
	}
	return instance;
}


const FClassInfo* FObjectFactory::GetClassInfoByName(const FString& className)
{
	if (!mClassInfoMap.Contains(className))
	{
		return nullptr;
	}

	return mClassInfoMap[className]();
}

bool FObjectFactory::RegisterClassInfo(FString className, const FClassInfo* classInfo)
{
	if (mClassInfoMap.Contains(className))
	{
		return false;
	}
	mClassInfoMap.Add(className, [classInfo]() -> const FClassInfo* { return classInfo; });
	return true;
}

#include "World.h"
#include "StaticMeshComponent.h"

TMap<FString, std::function<const FClassInfo* ()>> FObjectFactory::mClassInfoMap = {
	{"UObject", &UObject::GetClass },
	{"AActor", &AActor::GetClass },
	{"UActorComponent", &UActorComponent::GetClass },
	{"USceneComponent", &USceneComponent::GetClass },
	{ "UPrimitiveComponent", &UPrimitiveComponent::GetClass },
	{ "UMeshComponent", &UMeshComponent::GetClass },
	{ "UStaticMeshComponent", &UStaticMeshComponent::GetClass },
	{ "UMaterial", &UMaterial::GetClass },
	{ "UStaticMesh", &UStaticMesh::GetClass },
	{"UTexture", &UTexture::GetClass },
	{"UWorld", &UWorld::GetClass }
};
