#pragma once

#include "BlockArena.h"
#include "NameHashTable.h"

#define MAX_NAME_LENGTH 1023

struct FNameIds {
	uint32 ComparisonId;
	uint32 DisplayId;

	FNameIds() : ComparisonId(0), DisplayId(0) {}
	FNameIds(uint32 _ComparisonId, uint32 _DisplayId) :
		ComparisonId(_ComparisonId), DisplayId(_DisplayId) {}
};

class FNamePool
{
public:
	FNamePool();
	~FNamePool() = default;

	FNameIds	FindOrAdd(const char* Name, uint32 Length);
	FNameIds	Find(const char* Name, uint32 Length);
	const FNameEntry* Resolve(uint32 Id) const;
	
	static FNamePool& GetInstance();
	


private:
	uint32 HashString(const char* Name, uint32 Length);
	uint32 StoreEntry(const char* Name, uint32 Length);

	FBlockArena Arena;
	FNameHashTable HashTable;

};
