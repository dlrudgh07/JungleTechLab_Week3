#pragma once
#include "Core.h"
#include "TMap.h"
#include "TArray.h"

class FTexture;

struct FCharacterInfo
{
	//유니코드 id
	int32 Id;

	//아틀라스 내 x,y 좌표(시작 지점)
	int32 X, Y;

	//글자의 크기
	uint32 Width, Height;

	//실제로 그릴 오프셋
	int32 XOffset, YOffset;

	//다음 글자로 이동할 펜 간격
	int32 XAdvance;

	//글자가 들어있는 페이지 인덱스
	int32 Page;

	//Kerning 데이터는 추후 추가할 것. 한글에는 없지만 영어에는 있다.
};

struct FFontUV
{
	float U0, V0, U1, V1;
};

class FFontAsset
{
public:
	//파일 로드
	bool LoadFromFile(const FString& FilePath);

	//문자 Id에 따라 CharacterInfo를 반환
	const FCharacterInfo* FindCharInfo(uint32 CharId)const;

	//Page Index에 따라 해당하는 FTexture를 반환
	const FTexture* GetPageTexture(int32 PageIndex)const;

	//줄 하나 파싱
	TMap<FString, FString> ParseKeyValueLine(const FString& line);

	//UV 계산하여 반환
	FFontUV GetUV(const FCharacterInfo& Info)const;

	int32 GetLineHeight()const;
	int32 GetBase()const;

private:
	//Key : 문자 아스키 코드
	//Value : 정보
	TMap<uint32, FCharacterInfo> CharInfoMap;

	//생성한 FTexture를 Page Index에 맞춰서 저장합니다.
	//Resource Manager 생성한 걸 들기만 합니다.
	// 직접 해지하지 않기
	TArray<FTexture*> Pages;

	//페이지 이름들
	TArray<FString> PageFileNames;

	//common에서 읽을 데이터들
	//줄 바꿈 높이
	int32 LineHeight = 0;
	//상단에서 Base라인까지 높이
	int32 Base = 0;
	//아틀라스 Width, Height
	int32 AtlasWidth = 0;
	int32 AtlasHeight = 0;

};

