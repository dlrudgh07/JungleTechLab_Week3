#include "FResourceManager.h"
#include "FFontAsset.h"
#include "FTexture.h"

void FResourceManager::Initialize()
{
	ImportFont("Assets/Fonts/Eng_Num.fnt");
}

FFontAsset* FResourceManager::ImportFont(const FString& FontFilePath, bool bWriteManifest)
{
	FFontAsset* NewFont = new FFontAsset();
	if (NewFont == nullptr) return nullptr;
	if (!NewFont->LoadFromFile(FontFilePath))
	{
		assert(false);
		return nullptr;
	}

	Fonts.Add(0, NewFont);

	return NewFont;
}

FTexture* FResourceManager::LoadTexture(const FString& FilePath)
{
	FTexture* NewTexture = new FTexture();
	if (!NewTexture) return nullptr;

	//텍스처 로드
	if (!NewTexture->LoadFromFile(Device, FilePath))
	{
		delete NewTexture;
		return nullptr;
	}

	Textures.Add(Key, NewTexture);

	//포인터 빌려주기
	return NewTexture;
}

FTexture* FResourceManager::FindTexture(const int32 Key) const
{
	FTexture* Tex = *Textures.Find(Key);
	return Tex;
}

FFontAsset* FResourceManager::FindFont(const int32& Key) const
{
	return Fonts[0];
}

void FResourceManager::SetDevice(ID3D11Device* device)
{
	Device = device;
}

void FResourceManager::Release()
{
	for (auto& pair : Textures)
	{
		if (pair.second == nullptr) continue;
		delete pair.second;
	}

	Textures.Reset();

	for (auto& pair : Fonts)
	{
		if (pair.second == nullptr) continue;
		delete pair.second;
	}

	Fonts.Reset();
}

FResourceManager::~FResourceManager()
{
	//무언가 해제할 것
	Release();
}
