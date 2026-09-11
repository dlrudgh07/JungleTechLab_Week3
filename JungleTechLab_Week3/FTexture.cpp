#include "FTexture.h"
#include <WICTextureLoader.h>

FTexture::~FTexture()
{
	if (Texture)
	{
		Texture->Release();
		Texture = nullptr;
	}

	if (SRV)
	{
		SRV->Release();
		SRV = nullptr;
	}
}

bool FTexture::LoadFromFile(ID3D11Device* Device, const FString& FilePath)
{
	if (!Device || FilePath == "") return false;

	std::string path = static_cast<std::string>(FilePath);
	std::wstring widePath(path.begin(), path.end());

	//// 1. Get the required buffer length (passing 0 to the last argument returns size)
	//int w_len = MultiByteToWideChar(CP_ACP, 0, FilePath.CStr(), -1, nullptr, 0);

	//// 2. Allocate the buffer
	//TCHAR* WidePath = new wchar_t[w_len];

	//// 3. Perform the conversion
	//MultiByteToWideChar(CP_ACP, 0, FilePath.CStr(), -1, WidePath, w_len);


	//텍스처 로드
	ID3D11Resource* Resource = nullptr;
	HRESULT hr = DirectX::CreateWICTextureFromFile(Device, widePath.c_str(), &Resource, &SRV);
	if (FAILED(hr))
	{
		if (Resource) Resource->Release();
		return false;
	}

	//뭔지 모르겠다. Resource를 Texture2D로 바꿔주는 거 같다.
	hr = Resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&Texture);
	if (FAILED(hr))
	{
		return false;
	}
	//QueryInterface는 참조 카운트를 하나 올린다. 원본 Reource는 해지.
	Resource->Release();

	D3D11_TEXTURE2D_DESC desc;
	Texture->GetDesc(&desc);
	Width = desc.Width;
	Height = desc.Height;

	return true;
}

ID3D11ShaderResourceView* FTexture::GetSRV() const
{
	return SRV;
}

uint32 FTexture::GetWidth() const
{
	return Width;
}

uint32 FTexture::GetHeight() const
{
	return Height;
}
