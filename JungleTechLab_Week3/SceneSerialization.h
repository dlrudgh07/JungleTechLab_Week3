#pragma once
#include "JsonUtil.h"
#include "Object.h"
#include <memory>
#include <unordered_map>

// A load transaction resolves references only against its own preloaded objects.
class FSceneLoadScope
{
public:
	inline static thread_local FSceneLoadScope* Current = nullptr;
	std::unordered_map<std::string, UObject*> Objects;
	FSceneLoadScope(const FSceneLoadScope&) = delete;
	FSceneLoadScope& operator=(const FSceneLoadScope&) = delete;
	FSceneLoadScope() : Previous(Current)
	{
		Current = this;
	}
	~FSceneLoadScope()
	{
		Current = Previous;
	}
	void Register(UObject* Object, const json::JSON& Data)
	{
		Object->UObject::DeserializeClass(Data);
		if (!Object->ObjectID.GUID.IsValid() ||
			!Objects.emplace(Object->ObjectID.GUID.ToString().CStr(), Object).second)
			throw std::runtime_error("Invalid or duplicate scene GUID");
	}

private:
	FSceneLoadScope* Previous;
};

inline json::JSON ObjectReference(const UObject* Object)
{
	return Object ? json::JSON(Object->ObjectID.GUID.ToString()) : json::JSON();
}

template <class T> T* ResolveReference(const json::JSON& Value)
{
	if (Value.JSONType() == json::JSON::Class::Null)
		return nullptr;
	if (Value.JSONType() != json::JSON::Class::String)
		throw std::runtime_error("Expected GUID reference");
	FGuid Guid{};
	if (!Guid.Parse(FString(Value.ToString())) || !Guid.IsValid() ||
		Guid.ToString().ToLower() != FString(Value.ToString()).ToLower())
		throw std::runtime_error("Invalid reference GUID");
	UObject* Object = UObject::GetObjectByGUID(Guid);
	if (!Object || !Object->IsA<T>())
		throw std::runtime_error("Unresolved or incompatible GUID reference");
	return static_cast<T*>(Object);
}

template <class T> std::unique_ptr<T> PreloadObject(const json::JSON& Data)
{
	const auto* Class = FObjectFactory::GetClassInfoByName(FString(Data.at("ClassName").ToString()));
	std::unique_ptr<UObject> Object(FObjectFactory::ConstructUnInitializedObject(Class));
	if (!Object || !Object->IsA<T>())
		throw std::runtime_error("Invalid scene object class");
	if (FSceneLoadScope::Current)
		FSceneLoadScope::Current->Register(Object.get(), Data);
	else
		Object->UObject::DeserializeClass(Data);
	return std::unique_ptr<T>(static_cast<T*>(Object.release()));
}

// Verify the saved graph, including references held only by components.
inline void ValidateSceneReferences(const json::JSON& Scene)
{
	std::unordered_map<std::string, bool> Guids;
	std::function<void(const json::JSON&)> Collect = [&](const json::JSON& Value) {
		if (Value.JSONType() == json::JSON::Class::Object)
		{
			if (Value.hasKey("ClassName"))
			{
				auto Guid = Value.at("Properties").at("GUID").ToString();
				if (!Guids.emplace(Guid, true).second)
					throw std::runtime_error("Duplicate saved GUID");
			}
			for (const auto& [Key, Child] : Value.ObjectRange())
				Collect(Child);
		}
		else if (Value.JSONType() == json::JSON::Class::Array)
		{
			for (const auto& Child : Value.ArrayRange())
				Collect(Child);
		}
	};
	Collect(Scene);
	auto Check = [&](const json::JSON& Value) {
		if (Value.JSONType() == json::JSON::Class::Null || Value.ToString() == "-1")
			return;
		if (Value.JSONType() != json::JSON::Class::String || !Guids.contains(Value.ToString()))
			throw std::runtime_error("Scene references an unregistered object or asset");
	};
	std::function<void(const json::JSON&)> Visit = [&](const json::JSON& Value) {
		if (Value.JSONType() == json::JSON::Class::Object)
		{
			for (const auto& [Key, Child] : Value.ObjectRange())
			{
				if (Key != "GUID" && Key.ends_with("GUID"))
					Check(Child);
				else if (Key == "Materials" || Key == "OverrideMaterials")
				{
					for (const auto& Ref : Child.ArrayRange())
						Check(Ref);
				}
				else
					Visit(Child);
			}
		}
		else if (Value.JSONType() == json::JSON::Class::Array)
		{
			for (const auto& Child : Value.ArrayRange())
				Visit(Child);
		}
	};
	Visit(Scene);
}
