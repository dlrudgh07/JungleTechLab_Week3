#pragma once
#include <cstdint>
#include <string_view>
#include "Core.h"

using FNameEntryId = uint32;

class FName
{
private:
	FNameEntryId ID;

public:
	FName(std::string_view InString);
	bool operator==(const FName& Other) const;
	std::string ToString() const;
};
