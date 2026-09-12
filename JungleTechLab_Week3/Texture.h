#pragma once
#include "Object.h"
#include <wrl/client.h>
#include <d3d11.h>

// 렌더러가 쓸 순수 데이터
// 분리의 이유는 멀티 쓰레딩 때문
struct FTextureResource
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
};

// 에셋 매니저가 관리할 텍스처 에셋
class UTexture : public UObject
{
public:
	// factory 에서 필요함
	void Initialize() {};


	// 실제 렌더링에 쓰일 리소스 (렌더러로 넘어갈 놈)
	FTextureResource* Resource = nullptr;

	// 파일 경로, 해상도 등 메타 데이터
	// todo: 더 추가될수있음
	int32 Width;
	int32 Height;

	UTexture() = default;
	~UTexture() = default;

	void SetResource(FString FilePath);

	REFLECT_CLASS(UTexture, UObject)

};
