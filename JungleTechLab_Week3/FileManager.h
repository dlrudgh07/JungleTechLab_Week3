#pragma once

#include <filesystem>

#include "Core.h"


inline constexpr std::string_view kDefaultRootPath = ".\\";
inline constexpr std::string_view kDefaultAssetsPath = ".\\Assets\\";

// HWND의 실체를 전방 선언으로 속여서 컴파일러를 통과시키기
struct HWND__;
typedef HWND__* HWND;

class FFileManager
{
public:
	FFileManager(HWND InputWindowHandle);
	FFileManager(HWND InputWindowHandle, std::string_view fileDirPath);
	FFileManager(std::string_view fileDirPath, std::string_view rootPath, HWND InputWindowHandle);

	FString ReadFileToString(std::string_view fileName) const;
	void WriteStringToFile(std::string_view fileName, std::string_view content) const;

	FString OpenFileDialog(const std::string& DefaultFileName, std::string_view kSceneDataSuffix, std::string_view kSceneDataDir) const;

private:
	std::filesystem::path mFileDirPath;
	std::filesystem::path mRootPath;
	HWND WindowHandle;

	bool IsUnderRoot(const std::filesystem::path& filePath) const;
	bool IsUnderFileDir(const std::filesystem::path& filePath) const;
};

bool IsUnder(const std::filesystem::path& filePath, const std::filesystem::path& rootPath);
