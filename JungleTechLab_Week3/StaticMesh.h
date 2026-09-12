#pragma once
#include "Object.h"
#include <wrl/client.h> 
#include <d3d11.h>
#include "TArray.h"
#include <vector>
#include "Renderer.h"

// GPU용
struct FBuffer
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
	UINT NumVertices;
};

class UStaticMesh : public UObject
{
public:
	// factory 에서 필요함
	void Initialize() {};
	void Initialize(FBuffer&& InputVertexBufferGPU, const FVertexSimple* InputVertexBufferCPU, const uint32 VertexBufferCount,
		FBuffer&& InputIndexBufferGPU, const uint32* InputIndexBufferCPU, const uint32 IndexBufferCount);

	FBuffer VertexBufferGPU{};
	std::vector<FVertexSimple> VertexBufferCPU;
	FBuffer IndexBufferGPU{};
	std::vector<uint32> IndexBufferCPU;

	TArray<class UMaterial*> StaticMaterials;

	UStaticMesh() = default;
	~UStaticMesh()
	{
		// RAII
	}

	REFLECT_CLASS(UStaticMesh, UObject)

};
