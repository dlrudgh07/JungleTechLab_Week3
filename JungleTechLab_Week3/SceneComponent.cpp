#include "SceneComponent.h"
#include "PrimitiveComponent.h"
#include <format>

#include "Transform.h"
#include "JsonUtil.h"

void USceneComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	UActorComponent::Initialize();

	RelativeLocation = location;
	RelativeRotation = FQuaternion::FromEuler(rotation);
	RelativeScale3D = scale3D;
}

USceneComponent::~USceneComponent()
{
}

void USceneComponent::SerializeClass(json::JSON& outJson) const
{
	UActorComponent::SerializeClass(outJson);
	outJson["Properties"]["RelativeLocation"] = FVectorToJson(RelativeLocation);
	outJson["Properties"]["RelativeRotation"] = FRotatorToJson(RelativeRotation.ToEuler());   // 파일에는 각도 3개로
	outJson["Properties"]["RelativeScale3D"] = FVectorToJson(RelativeScale3D);
}

void USceneComponent::DeserializeClass(const json::JSON& inJson)
{
	UActorComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("RelativeLocation")
		|| propertiesJson.at("RelativeLocation").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("RelativeLocation").length() != 3)
	{
		throw std::runtime_error(std::format("{}: RelativeLocation property requires an array of length 3", GetRuntimeClass()->Name));
	}

	if (!propertiesJson.hasKey("RelativeRotation")
		|| propertiesJson.at("RelativeRotation").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("RelativeRotation").length() != 3)
	{
		throw std::runtime_error(std::format("{}: RelativeRotation property requires an array of length 3", GetRuntimeClass()->Name));
	}

	if (!propertiesJson.hasKey("RelativeScale3D")
		|| propertiesJson.at("RelativeScale3D").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("RelativeScale3D").length() != 3)
	{
		throw std::runtime_error(std::format("{}: RelativeScale3D property requires an array of length 3", GetRuntimeClass()->Name));
	}

	RelativeLocation = FVectorFromJson(propertiesJson.at("RelativeLocation"));
	RelativeRotation = FQuaternion::FromEuler(FRotatorFromJson(propertiesJson.at("RelativeRotation")));
	RelativeScale3D = FVectorFromJson(propertiesJson.at("RelativeScale3D"));
}

FVector USceneComponent::GetRelativeLocation() const
{
	return RelativeLocation;
}

void USceneComponent::SetRelativeLocation(FVector location)
{
	RelativeLocation = location;

	if (IsA<UPrimitiveComponent>())
	{
		static_cast<UPrimitiveComponent*>(this)->UpdateBounds();
	}
}

FQuaternion USceneComponent::GetRelativeRotation() const
{
	return RelativeRotation;
}

void USceneComponent::SetRelativeRotation(FRotator rotation)
{
	SetRelativeRotation(FQuaternion::FromEuler(rotation));
}

void USceneComponent::SetRelativeRotation(FQuaternion rotation)
{
	RelativeRotation = rotation;

	if (IsA<UPrimitiveComponent>())
	{
		static_cast<UPrimitiveComponent*>(this)->UpdateBounds();
	}
}

FVector USceneComponent::GetRelativeScale3D() const
{
	return RelativeScale3D;
}

void USceneComponent::SetRelativeScale3D(FVector scale)
{
	RelativeScale3D = scale;

	if (IsA<UPrimitiveComponent>())
	{
		static_cast<UPrimitiveComponent*>(this)->UpdateBounds();
	}
}

FTransform USceneComponent::GetTransformMatrix() const
{
	return FTransform(RelativeLocation, RelativeRotation, RelativeScale3D);
}
