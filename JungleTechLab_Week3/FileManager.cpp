#include "FileManager.h"


#include <fstream>
#include <sstream>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN // 잘 안 쓰는 불필요한 윈도우 API(네트워크, 암호화 등) 제외 (컴파일 속도 향상)
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>


FFileManager::FFileManager(HWND InputWindowHandle)
	: FFileManager(kDefaultAssetsPath, kDefaultRootPath, InputWindowHandle)
{
}

FFileManager::FFileManager(HWND InputWindowHandle, std::string_view fileDirPath)
	: FFileManager(fileDirPath, kDefaultRootPath, InputWindowHandle)
{
}

FFileManager::FFileManager(std::string_view fileDirPath, std::string_view rootPath, HWND InputWindowHandle)
	: mFileDirPath(fileDirPath)
	, mRootPath(rootPath)
	, WindowHandle(InputWindowHandle)
{
}


FString FFileManager::ReadFileToString(std::string_view fileName) const
{
	std::filesystem::path filePath = mFileDirPath / fileName;
	if (!IsUnderFileDir(filePath))
	{
		throw std::runtime_error("Attempted to read outside of the file directory: " + filePath.string());
	}

	std::ifstream fileStream(filePath, std::ios::in);
	if (!fileStream.is_open())
	{
		throw std::runtime_error("Failed to open file for reading: " + filePath.string());
	}

	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	return FString(buffer.str());
}

void FFileManager::WriteStringToFile(std::string_view fileName, std::string_view content) const
{
	std::filesystem::path filePath = mFileDirPath / fileName;
	if (!IsUnderFileDir(filePath))
	{
		throw std::runtime_error("Attempted to write outside of the file directory: " + filePath.string());
	}

	std::filesystem::create_directories(filePath.parent_path());

	std::ofstream fileStream(filePath, std::ios::out);
	if (!fileStream.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + filePath.string());
	}

	fileStream << content;
	return;
}

FString FFileManager::OpenFileDialog(const std::string& DefaultFileName, std::string_view kSceneDataSuffix, std::string_view kSceneDataDir) const
{
	char FileName[MAX_PATH] = { 0 };

	// std::string_view의 안전한 복사 방식 적용
	if (!DefaultFileName.empty())
	{
		size_t CopyLength = std::min(DefaultFileName.size(), static_cast<size_t>(MAX_PATH - 1));
		DefaultFileName.copy(FileName, CopyLength);
		FileName[CopyLength] = '\0';
	}

	// 1. 하드코딩 제거: std::string을 이용해 동적 필터 문자열 조립
	std::string FilterString;
	FilterString.reserve(128); // 메모리 재할당 방지용 넉넉한 예약

	FilterString += "Scene Files (*";
	FilterString += kSceneDataSuffix; // ".Scene"
	FilterString += ")";
	FilterString += '\0';             // 화면 표시용 이름 끝

	FilterString += "*";
	FilterString += kSceneDataSuffix; // "*.Scene"
	FilterString += '\0';             // 실제 확장자 필터 끝

	FilterString += "All Files (*.*)";
	FilterString += '\0';
	FilterString += "*.*";
	FilterString += '\0';
	// std::string의 c_str()이 끝에 '\0'을 하나 더 붙여주므로, 
	// 자연스럽게 윈도우 API가 요구하는 더블 널('\0\0') 조건이 만족됨

	OPENFILENAMEA OpenFileName = { 0 };
	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.hwndOwner = WindowHandle;

	// 조립된 동적 필터 적용
	OpenFileName.lpstrFilter = FilterString.c_str();
	OpenFileName.lpstrFile = FileName;
	OpenFileName.nMaxFile = MAX_PATH;

	OpenFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	OpenFileName.lpstrDefExt = kSceneDataSuffix.data() + 1; // 맨 앞의 '.' 제거

	std::string RelativePath = std::string(kDefaultAssetsPath) + std::string(kSceneDataDir);
	std::string InitialDirectoryPath = std::filesystem::absolute(RelativePath).string();

	if (!std::filesystem::exists(InitialDirectoryPath))
	{
		std::filesystem::create_directories(InitialDirectoryPath);
	}

	OpenFileName.lpstrInitialDir = InitialDirectoryPath.c_str();

	if (GetOpenFileNameA(&OpenFileName))
	{
		return FString(FileName);
	}

	return "";
}

/*
FString FFileManager::OpenFileDialog() const
{
	char FileName[MAX_PATH] = { 0 };
	OPENFILENAMEA OpenFileName = { 0 };

	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.hwndOwner = WindowHandle;

	OpenFileName.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
	OpenFileName.lpstrFile = FileName;
	OpenFileName.nMaxFile = MAX_PATH;

	OpenFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	OpenFileName.lpstrDefExt = "json";

	std::string InitialDirectoryPath = std::filesystem::absolute("./Assets/SceneData").string();

	if (!std::filesystem::exists(InitialDirectoryPath))
	{
		std::filesystem::create_directories(InitialDirectoryPath);
	}

	OpenFileName.lpstrInitialDir = InitialDirectoryPath.c_str();

	if (GetOpenFileNameA(&OpenFileName))
	{
		return FString(FileName);
	}

	return "";
}
*/



bool FFileManager::IsUnderRoot(const std::filesystem::path& filePath) const
{
	return IsUnder(filePath, mRootPath);
}

bool FFileManager::IsUnderFileDir(const std::filesystem::path& filePath) const
{
	return IsUnder(filePath, mFileDirPath);
}

bool IsUnder(const std::filesystem::path& targetPath, const std::filesystem::path& basePath)
{
	auto normalizedFile = std::filesystem::weakly_canonical(targetPath);
	auto normalizedRoot = std::filesystem::weakly_canonical(basePath);

	auto relativePath = std::filesystem::relative(normalizedFile, normalizedRoot);
	return !relativePath.empty() && relativePath.begin()->string() != "..";
}
