#pragma once

#include <functional>

#include "Core.h"
#include "TArray.h"
#include "TSparseArray.h"
#include "ObjectFactory.h"

#include "Guid.h"


namespace json { class JSON; }

class UObject;

//using ConstructorFunc = UObject * (*)();

struct FClassInfo
{
	FString Name;
	const FClassInfo* SuperClass;
	std::function<UObject* ()> Constructor;

	FClassInfo(FString name, const FClassInfo* superClass, std::function<UObject* ()> constructor)
		: Name(std::move(name)), SuperClass(superClass), Constructor(constructor) {
	}

	UObject* CreateInstance() const;

private:
};

struct FObjectID
{
	FGuid GUID;
	uint32 InternalIndex;
};

class UObject
{
public:
	FObjectID ObjectID{};

	virtual ~UObject();
	virtual void Destroy();

	void Initialize();

	// StaticClass() in Unreal Engine
	static const FClassInfo* GetClass();

	// GetClass() in Unreal Engine
	inline const FClassInfo* GetRuntimeClass() const { return mClassInfo; }

	// TODO?: Replace json type with a more generic type, such as a variant or a map
	virtual void SerializeClass(json::JSON& outJson) const;
	virtual void DeserializeClass(const json::JSON& inJson);

	template<typename TObject>
		requires std::derived_from<TObject, UObject>
	bool IsA() const;

	bool IsA(const FClassInfo* classInfo) const;

	template<typename TObject>
		requires std::derived_from<TObject, UObject>
	TObject* Cast();

	static UObject* GetObjectByGUID(FGuid TargetGuid);
	static UObject* GetObjectByInternalIndex(uint32 internalIndex);

	template<typename TObject>
		requires std::derived_from<TObject, UObject>
	static TObject* GetObjectByGUID(FGuid TargetGuid);

	template<typename TObject>
		requires std::derived_from<TObject, UObject>
	static TObject* GetObjectByInternalIndex(uint32 internalIndex);

	static TSparseArray<UObject*>& GetGObjectArray() { return GUObjectArray; }
	inline static uint64 GetGObjectRevision() { return GUObjectRevision; }

public:
	static TSparseArray<UObject*> GUObjectArray;

protected:
	UObject();
	inline static uint64 GUObjectRevision = 0;

private:

	friend struct FObjectFactory;
	const FClassInfo* mClassInfo = nullptr;
};


#include  "Object.inl"
