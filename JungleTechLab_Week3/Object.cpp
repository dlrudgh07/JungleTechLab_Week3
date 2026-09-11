
#include "Object.h"
#include "EngineStatics.h"
#include "Json/json.hpp"

TSparseArray<UObject*> UObject::GUObjectArray;

UObject* FClassInfo::CreateInstance() const
{
	if (Constructor)
	{
		return Constructor();
	}
	return nullptr;
}


UObject::UObject() 
{
	ObjectID.InternalIndex = GUObjectArray.Add(this);
	ObjectID.GUID = FGuid::NewGuid();
	GUObjectRevision++;
}

UObject::~UObject()
{
	/*
	// Ensure that the object is in the GUObjectArray before attempting to remove it
	if (GUObjectArray.Num() < InternalIndex || GUObjectArray[InternalIndex] != this)
	{
		assert(false && "Invalid InternalIndex or GUObjectArray mismatch.");
		return;
	}
	*/

	GUObjectArray.RemoveAt(ObjectID.InternalIndex);
	GUObjectRevision++;
}

void UObject::Destroy()
{
	delete this;
}

void UObject::Initialize()
{
	if (ObjectID.GUID.IsValid() == false)
		ObjectID.GUID = FGuid::NewGuid();
}

const FClassInfo* UObject::GetClass()
{
	static FClassInfo classInstance = FClassInfo(
		"UObject",
		nullptr,
		[]() -> UObject* { return new UObject(); }
	);
	return &classInstance;
}

void UObject::SerializeClass(json::JSON& outJson) const
{
	outJson["ClassName"] = GetRuntimeClass()->Name;

	json::JSON propertiesJson = json::JSON::Make(json::JSON::Class::Object);
	propertiesJson["GUID"] = ObjectID.GUID.ToString();
	outJson["Properties"] = propertiesJson;
}

void UObject::DeserializeClass(const json::JSON& inJson)
{
	if (!inJson.hasKey("Properties") || inJson.at("Properties").JSONType() != json::JSON::Class::Object)
	{
		throw std::runtime_error("Invalid JSON format for Properties");
	}
	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("GUID") || propertiesJson.at("GUID").JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error("Invalid JSON format for GUID");
	}

	ObjectID.GUID.Parse(FString{ propertiesJson.at("GUID").ToString() });
}

bool UObject::IsA(const FClassInfo* classInfo) const
{
	const FClassInfo* currentClass = GetRuntimeClass();
	while (currentClass)
	{
		if (currentClass == classInfo)
		{
			return true;
		}
		currentClass = currentClass->SuperClass;
	}
	return false;
}

UObject* UObject::GetObjectByGUID(FGuid TargetGuid)
{
	for (const auto& object : GUObjectArray)
	{
		if (object && object->ObjectID.GUID == TargetGuid)
		{
			return object;
		}
	}
	return nullptr;
}

UObject* UObject::GetObjectByInternalIndex(uint32 internalIndex)
{
	if (GUObjectArray.IsValidIndex(internalIndex))
	{
		return GUObjectArray[internalIndex];
	}
	return nullptr;
}
