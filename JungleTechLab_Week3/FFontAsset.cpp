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

TArray<FVector2> FFontAsset::GetUV(const FCharacterInfo& Info) const
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

#include "SceneSerialization.h"
#include "Texture.h"

void FFontAsset::SerializeClass(json::JSON& Out) const
{
    UObject::SerializeClass(Out);
    auto& P = Out["Properties"];
    P["FontName"] = FontName;
    P["LineHeight"] = LineHeight;
    P["Base"] = Base;
    P["AtlasWidth"] = AtlasWidth;
    P["AtlasHeight"] = AtlasHeight;
    P["Pages"] = json::JSON::Make(json::JSON::Class::Array);
    for (const auto& [Index, Name] : PageNames)
    {
        json::JSON Page;
        Page["Index"] = Index;
        Page["Name"] = Name;
        const auto* Texture = PageTextures.Find(Index);
        if (!Texture || !*Texture) throw std::runtime_error("Missing font atlas texture");
        Page["TextureGUID"] = ObjectReference(*Texture);
        P["Pages"].append(Page);
    }
    P["Characters"] = json::JSON::Make(json::JSON::Class::Array);
    for (const auto& [Id, Info] : CharInfoMap)
    {
        json::JSON Char;
        Char["Id"] = Info.Id;
        Char["X"] = Info.X;
        Char["Y"] = Info.Y;
        Char["Width"] = Info.Width;
        Char["Height"] = Info.Height;
        Char["XOffset"] = Info.XOffset;
        Char["YOffset"] = Info.YOffset;
        Char["XAdvance"] = Info.XAdvance;
        Char["Page"] = Info.Page;
        P["Characters"].append(Char);
    }
}

void FFontAsset::DeserializeClass(const json::JSON& In)
{
    UObject::DeserializeClass(In);
    const auto& P = In.at("Properties");
    if (P.at("FontName").JSONType() != json::JSON::Class::String) throw std::runtime_error("Invalid font name");
    FontName = FString(P.at("FontName").ToString());
    LineHeight = IntegerFromJson(P.at("LineHeight"), 1);
    Base = IntegerFromJson(P.at("Base"), 0);
    AtlasWidth = IntegerFromJson(P.at("AtlasWidth"), 1);
    AtlasHeight = IntegerFromJson(P.at("AtlasHeight"), 1);
    if (P.at("Pages").JSONType() != json::JSON::Class::Array || P.at("Characters").JSONType() != json::JSON::Class::Array)
        throw std::runtime_error("Invalid font arrays");
    PageNames.Reset();
    PageTextures.Reset();
    CharInfoMap.Reset();
    for (const auto& Page : P.at("Pages").ArrayRange())
    {
        const int32 Index = IntegerFromJson(Page.at("Index"), 0);
        if (PageNames.Contains(Index) || Page.at("Name").JSONType() != json::JSON::Class::String)
            throw std::runtime_error("Invalid font page");
        PageNames.Add(Index, FString(Page.at("Name").ToString()));
        auto* Texture = ResolveReference<UTexture>(Page.at("TextureGUID"));
        if (!Texture) throw std::runtime_error("Null font atlas");
        PageTextures.Add(Index, Texture);
    }
    for (const auto& Char : P.at("Characters").ArrayRange())
    {
        FCharacterInfo Info{};
        Info.Id = IntegerFromJson(Char.at("Id"), 0);
        Info.X = IntegerFromJson(Char.at("X"), 0);
        Info.Y = IntegerFromJson(Char.at("Y"), 0);
        Info.Width = IntegerFromJson(Char.at("Width"), 0);
        Info.Height = IntegerFromJson(Char.at("Height"), 0);
        Info.XOffset = IntegerFromJson(Char.at("XOffset"));
        Info.YOffset = IntegerFromJson(Char.at("YOffset"));
        Info.XAdvance = IntegerFromJson(Char.at("XAdvance"));
        Info.Page = IntegerFromJson(Char.at("Page"), 0);
        if (!PageNames.Contains(Info.Page) || CharInfoMap.Contains(Info.Id)) throw std::runtime_error("Invalid font character");
        CharInfoMap.Add(Info.Id, Info);
    }
}

