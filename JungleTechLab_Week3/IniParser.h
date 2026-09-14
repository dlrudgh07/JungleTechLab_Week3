#pragma once

#include <string_view>
#include "Core.h"
#include "FileManager.h"
#include "TArray.h"
#include "Console.h"

struct FIniEntry
{
	FString Section;
	FString Key;
	FString Value;

	FIniEntry(FString _Section, FString _Key, FString _Value) : Section(_Section), Key(_Key), Value(_Value) {}
};

class FIniParser
{
public:
	FIniParser();

	void	WriteStringToFile(FString fileName);
	void	ReadFileToString(FString fileName);

	template <typename T>
	void	SetValue(FString Section, FString Key, T& Value)
	{
		char Buffer[32];
		auto [Ptr, Ec] = std::to_chars(Buffer, Buffer + sizeof(Buffer), Value);
		if (Ec != std::errc{})
		{
			return;
		}
		std::string_view StringValue(Buffer, Ptr - Buffer);

		for (FIniEntry& CurrentEntry : IniEntry)
		{
			if (CurrentEntry.Section == Section && CurrentEntry.Key.Equals(Key))
			{
				CurrentEntry.Value = StringValue;
				return;
			}
		}
		Add(Section, Key, StringValue);
	}

	template <typename T>
	T	GetValue(FString Section, FString Key, T& _Value, T DefaultValue)
	{
		for (const FIniEntry& CurrentEntry : IniEntry)
		{
			if (CurrentEntry.Section == Section && CurrentEntry.Key.Equals(Key))
			{
				auto [Ptr, Ec] = std::from_chars(CurrentEntry.Value.Data(), CurrentEntry.Value.Data() + CurrentEntry.Value.Size(), _Value);

				if (Ec != std::errc{})
				{
					return (DefaultValue);
				}
				return _Value;
			}
		}
		return (DefaultValue);
	}

private:
	FString Section;
	TArray<FIniEntry> IniEntry;

	void ParseLine(std::string_view Line);
	void StartParser(FString File);
	void Add(FString Key, FString Value);
	void Add(FString Section, FString Key, FString Value);
	std::string_view Trim(std::string_view Text);
};
