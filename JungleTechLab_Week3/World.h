#pragma once

#include "Object.h"
#include "Actor.h"

#include "RenderInfo.h"

class FResourceManager;

class UWorld final : public UObject
{
	REFLECT_CLASS(UWorld, UObject)
public:
	UWorld() = default;
	virtual ~UWorld();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	void AddActor(AActor* actor);
	bool RemoveActor(FGuid TargetComponentGuid);

	const TArray<FRenderInfo> GetRenderInfos();
	TArray<AActor*>& GetActors() { return Actors; }

	void Update(float DeltaTime);
	void ClearRenderInfos();

	AActor* SpawnStaticMeshActor(const std::string& AssetName, struct FTransform Transform, const FResourceManager& ResourceManager);

private:
	int32 GetActorIndex(FGuid TargetGuid) const;

private:
	enum
	{
		DEFAULT_RESERVE_MEM = 1024U
	};
	
	// Todo: Must reserve
	TArray<AActor*> Actors;

	// Todo: Maybe, move to FSceneManager
	TArray<FRenderInfo> RenderInfos;
};
