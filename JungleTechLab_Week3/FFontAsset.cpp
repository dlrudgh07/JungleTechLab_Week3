#include "FFontAsset.h"


void FFontAsset::AddCharInfo(int32 CharKey, FCharacterInfo Info)
{
	CharInfoMap.Add(CharKey, Info);
}

const FCharacterInfo* FFontAsset::GetCharInfo(int32 CharId) const
{
	const FCharacterInfo* info = CharInfoMap.Find(CharId);
	if (!info) return nullptr;
	return info;
}

TArray<FVector2>& FFontAsset::GetUV(const FCharacterInfo& Info) const
{
	TArray<FVector2> uv;
	FVector2 uv0, uv1;
	uv0.x = (float)Info.X / (float)AtlasWidth;
	uv0.y = (float)Info.Y / (float)AtlasHeight;
	uv1.x = (float)(Info.X + Info.Width) / (float)AtlasWidth;
	uv1.y = (float)(Info.Y + Info.Height) / (float)AtlasHeight;
	uv.Add(uv0);
	uv.Add(uv1);
	return uv;
}

int32 FFontAsset::GetLineHeight() const
{
	return LineHeight;
}

void FFontAsset::SetLineHeight(int32 pLineHeight)
{
	LineHeight = pLineHeight;
}

int32 FFontAsset::GetBase() const
{
	return Base;
}

void FFontAsset::SetBaseLine(int32 pBaseLine)
{
	Base = pBaseLine;
}

int32 FFontAsset::GetAtlasWidth() const
{
	return AtlasWidth;
}

void FFontAsset::SetAtlasWidth(int32 pAtlasWidth)
{
	AtlasWidth = pAtlasWidth;
}

int32 FFontAsset::GetAtlasHeight() const
{
	return AtlasHeight;
}

void FFontAsset::SetAtlasHeight(int32 pAtlasHeight)
{
	AtlasHeight = pAtlasHeight;
}

FString& FFontAsset::GetPageName(int32 PageIndex)
{
	return PageNames[PageIndex];
}

void FFontAsset::AddPageName(int32 Index, FString& NewPage)
{
	PageNames.Add(Index, NewPage);
}

FString FFontAsset::GetFontName() const
{
	return FontName;
}

void FFontAsset::SetFontName(const FString& pFontName)
{
	FontName = pFontName;
}

