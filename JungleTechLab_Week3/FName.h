#pragma once
#include <cstdint>
#include <string_view>
#include <string>

using FNameEntryId = uint32_t;

class FName
{
private:
	FNameEntryId ComparisonIndex; // 비교용 (소문자 전용)
	FNameEntryId DisplayIndex;    // 출력용 (원본 대소문자 전용)

public:
	FName(std::string_view InputString);
	bool operator==(const FName& Other) const;
	std::string ToString() const;
};
