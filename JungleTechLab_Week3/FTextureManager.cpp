#include "FTextureManager.h"

FTexture* FTextureManager::LoadTexture(const FString& FilePath)
{
	// 1. 이미 로드된 텍스처인지 캐시에서 검색
	// FString을 std::string으로 변환 (해시맵 키용)
	// TODO -> 일단 이대로 진행

	std::string Key = FilePath.CStr();
	auto CurrentTexture = TextureMap.find(Key);
	if (CurrentTexture != TextureMap.end())
	{
		return CurrentTexture->second.get();
	}

	// 2. 캐시에 없으면 새로 생성
	auto NewTexture = std::make_unique<FTexture>();

	FString FileExtension = FPaths::GetExtension(FilePath).ToLower();

	
	std::wstring WideFilePath = FilePath.ToWideString();
	Microsoft::WRL::ComPtr<ID3D11Resource> Resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;

	HRESULT hr = S_OK;

	// 3. 확장자에 따른 로더 분기
	if (FileExtension == "dds")
	{
		hr = DirectX::CreateDDSTextureFromFile(
			Device.Get(),
			WideFilePath.c_str(),
			Resource.GetAddressOf(),
			SRV.GetAddressOf()
		);
	}
	else if (FileExtension == "png" || FileExtension == "jpg" || FileExtension == "bmp")
	{
		hr = DirectX::CreateWICTextureFromFile(
			Device.Get(),
			DeviceContext.Get(),
			WideFilePath.c_str(),
			Resource.GetAddressOf(),
			SRV.GetAddressOf()
		);
	}
	else
	{
		return nullptr; // 지원하지 않는 포맷
	}

	if (FAILED(hr))
	{
		return nullptr; // 텍스처 로드 실패
	}

	// 로컬에서 생성된 리소스를 NewTexture 객체에 할당
	NewTexture->Resource = Resource;
	NewTexture->SRV = SRV;

	// 4. 텍스처의 너비/높이 정보 추출
	Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2D;
	if (SUCCEEDED(NewTexture->Resource.As(&Texture2D)))
	{
		D3D11_TEXTURE2D_DESC Description;
		Texture2D->GetDesc(&Description);
		NewTexture->Width = Description.Width;
		NewTexture->Height = Description.Height;
	}

	// 5. 캐시 맵에 등록하고 반환
	FTexture* RawPointer = NewTexture.get();
	TextureMap[Key] = std::move(NewTexture);

	return TextureMap[Key].get();
}
