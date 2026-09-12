#include "StaticMesh.h"

void UStaticMesh::Initialize(FBuffer&& InputVertexBufferGPU, const FVertexSimple* InputVertexBufferCPU, const uint32 VertexBufferCount,
	FBuffer&& InputIndexBufferGPU, const uint32* InputIndexBufferCPU, const uint32 IndexBufferCount)
{
	VertexBufferGPU = InputVertexBufferGPU;
	IndexBufferGPU = InputIndexBufferGPU;

	for (uint32 CurrentIndex = 0; CurrentIndex < VertexBufferCount; ++CurrentIndex)
		VertexBufferCPU.emplace_back(InputVertexBufferCPU[CurrentIndex]);
	for (uint32 CurrentIndex = 0; CurrentIndex < IndexBufferCount; ++CurrentIndex)
		IndexBufferCPU.emplace_back(InputIndexBufferCPU[CurrentIndex]);
}
