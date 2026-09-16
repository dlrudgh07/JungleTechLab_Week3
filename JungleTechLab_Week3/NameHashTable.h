#pragma once

#include "Core.h"
#include "BlockArena.h"

#define FIRST_SLOT_NUMBER 1024
#define LOAD_FACTOR 7// 실제로는 0.7 

struct FHashStruct{
	uint32 Hash;
	uint32 Id;
};

class FNameHashTable
{
public:
	FNameHashTable();
	~FNameHashTable() = default;

	uint32	Find(const char* Name, uint16 Length, uint32 Hash, FBlockArena& Arena);
	uint32	FindDisplayId(const char* Name, uint16 Length, uint32 Hash, FBlockArena& Arena);
	void	Insert(uint32 Id, uint32 Hash);
	void	Rehash();
	bool	NeedsRehash();

private:
	std::unique_ptr<FHashStruct[]> Slots;   // 각 칸에 ID 하나
	uint32 SlotCount;                  // 항상 2의 거듭제곱
	uint32 UsedCount;                  // 채워진 칸 수

	uint32 ProbeStart(uint32 Hash);
	uint32 NextProbe(uint32 Index);
	
};
