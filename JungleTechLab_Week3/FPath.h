#include "Core.h"

#include <Windows.h>

namespace FPaths
{
	// 파일 확장자 추출 (예: "DiffuseMap.png" -> "png")
	FORCEINLINE FString GetExtension(std::string_view path) {
		size_t dotPos = path.find_last_of('.');
		if (dotPos == std::string_view::npos)
			return FString(""); // 확장자 없음

		return FString(path.substr(dotPos + 1));
	}

	// 경로와 확장자를 제외한 순수 파일명 추출 (예: "C:/Textures/DiffuseMap.png" -> "DiffuseMap")
	FORCEINLINE FString GetBaseFilename(std::string_view path) {
		size_t slashPos = path.find_last_of("/\\");
		size_t startPos = (slashPos == std::string_view::npos) ? 0 : slashPos + 1;

		size_t dotPos = path.find_last_of('.');

		// 점이 없거나, 점이 폴더 이름에 포함된 경우 (예: "My.Folder/File")
		if (dotPos == std::string_view::npos || dotPos < startPos)
		{
			return FString(path.substr(startPos));
		}

		return FString(path.substr(startPos, dotPos - startPos));
	}

	// 경로를 제외하고 확장자가 포함된 파일명 추출 (예: "C:/Textures/DiffuseMap.png" -> "DiffuseMap.png")
	FORCEINLINE FString GetFilename(std::string_view path) {
		size_t slashPos = path.find_last_of("/\\");
		size_t startPos = (slashPos == std::string_view::npos) ? 0 : slashPos + 1;

		return FString(path.substr(startPos));
	}


	// todo: 이건 FString으로 넘겨야할듯함
	FORCEINLINE std::wstring StringToWString(const std::string& str) {
		if (str.empty()) return std::wstring();

		// 1. 필요한 와이드 차 문자열의 길이를 구합니다. (CP_UTF8 기준)
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);

		// 2. 해당 크기만큼 wstring 버퍼를 할당합니다.
		std::wstring wstrTo(size_needed, 0);

		// 3. 변환을 수행합니다.
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);

		return wstrTo;
	}
}
