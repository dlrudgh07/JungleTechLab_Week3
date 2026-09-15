#include "IniParser.h"

FIniParser::FIniParser() : Section("") {}

std::string_view FIniParser::Trim(std::string_view Text)
{
	constexpr std::string_view Spaces = " \t\r\n";

	size_t First = Text.find_first_not_of(Spaces);
	if (First == std::string_view::npos)
	{
		return {};
	}

	size_t Last = Text.find_last_not_of(Spaces);
	return Text.substr(First, Last - First + 1);
}

void FIniParser::Add(FString Section, FString Key, FString Value)
{
	for (FIniEntry& Entry : IniEntry)
	{
		if (Entry.Section == Section && Entry.Key.Equals(Key))
		{
			Entry.Value = Value;
			return;
		}
	}
	IniEntry.Add(FIniEntry(Section, Key, Value));
}

void FIniParser::Add(FString Key, FString Value)
{
	for (FIniEntry& Entry : IniEntry)
	{
		if (Entry.Section == Section && Entry.Key.Equals(Key))
		{
			Entry.Value = Value;
			return;
		}
	}
	IniEntry.Add(FIniEntry(Section, Key, Value));
}

void FIniParser::ParseLine(std::string_view Line)
{
	

	size_t Annotation = Line.find(';');
	if (Annotation != std::string_view::npos)
	{
		Line = Line.substr(0, Annotation);
	}

	Line = Trim(Line);
	if (Line.empty())
		return;

	if (Line[0] == '[')
	{
		size_t Last = Line.find(']');
		if (Last == std::string_view::npos)
			return;
		std::string_view TempStringView = Trim(Line.substr(1, Last - 1));
		if (!TempStringView.empty())
			Section = TempStringView;
		return;
	}
	if (Section.Empty())
		return;
	size_t Pivot = Line.find('=');
	if (Pivot == std::string_view::npos)
	{
		return;
	}
	FString Key = Line.substr(0, Pivot);
	Key = Trim(Key);
	if (Key.Empty())
		return;

	FString Value = Line.substr(Pivot + 1);
	Value = Trim(Value);
	if (Value.Empty())
		return;
	Add(Key, Value);
}

void FIniParser::StartParser(FString File)
{
	std::string_view ParserFile = File;
	while (!ParserFile.empty())
	{
		std::string_view Line;

		size_t NewLine = ParserFile.find('\n');
		if (NewLine == std::string_view::npos)
		{
			Line = ParserFile;
			ParserFile = {};
		}
		else
		{
			Line = ParserFile.substr(0, NewLine);
			ParserFile.remove_prefix(NewLine + 1);
		}
		ParseLine(Line);
	}
	;
}

void FIniParser::ReadFileToString(FString fileName)
{
	try {
		FFileManager FileManager(nullptr);
		FString File = FileManager.ReadFileToString(fileName);
		StartParser(File);

	}
	catch (const std::exception& e) {
		UE_LOG(e.what());
	}
}

void FIniParser::WriteStringToFile(FString FilePath)
{
	FString WriteStringView;
	FString CurrentSection;

	if (IniEntry.IsEmpty())
	{
		UE_LOG("IniEntry is Empty");
		return;
	}
	CurrentSection = IniEntry[0].Section;

	WriteStringView.Append((FString("[").Append(FString(CurrentSection)).Append(FString("]"))).Append(FString("\n")));

	for (const FIniEntry& CurrentEntry : IniEntry)
	{
		if (CurrentSection != CurrentEntry.Section)
		{
			WriteStringView.Append((FString("[").Append(FString(CurrentEntry.Section)).Append(FString("]"))).Append(FString("\n")));
			CurrentSection = CurrentEntry.Section;
		}
		WriteStringView.Append(FString(CurrentEntry.Key).Append(FString(" = ")).Append(CurrentEntry.Value).Append(FString("\n")));
	}
	
	FFileManager FileManager(nullptr);
	FileManager.WriteStringToFile(FilePath, WriteStringView);

};
