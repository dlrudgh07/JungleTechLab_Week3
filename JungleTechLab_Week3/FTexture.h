#pragma once

#include "Core.h"
#include <wrl/client.h>
#include <d3d11.h>

class FTexture
{
public:
	Microsoft::WRL::ComPtr<ID3D11Resource>           Resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;

	uint32 Width = 0;
	uint32 Height = 0;

	// 렌더러가 쉽게 바인딩할 수 있도록 도와주는 헬퍼
	void Bind(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, uint32 slot = 0)
	{
		context->PSSetShaderResources(slot, 1, SRV.GetAddressOf());
	}
};
