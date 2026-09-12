#include "FFontAsset.h"
#include "FTexture.h"
#include <fstream>
#include "Console.h"
#include <ctype.h>
#include "FResourceManager.h"


bool FFontAsset::LoadFromFile(const FString& FilePath)
{
	std::ifstream file(FilePath);
	if (!file.is_open())
	{
		UE_LOG("Cannot open file %s", FilePath.CStr());
		return false;
	}

	std::string rawLine;
	while (std::getline(file, rawLine))
	{
		FString line = std::string_view(rawLine);

		//\r은 잘라내기
		if (line.EndsWith(std::string("\r")))
		{
			line = line.LeftChop(1);
		}

		//비어있으면
		if (line.Equals(FString("")))
			continue;

		//기본 정보
		if (line.Find(FString("common"), 0) == 0)
		{
			//파싱
			TMap<FString, FString> CharMap = ParseKeyValueLine(line);

			//기본 정보 저장
			LineHeight = std::stoi(CharMap["lineHeight"]);
			Base = std::stoi(CharMap["base"]);
			AtlasWidth = std::stoi(CharMap["scaleW"]);
			AtlasHeight = std::stoi(CharMap["scaleH"]);
		}
		//페이지 저장
		else if (line.Find(FString("page"), 0) == 0)
		{
			TMap<FString, FString> CharMap = ParseKeyValueLine(line);

			//페이지 인덱스
			int32 PageId = std::stoi(CharMap["id"]);
			FString FileName = CharMap["file"];

			//이름을 저장해야하나??
			PageFileNames.Insert(FileName, PageId);

			//파일 경로
			FString FilePath = FString("Assets/Fonts/").Append(FileName);

			//Page 텍스처 로드
			FTexture* PageTex = FResourceManager::Get().LoadTexture(FilePath);
			if (!PageTex)
			{
				UE_LOG("Font Page Texture Load Failed");
				continue;
			}

			Pages.Add(PageTex);
		}
		//문자 데이터 저장
		else if (line.Find(FString("char"), 0) == 0)
		{
			//chars 항목은 건너 뛰기 위함.
			//공백 인덱스 찾기
			//int32 SpaceIndex = line.Find(FString(" "), 0);

			////공백이 없다면 넘어가기, 있다면 공백 왼쪽으로 가져오기
			//if (SpaceIndex == -1) continue;
			//FString Token = line.Left(SpaceIndex);
			////chars라면 넘어가기
			//if (Token == "chars") continue;

			TMap<FString, FString> CharMap = ParseKeyValueLine(line);

			//chars 항목이면 넘어가기
			if (CharMap.Num() < 2) continue;

			FCharacterInfo info;
			info.Id = std::stoi(CharMap["id"]);
			info.X = std::stoi(CharMap["x"]);
			info.Y = std::stoi(CharMap["y"]);
			info.Width = std::stoi(CharMap["width"]);
			info.Height = std::stoi(CharMap["height"]);
			info.XOffset = std::stoi(CharMap["xoffset"]);
			info.YOffset = std::stoi(CharMap["yoffset"]);
			info.XAdvance = std::stoi(CharMap["xadvance"]);
			info.Page = std::stoi(CharMap["page"]);

			CharInfoMap.Add(info.Id, info);
		}
		//kerning이 생기면 작업할 것.
		/*else if (line.Find(FString("char"), 0) == 0)
		{

		}*/
	}

	if (AtlasWidth == 0 || AtlasHeight == 0 || CharInfoMap.Num() == 0)
	{
		UE_LOG("Atlas fnt file is empty..");
		return false;
	}

	return true;
}

const FCharacterInfo* FFontAsset::FindCharInfo(int32 CharId) const
{
	const FCharacterInfo* info = CharInfoMap.Find(CharId);
	if (!info) return nullptr;
	return info;
}

const FTexture* FFontAsset::GetPageTexture(int32 PageIndex) const
{
	const FTexture* tex = Pages[PageIndex];
	if (!tex) return nullptr;
	return tex;
}

TMap<FString, FString> FFontAsset::ParseKeyValueLine(const FString& line)
{
	TMap<FString, FString> Result;

	int32 i = 0;
	int32 len = line.Len();

	FString remaining = line;

	while (remaining.Len() > 0)
	{
		//공백 인덱스 찾기
		int32 SpaceIndex = remaining.Find(FString(" "), 0);

		//공백이 없다면 한 글자, 있다면 공백 왼쪽으로 가져오기
		FString Token = (SpaceIndex == -1) ? remaining : remaining.Left(SpaceIndex);
		//한글자였다면 빈칸으로 두어 while 탈출, 아니라면 다음 검사를 위해 공백 오른쪽 문자열로 할당
		remaining = (SpaceIndex == -1) ? FString("") : remaining.RightChop(SpaceIndex + 1);

		//공백 왼쪽 문자열에 = 가 없다면 common char와 같은 타입이다.
		int32 EqualIndex = Token.Find(FString("="), 0);
		if (EqualIndex == -1) continue;

		//key, value 할당
		FString key = Token.Left(EqualIndex);
		FString value = Token.RightChop(EqualIndex + 1);

		//value가 page의 file일 경우 따옴표를 제거해야 함.
		//""만 해도 len이 2다.
		if (value.Len() >= 2 && value.StartsWith(FString("\"")) && value.EndsWith(FString("\"")))
		{
			value = value.Mid(1, value.Len() - 2);
		}

		Result.Add(key, value);
	}

	return Result;
}

//FVector2 FFontAsset::GetUV(const FCharacterInfo& Info) const
//{
//	FFontUV uv;
//	uv.U0 = (float)Info.X / (float)AtlasWidth;
//	uv.V0 = (float)Info.Y / (float)AtlasHeight;
//	uv.U1 = (float)(Info.X + Info.Width) / (float)AtlasWidth;
//	uv.V1 = (float)(Info.Y + Info.Height) / (float)AtlasHeight;
//
//	return uv;
//}

int32 FFontAsset::GetLineHeight() const
{
	return LineHeight;
}

int32 FFontAsset::GetBase() const
{
	return Base;
}

int32 FFontAsset::GetAtlasWidth() const
{
	return AtlasWidth;
}

int32 FFontAsset::GetAtlasHeight() const
{
	return AtlasHeight;
}

