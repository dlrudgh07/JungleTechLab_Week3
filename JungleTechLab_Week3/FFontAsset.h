#pragma once
#include "Core.h"
#include "TMap.h"
#include "Vector.h"
#include "TArray.h"

//한 글자가 담고 있는 폰트 정보
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

class FFontAsset
{
public:
	void AddCharInfo(int32 CharKey, FCharacterInfo Info);
	//문자 Id에 따라 CharacterInfo를 반환
	const FCharacterInfo* GetCharInfo(int32 CharId)const;

	//UV 계산하여 반환
	TArray<FVector2>& GetUV(const FCharacterInfo& Info)const;

	int32 GetLineHeight()const;
	void SetLineHeight(int32 pLineHeight);

	int32 GetBase()const;
	void SetBaseLine(int32 pBaseLine);

	int32 GetAtlasWidth()const;
	void SetAtlasWidth(int32 pAtlasWidth);

	int32 GetAtlasHeight()const;
	void SetAtlasHeight(int32 pAtlasHeight);

	FString& GetPageName(int32 PageIndex);
	void AddPageName(int32 Index, FString& NewPage);

	//폰트의 이름을 반환합니다.
	FString GetFontName()const;
	void SetFontName(const FString& pFontName);

private:
	//폰트 이름
	FString FontName;

	//Key : 문자 아스키 코드
	//Value : 정보
	TMap<int32, FCharacterInfo> CharInfoMap;

	//생성한 아틀라스 텍스처 이름을 저장합니다.
	// Page 인덱스 번호로 해당하는 Texture의 이름을 들고 옵니다.
	TMap<int32, FString> PageNames;

	//common에서 읽을 데이터들
	//줄 바꿈 높이
	int32 LineHeight = 0;
	//상단에서 Base라인까지 높이
	int32 Base = 0;
	//아틀라스 Width, Height
	int32 AtlasWidth = 0;
	int32 AtlasHeight = 0;

};

