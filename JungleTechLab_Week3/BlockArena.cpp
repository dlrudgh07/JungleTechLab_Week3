#include "BlockArena.h"

FBlockArena::FBlockArena() : CurrentBlock(0), Cursor(0)
{
	Blocks[0] = std::make_unique<uint8[]>(BLOCK_SIZE);
}

FBlockArena::~FBlockArena()
{

}

FAllocateResult FBlockArena::Allocate(uint32 Size)
{
	FAllocateResult AllocateResult;
	Size = AlignUp(Size);
	if (!HasSpace(Size))
	{
		AllocateNewBlock();
		assert(HasSpace(Size));
	}

	AllocateResult.Id = (CurrentBlock << OFFSET_BIT) | (Cursor / MEMORY_ALIGN);
	AllocateResult.Memory = Blocks[CurrentBlock].get() + Cursor;
	Cursor += Size;

	return AllocateResult;
}

const FNameEntry* FBlockArena::Resolve(uint32 Id) const
{
	uint32 BlockIdx = Id >> OFFSET_BIT;
	uint32 Offset = Id & ((1 << OFFSET_BIT) - 1);

	return (reinterpret_cast<const FNameEntry*>(Blocks[BlockIdx].get() + Offset * MEMORY_ALIGN));
}

bool	FBlockArena::AllocateNewBlock()
{
	if (CurrentBlock + 1 >= MAX_BLOCK_NUMBER)
		return (false);
	CurrentBlock++;
	Blocks[CurrentBlock] = std::make_unique<uint8[]>(BLOCK_SIZE);
	Cursor = 0;
	return (true);
}

uint32 FBlockArena::AlignUp(uint32 Size) const // 할당할 메모리 사이즈를 8의 배수로 맞춤
{
	if (Size % MEMORY_ALIGN != 0)
		return (Size + MEMORY_ALIGN - (Size % MEMORY_ALIGN));
	return (Size);
}

bool	FBlockArena::HasSpace(uint32 Size) const
{
	return (Cursor + Size <= BLOCK_SIZE);
}
