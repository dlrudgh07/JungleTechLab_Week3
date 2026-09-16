#pragma once

#include "ActorComponent.h"

#include "Vector.h"
#include "Quaternion.h"

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

	FQuaternion GetRelativeRotation() const;
	void SetRelativeRotation(FRotator rotation);      // 오일러 입력 -> 내부에서 쿼터니언으로 변환
	void SetRelativeRotation(FQuaternion rotation);

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
	FQuaternion RelativeRotation;
	FVector RelativeScale3D;
};
