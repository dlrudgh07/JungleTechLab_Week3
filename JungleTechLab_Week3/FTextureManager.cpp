#include "FTextureManager.h"

void FTextureManager::Initialize(ID3D11Device* InputDevice, ID3D11DeviceContext* InputDeviceContext)
{
	Device = InputDevice;
	DeviceContext = InputDeviceContext;

	CreateDefaultWhiteTexture();
}

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

void FTextureManager::CreateDefaultWhiteTexture()
{
	// 1. 1x1 크기의 하얀색 픽셀 데이터 준비
	uint32 WhitePixel = 0xFFFFFFFF; // RGBA 모두 255

	D3D11_TEXTURE2D_DESC Desc = {};
	Desc.Width = 1;
	Desc.Height = 1;
	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Desc.SampleDesc.Count = 1;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = &WhitePixel;
	InitData.SysMemPitch = sizeof(uint32); // 1줄의 바이트 크기 (4바이트)

	// 2. DX11 리소스 및 SRV 생성
	Microsoft::WRL::ComPtr<ID3D11Texture2D> DefaultTexture2D;
	Device->CreateTexture2D(&Desc, &InitData, DefaultTexture2D.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DefaultSRV;
	Device->CreateShaderResourceView(DefaultTexture2D.Get(), nullptr, DefaultSRV.GetAddressOf());

	// 3. FTexture 객체 조립
	auto DefaultTexture = std::make_unique<FTexture>();
	DefaultTexture->Resource = DefaultTexture2D;
	DefaultTexture->SRV = DefaultSRV;
	DefaultTexture->Width = 1;
	DefaultTexture->Height = 1;

	// 4. 캐시 맵에 "DefaultWhite"라는 예약된 키값으로 등록
	TextureMap["DefaultWhite"] = std::move(DefaultTexture);
}
