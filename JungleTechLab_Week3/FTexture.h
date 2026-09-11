#pragma once
#include "Core.h"
#include <d3d11.h>

class ID3D11Texture2D;
class ID3D11ShaderResourceView;
class ID3D11Device;

class FTexture
{
public:
	FTexture() = default;
	~FTexture();

	//복사 금지 : 이중 해제 위험 방지
	FTexture(const FTexture&) = delete;
	FTexture operator=(const FTexture&) = delete;

	//이동도 일단 금지
	//TMap<id(FName이길), FTexture*>에 포인터만 저장하는 구조이므로 이동할일이 없음.
	//필요해지면 구현하기
	FTexture(FTexture&&) = delete;
	FTexture& operator=(FTexture&&) = delete;

	//텍스처를 로드합니다.
	bool LoadFromFile(ID3D11Device* Device, const FString& FilePath);

	//Shader Resource View 반환
	ID3D11ShaderResourceView* GetSRV() const;

	uint32 GetWidth()const;
	uint32 GetHeight()const;

private:
	ID3D11Texture2D* Texture = nullptr;
	ID3D11ShaderResourceView* SRV = nullptr;
	uint32 Width = 0;
	uint32 Height = 0;
};

