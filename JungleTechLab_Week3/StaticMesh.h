#pragma once
#include "Object.h"
#include <wrl/client.h> 
#include <d3d11.h>
#include "TArray.h"
#include <vector>
#include "Renderer.h"

struct FBuffer
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	UINT NumVertices;
};

class UStaticMesh : public UObject
{
public:
	// factory 에서 필요함
	void Initialize() {};


	// 1. 렌더러가 쓸 GPU 버퍼
	struct FBuffer* VertexBuffer = nullptr;

	// 2. 레이캐스트/물리 연산을 위한 CPU 원본 데이터
	// todo 일단은 vector로 쓰고 나중에 바꾸기
	std::vector<FVertexSimple> CPUVertices;

	TArray<class UMaterial*> StaticMaterials;

	UStaticMesh() = default;
	~UStaticMesh()
	{
		if (VertexBuffer != nullptr)
		{
			delete VertexBuffer;
			VertexBuffer = nullptr;
		}
	}

	REFLECT_CLASS(UStaticMesh, UObject)

};
