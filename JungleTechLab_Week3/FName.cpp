#include "FName.h"

#include "NamePool.h"

FName::FName() : ComparisonIndex(0), DisplayIndex(0){}
FName::FName(const char* pStr)
{
	SetName(pStr);
}

FName::FName(const char* pStr, uint32 Length)
{
	SetName(pStr, Length);
}

FName::FName(const FString& str)
{
	SetName(str);
}

void FName::SetName(const char* pStr)
{
	FNameIds NameIds = FNamePool::GetInstance().FindOrAdd(pStr, static_cast<uint32>(std::strlen(pStr)));

	ComparisonIndex = NameIds.ComparisonId;
	DisplayIndex = NameIds.DisplayId;
}

void FName::SetName(const char* pStr, uint32 Length)
{
	FNameIds NameIds = FNamePool::GetInstance().FindOrAdd(pStr, Length);

	ComparisonIndex = NameIds.ComparisonId;
	DisplayIndex = NameIds.DisplayId;
}

void FName::SetName(const FString& str)
{
	FNameIds NameIds = FNamePool::GetInstance().FindOrAdd(str.CStr(), static_cast<uint32>(str.Len()));

	ComparisonIndex = NameIds.ComparisonId;
	DisplayIndex = NameIds.DisplayId;
}

bool FName::IsNone() const
{
	return (ComparisonIndex == 0);
}

FString FName::ToString() const
{
	const FNameEntry* NameEntry = FNamePool::GetInstance().Resolve(DisplayIndex);

	return (FString(std::string_view(NameEntry->Data, NameEntry->Length)));

}

std::string_view FName::ToStringView() const
{
	const FNameEntry* NameEntry = FNamePool::GetInstance().Resolve(DisplayIndex);

	return (std::string_view(NameEntry->Data, NameEntry->Length));
}

uint32 FName::GetLength() const
{
	const FNameEntry* NameEntry = FNamePool::GetInstance().Resolve(DisplayIndex);

	return (NameEntry->Length);
}

uint32	FName::GetComparisonIndex() const
{
	return (ComparisonIndex);
}

uint32	FName::GetDisplayIndex() const
{
	return (DisplayIndex);
}

int32 FName::CompareIndexes(const FName& Other) const
{
	return static_cast<int32>(ComparisonIndex) - static_cast<int32>(Other.ComparisonIndex);
}

int32 FName::Compare(const FName& Other) const
{
	if (ComparisonIndex == Other.ComparisonIndex)
		return 0;

	const FNamePool& Pool = FNamePool::GetInstance();
	const FNameEntry* A = Pool.Resolve(ComparisonIndex);
	const FNameEntry* B = Pool.Resolve(Other.ComparisonIndex);

	const uint32 MinLength = (A->Length < B->Length) ? A->Length : B->Length;

	for (uint32 i = 0; i < MinLength; ++i)
	{
		const char LowerA = FNameEntry::ToLower(A->Data[i]);
		const char LowerB = FNameEntry::ToLower(B->Data[i]);
		if (LowerA != LowerB)
			return static_cast<int32>(LowerA) - static_cast<int32>(LowerB);
	}

	return static_cast<int32>(A->Length) - static_cast<int32>(B->Length);
}

bool FName::FastLess(const FName& Other) const
{
	return CompareIndexes(Other) < 0;
}

bool FName::LexicalLess(const FName& Other) const
{
	return Compare(Other) < 0;
}
