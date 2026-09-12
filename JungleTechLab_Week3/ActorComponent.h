#pragma once

#include "Object.h"

struct FRenderInfo;

class UActorComponent : public UObject
{
	REFLECT_CLASS(UActorComponent, UObject)
public:
	UActorComponent()
		: Owner(nullptr)
	{
	}

	~UActorComponent()
	{
	}

	void SetOwner(AActor* owner)
	{
		assert(Owner == nullptr);

		Owner = owner;
	}

	AActor* GetOwner() const
	{
		return Owner;
	}

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) 
	{
	}
	virtual void AddRenderInfos(TArray<FRenderInfo>* OutRenderInfos) const {};

protected:
	AActor* Owner;
};
