#pragma once

#include "Core.h"
#include <assert.h>

#define MAX_BLOCK_NUMBER 8192
#define BLOCK_SIZE 2048
#define BLOCK_BIT 13
#define OFFSET_BIT 16 // MemoryAlign(8)로 나눠서 저장해서 512KB사용
#define MEMORY_ALIGN 8
#define HEADDER_SIZE 2

#pragma warning(push)
#pragma warning(disable: 4200)

struct FNameEntry
{
	uint16	Length;
	char	Data[0];

	static uint32 CalcSize(uint32 Len)
	{
		return (HEADDER_SIZE + Len);
	}

	static FNameEntry* PlaceAt(void* Memory, const char* Name, uint16 Length)
	{
		FNameEntry* NameEntry = static_cast<FNameEntry*>(Memory);

		NameEntry->Length = Length;
		std::memcpy(NameEntry->Data, Name, Length);

		return (NameEntry);
	}

	char* GetChars()
	{
		return Data;
	}

	static char ToLower(char Word)
	{
		if (Word <= 'Z' && Word >= 'A')
			return (Word + 32);
		return (Word);
	}

	static bool StringCompareToLower(const char* Src, const char* Dest, uint16 Length)
	{
		for (int i = 0; i < Length; i++)
		{
			if (ToLower(Src[i]) != ToLower(Dest[i]))
				return (false);
		}
		return (true);
	}

	static bool StringCompare(const char* Src, const char* Dest, uint16 Length)
	{
		for (int i = 0; i < Length; i++)
		{
			if (Src[i] != Dest[i])
				return (false);
		}
		return (true);
	}

};

#pragma warning(pop)


struct FAllocateResult {
	uint32 Id;
	uint8* Memory;
};

class FBlockArena {

public:
	FBlockArena();
	~FBlockArena();

	FAllocateResult	Allocate(uint32 Size);
	const FNameEntry* Resolve(uint32 Id) const;



private:
	std::unique_ptr<uint8[]> Blocks[MAX_BLOCK_NUMBER]; // 블록 시작 주소들 BlockBits가 13bit로 8192개
	uint32	CurrentBlock; // 현재 쓰고 있는 블록
	uint32	Cursor; // 블록 에서 어디까지 사용했는지

	bool	AllocateNewBlock();
	uint32	AlignUp(uint32 Size) const;
	bool	HasSpace(uint32 Size) const;

};
