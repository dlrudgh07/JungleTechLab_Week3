#pragma once

#include "Object.h"
#include "ActorComponent.h"

class UWorld;
struct FRenderInfo;
struct FTransform;
class USceneComponent;

class AActor : public UObject
{
	REFLECT_CLASS(AActor, UObject)
public:
	AActor() = default;
	virtual ~AActor();

	virtual void Destroy();

	void Initialize();
	void Initialize(UWorld* InputWorld) { World = InputWorld; }
	UWorld* GetWorld() const { return World; }
	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	void AddComponent(UActorComponent* actorComponent);
	void AddRootSceneComponent(USceneComponent* sceneComponent);
	bool RemoveComponent(FGuid TargetComponentGuid);

	FTransform GetTransform() const;

	virtual void Update(TArray<FRenderInfo>* outRenderInfos, float DeltaTime);

	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const;
	bool GetFirstRenderInfo(FRenderInfo& outRenderInfo) const;

	void SetLocation(FVector location);
	void SetRotation(FRotator rotation);
	void SetScale(FVector scale);

	bool IsPendingKill() const {
		return bPendingKill;
	};

private:
	int32 GetComponentIndex(FGuid TargetComponentGuid) const;

private:
	
	USceneComponent* RootComponent = nullptr;
	TArray<UActorComponent*> Components;

	UWorld* World = nullptr;

	bool bPressed = false;
	bool bStarted = false;
	bool bPendingKill = false;
};

