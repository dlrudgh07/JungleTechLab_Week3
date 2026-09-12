#include "ObjectFactory.h"

#include "Json/json.hpp"

#include "Actor.h"
#include "PrimitiveComponent.h"
#include "FResourceManager.h"
#include "FFontAsset.h"
#include "FTexture.h"
#include <d3d11.h>

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

AActor* FObjectFactory::SpawnPrimitiveActor(
	EPrimitive primitiveType,
	FVector3 Location, FRotator Rotation, FVector3 Scale, FCharDataInfo CharInfo)
{
	ID3D11ShaderResourceView* TempSRV = nullptr;
	const FCharacterInfo* TempInfo = nullptr;
	FCharDataInfo TempDataInfo;
	if (primitiveType == EPrimitive::EP_Quad)
	{
		FFontAsset* TempAsset = FResourceManager::Get().FindFont(0);
		//TempSRV = TempAsset->GetPageTexture(TempAsset->FindCharInfo(uint32("가"))->Page)->GetSRV();
		TempSRV = TempAsset->GetPageTexture(TempAsset->FindCharInfo(static_cast<int32>('A'))->Page)->GetSRV();
		TempInfo = TempAsset->FindCharInfo(97);
		TempDataInfo.CharX = TempInfo->X;
		TempDataInfo.CharY = TempInfo->Y;
		TempDataInfo.AtlasWidth = TempAsset->GetAtlasWidth();
		TempDataInfo.AtlasHeight = TempAsset->GetAtlasHeight();
		TempDataInfo.CharHeight = TempInfo->Height;
		TempDataInfo.CharWidth = TempInfo->Width;
	}
	// Create a new actor
	AActor* actor = ConstructObject<AActor>();
	
	UPrimitiveComponent* component = ConstructObject<UPrimitiveComponent>(
		primitiveType, Location, Rotation, Scale, TempDataInfo, TempSRV);

	actor->AddRootSceneComponent(component);

	return actor;
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

#include "SceneComponent.h"
#include "PrimitiveComponent.h"
#include "CubeComponent.h"
#include "SphereComponent.h"
#include "World.h"

TMap<FString, std::function<const FClassInfo* ()>> FObjectFactory::mClassInfoMap = {
	{"UObject", &UObject::GetClass },
	{"AActor", &AActor::GetClass },
	{"UActorComponent", &UActorComponent::GetClass },
	{"USceneComponent", &USceneComponent::GetClass },
	{"UPrimitiveComponent", &UPrimitiveComponent::GetClass },
	{"UCubeComponent", &UCubeComponent::GetClass },
	{"USphereComponent", &USphereComponent::GetClass },
	{"UWorld", &UWorld::GetClass }
};
