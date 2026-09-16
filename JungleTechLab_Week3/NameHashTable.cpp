#include "NameHashTable.h"

FNameHashTable::FNameHashTable() : SlotCount(FIRST_SLOT_NUMBER), UsedCount(0)
{
	Slots = std::make_unique<FHashStruct[]>(SlotCount);
}

uint32 FNameHashTable::ProbeStart(uint32 Hash)
{
	return (Hash & (SlotCount - 1));
	// SlotCount가 2의 거듭제곱일 떄
	// Hash % SlotCount랑 동일한 동작을 한다
}

uint32 FNameHashTable::NextProbe(uint32 Index)
{
	return ((Index + 1) & (SlotCount - 1));
}

void FNameHashTable::Rehash()
{
	uint32 OldSlotCount = SlotCount;
	SlotCount *= 2;

	std::unique_ptr<FHashStruct[]> OldSlots = std::move(Slots);
	Slots = std::make_unique<FHashStruct[]>(SlotCount);
	UsedCount = 0;

	for (uint32 i = 0; i < OldSlotCount; i++)
	{
		if (OldSlots[i].Id != 0)
		{
			Insert(OldSlots[i].Id, OldSlots[i].Hash);
		}
	}
}

bool FNameHashTable::NeedsRehash()
{
	return (UsedCount * 10 >= SlotCount * LOAD_FACTOR);
}


uint32	FNameHashTable::Find(const char* Name, uint16 Length, uint32 Hash, FBlockArena& Arena)
{
	uint32 Index = ProbeStart(Hash);

	while (1)
	{
		if (Slots[Index].Id == 0)
			return (0);

		if (Slots[Index].Hash != Hash)
		{
			Index = NextProbe(Index);
			continue;
		}

		const FNameEntry* Entry = Arena.Resolve(Slots[Index].Id);

		if (Entry->Length == Length && FNameEntry::StringCompareToLower(Entry->Data, Name, Length))
			return (Slots[Index].Id);

		Index = NextProbe(Index);
	}
}

uint32	FNameHashTable::FindDisplayId(const char* Name, uint16 Length, uint32 Hash, FBlockArena& Arena)
{
	uint32 Index = ProbeStart(Hash);

	while (1)
	{
		if (Slots[Index].Id == 0)
			return (0);

		if (Slots[Index].Hash != Hash)
		{
			Index = NextProbe(Index);
			continue;
		}

		const FNameEntry* Entry = Arena.Resolve(Slots[Index].Id);

		if (Entry->Length == Length && FNameEntry::StringCompare(Entry->Data, Name, Length))
			return (Slots[Index].Id);

		Index = NextProbe(Index);
	}
}

void	FNameHashTable::Insert(uint32 Id, uint32 Hash)
{
	uint32 Index = ProbeStart(Hash);
	while (Slots[Index].Id != 0)
	{
		Index = NextProbe(Index);
	}
	Slots[Index].Hash = Hash;
	Slots[Index].Id = Id;
	UsedCount++;
}
