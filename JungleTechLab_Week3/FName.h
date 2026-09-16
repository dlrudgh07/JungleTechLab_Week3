#pragma once
#include "Core.h"


struct FName
{
public:
	FName();
	FName(const char* pStr);
	FName(const char* pStr, uint32 Length);
	FName(const FString& str);

	bool				IsNone() const;

	FString				ToString() const;
	std::string_view	ToStringView() const;

	void				SetName(const char* pStr);
	void				SetName(const char* pStr, uint32 Length);
	void				SetName(const FString& str);

	uint32				GetLength() const;
	uint32				GetComparisonIndex() const;
	uint32				GetDisplayIndex() const;

	int32				CompareIndexes(const FName& Other) const;
	int32				Compare(const FName& Other) const;

	bool				FastLess(const FName& Other) const;
	bool				LexicalLess(const FName& Other) const;

	bool operator==(const FName& Other) const
	{
		return (ComparisonIndex == Other.ComparisonIndex);
	}

	bool operator!=(const FName& Other) const
	{
		return (ComparisonIndex != Other.ComparisonIndex);
	}

	bool operator<(const FName& Other) = delete;
	bool operator>(const FName& Other) = delete;
	bool operator<=(const FName& Other) = delete;
	bool operator>=(const FName& Other) = delete;

private:

	uint32 ComparisonIndex;
	uint32 DisplayIndex;
};



struct FNameFastLess
{
	bool operator()(const FName& A, const FName& B) const
	{
		return A.FastLess(B);
	}
};

struct FNameLexicalLess
{
	bool operator()(const FName& A, const FName& B) const
	{
		return A.LexicalLess(B);
	}
};
