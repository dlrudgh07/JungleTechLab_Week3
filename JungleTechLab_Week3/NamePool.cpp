#include "NamePool.h"

FNamePool::FNamePool()
{
	assert(StoreEntry("None", 4) == 0);
}

FNamePool& FNamePool::GetInstance()
{
	static FNamePool instance;
	return instance;
}

FNameIds	FNamePool::FindOrAdd(const char* Name, uint32 Length)
{
	uint32 checkLength = Length;
	assert(!(Length == 0 || Length > MAX_NAME_LENGTH));

	char LowerBuffer[MAX_NAME_LENGTH];
	for (uint32 i = 0; i < Length; i++)
	{
		LowerBuffer[i] = FNameEntry::ToLower(Name[i]);
	}
	if (FNameEntry::StringCompare(LowerBuffer, "none", 4))
		return (FNameIds(0, 0));

	uint32 Hash = HashString(LowerBuffer, Length);
	uint32 Existing = HashTable.Find(LowerBuffer, Length, Hash, Arena);
	FNameIds NameIds;

	if (Existing != 0)
	{
		const FNameEntry* Entry = Arena.Resolve(Existing);
		NameIds.ComparisonId = Existing;
		if (FNameEntry::StringCompare(Entry->Data, Name, Length))
		{
			NameIds.DisplayId = Existing;
		}
		else
		{
			Existing = HashTable.FindDisplayId(Name, Length, Hash, Arena);
			if (Existing != 0)
			{
				NameIds.DisplayId = Existing;
			}
			else
			{
				NameIds.DisplayId = StoreEntry(Name, Length);
				HashTable.Insert(NameIds.DisplayId, Hash);
				if (HashTable.NeedsRehash())
					HashTable.Rehash();
			}
			
		}
	}
	else
	{
		uint32 NewId = StoreEntry(Name, Length);
		HashTable.Insert(NewId, Hash);
		if (HashTable.NeedsRehash())
			HashTable.Rehash();
		NameIds.ComparisonId = NewId;
		NameIds.DisplayId = NewId;
	}

	return (NameIds);
}

FNameIds	FNamePool::Find(const char* Name, uint32 Length)
{
	assert(!(Length == 0 || Length > MAX_NAME_LENGTH));

	char LowerBuffer[MAX_NAME_LENGTH];
	for (uint32 i = 0; i < Length; i++)
	{
		LowerBuffer[i] = FNameEntry::ToLower(Name[i]);
	}

	uint32 Hash = HashString(LowerBuffer, Length);
	uint32 Existing = HashTable.Find(LowerBuffer, Length, Hash, Arena);

	if (Existing != 0)
	{
		return (FNameIds(Existing, Existing));
	}
	else
	{
		return (FNameIds(0, 0));
	}
}

const FNameEntry* FNamePool::Resolve(uint32 Id) const
{
	return (Arena.Resolve(Id));
}

uint32 FNamePool::StoreEntry(const char* Name, uint32 Length)
{
	uint32 Size = FNameEntry::CalcSize(Length);
	FAllocateResult AllocateResult = Arena.Allocate(Size);
	FNameEntry::PlaceAt(AllocateResult.Memory, Name, Length);
	return (AllocateResult.Id);
}

static constexpr uint32 FNV_OFFSET_BASIS = 2166136261u;
static constexpr uint32 FNV_PRIME = 16777619u;

uint32 FNamePool::HashString(const char* Name, uint32 Length) //FNV-1a알고리즘
{
	uint32 Hash = FNV_OFFSET_BASIS;

	for (uint32 i = 0; i < Length; ++i)
	{
		Hash ^= static_cast<uint8>(Name[i]);
		Hash *= FNV_PRIME;
	}

	return Hash;
}
