#pragma once
#include "Object.h"
#include <wrl/client.h> 
#include <d3d11.h>
#include "TArray.h"
#include <vector>
#include "Renderer.h"

// 메시 하나를 그리는 데 필요한 GPU 버퍼 묶음
struct FBuffer //별도 헤더로 분리필요(Vertex,Index 정보가 Mesh에 종속적)
{

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer; //정점들이 여기있다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer; // 그릴순서는 여기에 담겨있다.
	uint32 NumVertices;   // welding된 정점개수
	uint32 NumIndices;    // 원본 Vertex의 정점정보 개수 (몇번 그릴것인가)
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
