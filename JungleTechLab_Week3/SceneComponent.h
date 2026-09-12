#pragma once

#include "ActorComponent.h"
#include "GraphicsManager.h"

#include "Vector.h"

struct FTransform;

class USceneComponent : public UActorComponent
{
public:
	USceneComponent() = default;

	// factory 에서 필요함
	void Initialize() {};
	void Initialize(FVector Location, FRotator Rotation, FVector Scale3D);
	virtual ~USceneComponent();

	virtual void SerializeClass(json::JSON& OutJson) const override;
	virtual void DeserializeClass(const json::JSON& InJson) override;

	FVector GetRelativeLocation() const;
	void SetRelativeLocation(FVector location);

	FRotator GetRelativeRotation() const;
	void SetRelativeRotation(FRotator rotation);

	FVector GetRelativeScale3D() const;
	void SetRelativeScale3D(FVector scale);

	FTransform GetTransformMatrix() const;

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override
	{
		UActorComponent::Update(OutRenderInfos, DeltaTime);
	}

	REFLECT_CLASS(USceneComponent, UActorComponent)

private:
	FVector RelativeLocation;
	FRotator RelativeRotation;
	FVector RelativeScale3D;
};
